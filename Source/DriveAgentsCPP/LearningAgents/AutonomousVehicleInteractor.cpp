
#include "AutonomousVehicleInteractor.h"
#include "AutonomousVehiclePawn.h"

// Learning agents API
#include "LearningAgentsManager.h"
#include "LearningAgentsObservations.h"
#include "LearningAgentsHelpers.h"
#include "LearningAgentsActions.h"

// Vehicle movement
#include "ChaosWheeledVehicleMovementComponent.h"

// Misc
#include "Kismet/KismetStringLibrary.h"

UAutonomousVehicleInteractor::UAutonomousVehicleInteractor(FVTableHelper& Helper) : Super(Helper) {}

void UAutonomousVehicleInteractor::SetupObservations_Implementation()
{
	// Setup the four basic observations (mostly with defaults)
	TrackPositionObservation = UPlanarPositionObservation::AddPlanarPositionObservation(this, "TrackPositionObservation");
	TrackDirectionObservation = UPlanarDirectionObservation::AddPlanarDirectionObservation(this, "TrackDirectionObservation");
	TrackPositionParameterObservation = UAngleObservation::AddAngleObservation(this, "TrackPositionParameterObservation");
	CarVelocityObservation = UPlanarVelocityObservation::AddPlanarVelocityObservation(this, "CarVelocityObservation");

	// Setup the nearby vehicle observations
	NearbyPositionObservations = UPositionArrayObservation::AddPositionArrayObservation(this, "NearbyPositionObservations", NearbyObservationCount);

	// Setup the look ahead observations
	TrackLookAheadPositionObservations = UPositionArrayObservation::AddPositionArrayObservation(this, "TrackLookAheadPositionObservations", LookAheadObservationCount);
	TrackLookAheadDirectionObservations = UDirectionArrayObservation::AddDirectionArrayObservation(this, "TrackLookAheadDirectionObservations", LookAheadObservationCount);

	// Setup the helper
	TrackSplineHelper = USplineComponentHelper::AddSplineComponentHelper(this, "TrackSplineHelper");
}

void UAutonomousVehicleInteractor::SetupActions_Implementation()
{
	// Add our two input actions
	ThrottleBrakeAction = UFloatAction::AddFloatAction(this, "ThrottleBrakeAction");
	SteeringAction = UFloatAction::AddFloatAction(this, "SteeringAction");
}

void UAutonomousVehicleInteractor::SetObservations_Implementation(const TArray<int32>& AgentIds)
{
	// Sanity Check
	if (TrackSpline == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot observe a null track spline. Did you remember to assign the spline?"));
		return;
	}

	// Loop over all agent IDs
	for (int32 AgentId : AgentIds)
	{
		// Retrieve Agent as Actor
		AActor* Agent = Cast<AActor>(GetAgent(AgentId));
		if (Agent != nullptr)
		{
			// Retrieve basic physical properties of Agent
			FVector relativePosition = Agent->GetActorLocation();
			FRotator relativeRotation = Agent->GetActorRotation();

			// Compute un-normalized distance along spline
			float splineDistance = TrackSplineHelper->GetDistanceAlongSplineAtPosition(AgentId, TrackSpline, relativePosition);

			// Compute exact track position at that distance and set track position observation
			FVector splinePosition = TrackSplineHelper->GetPositionAtDistanceAlongSpline(AgentId, TrackSpline, splineDistance);
			TrackPositionObservation->SetPlanarPositionObservation(AgentId, splinePosition, relativePosition, relativeRotation);

			// Compute spline tangent direction at that distance and set observation
			FVector splineDirection = TrackSplineHelper->GetDirectionAtDistanceAlongSpline(AgentId, TrackSpline, splineDistance);
			TrackDirectionObservation->SetPlanarDirectionObservation(AgentId, splineDirection, relativeRotation);

			// Compute look ahead positions and directions
			float lookAheadSplineDistance = splineDistance + LookAheadDistance;
			TArray<FVector> lookAheadPos;
			TArray<FVector> lookAheadDir;
			for (int i = 0; i < LookAheadObservationCount; i++)
			{
				// Compute look ahead position/direction and gather in arrays
				lookAheadPos.Add(TrackSplineHelper->GetPositionAtDistanceAlongSpline(AgentId, TrackSpline, lookAheadSplineDistance));
				lookAheadDir.Add(TrackSplineHelper->GetDirectionAtDistanceAlongSpline(AgentId, TrackSpline, lookAheadSplineDistance));

				// Increment look ahead distance
				lookAheadSplineDistance += LookAheadDistance;
			}

			// Add the observation arrays
			TrackLookAheadPositionObservations->SetPositionArrayObservation(AgentId, lookAheadPos, relativePosition, relativeRotation);
			TrackLookAheadDirectionObservations->SetDirectionArrayObservation(AgentId, lookAheadDir, relativeRotation);

			// Compute normalized distance along spline (as angle) and set observation
			float splineParameter = TrackSplineHelper->GetProportionAlongSplineAsAngle(AgentId, TrackSpline, splineDistance);
			TrackPositionParameterObservation->SetAngleObservation(AgentId, splineParameter);

			// Read actor velocity and set observation
			FVector velocity = Agent->GetVelocity();
			CarVelocityObservation->SetPlanarVelocityObservation(AgentId, velocity, relativeRotation);

			// Loop over other actors and find closest
			TArray<FVector> nearbyPos = FindClosestAgents(AgentIds, AgentId, relativePosition);
			NearbyPositionObservations->SetPositionArrayObservation(AgentId, nearbyPos, relativePosition, relativeRotation);
		}
	}
}

TArray<FVector> UAutonomousVehicleInteractor::FindClosestAgents(const TArray<int32>& AgentIds, int32 CurId, FVector relativePosition) const
{
	// Initialize arrays with max allowed distance
	TArray<FVector> minPos;
	TArray<float> minDist;

	minPos.Init(FVector(-NearbyMaxDistance, 0.0f, 0.0f), NearbyObservationCount);
	minDist.Init(NearbyMaxDistance * NearbyMaxDistance, NearbyObservationCount);
	
	// Loop over all agent IDs
	for (int32 AgentId : AgentIds)
	{
		// Skip the current agent
		if (AgentId == CurId) {
			continue;
		}

		// Retrieve Agent as Actor
		const AActor* Agent = Cast<AActor>(GetAgent(AgentId));
		if (Agent != nullptr)
		{
			float sqDistance = FVector::DistSquared(relativePosition, Agent->GetActorLocation());
			int32 minIndex = 0;
			while (minIndex < NearbyObservationCount && minDist[minIndex] > sqDistance) {
				minIndex++;
			}

			if (minIndex < NearbyObservationCount) {
				minDist.Insert(sqDistance, minIndex);
				minPos.Insert(Agent->GetActorLocation(), minIndex);
			}
		}
	}

	while (minPos.Num() > NearbyObservationCount) {
		minPos.Pop();
	}

	// Return back the list
	return minPos;
}


void UAutonomousVehicleInteractor::GetActions_Implementation(const TArray<int32>& AgentIds)
{
	// Loop over all agent IDs
	for (int32 AgentId : AgentIds)
	{
		// Retrieve Agent as our specific pawn type (for access to vehicle movement component)
		AAutonomousVehiclePawn* Agent = Cast<AAutonomousVehiclePawn>(GetAgent(AgentId));
		if (Agent != nullptr)
		{
			// Retrieve the movement component
			UChaosWheeledVehicleMovementComponent* MovementComponent = Agent->GetChaosVehicleMovement();

			// Apply steering action
			MovementComponent->SetSteeringInput(SteeringAction->GetFloatAction(AgentId));

			// Are we accelerating or braking?
			float throttle = ThrottleBrakeAction->GetFloatAction(AgentId);
			if (throttle >= 0)
			{
				// Apply as acceleration
				MovementComponent->SetBrakeInput(0.0f);
				MovementComponent->SetThrottleInput(throttle);
			}
			else
			{
				// Apply as braking
				MovementComponent->SetBrakeInput(0.0f - throttle);
				MovementComponent->SetThrottleInput(0.0f);
			}
		}
	}
}
