
#include "AutonomousVehicleInteractor.h"
#include "AutonomousVehiclePawn.h"

// Learning agents API
#include "LearningAgentsObservations.h"
#include "LearningAgentsHelpers.h"
#include "LearningAgentsActions.h"

// Vehicle movement
#include "ChaosWheeledVehicleMovementComponent.h"

UAutonomousVehicleInteractor::UAutonomousVehicleInteractor(FVTableHelper& Helper) : Super(Helper) {}

void UAutonomousVehicleInteractor::SetupObservations_Implementation()
{
	// Setup the four basic observations (mostly with defaults)
	TrackPositionObservation = UPlanarPositionObservation::AddPlanarPositionObservation(this, "TrackPositionObservation");
	TrackDirectionObservation = UPlanarDirectionObservation::AddPlanarDirectionObservation(this, "TrackDirectionObservation");
	TrackPositionParameterObservation = UAngleObservation::AddAngleObservation(this, "TrackPositionParameterObservation");
	CarVelocityObservation = UPlanarVelocityObservation::AddPlanarVelocityObservation(this, "CarVelocityObservation");

	// Setup the look ahead observations
	TrackLookAheadPositionArrayObservations = UPlanarPositionArrayObservation::AddPlanarPositionArrayObservation(this, "TrackLookAheadPositionArrayObservations", LookAheadObservationCount);
	TrackLookAheadDirectionArrayObservations = UPlanarDirectionArrayObservation::AddPlanarDirectionArrayObservation(this, "TrackLookAheadDirectionArrayObservations", LookAheadObservationCount);

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
			TArray<FVector> lookAheadPositionArray;
			TArray<FVector> lookAheadDirectionArray;
			lookAheadPositionArray.Reserve(LookAheadObservationCount);
			lookAheadDirectionArray.Reserve(LookAheadObservationCount);
			for (int i = 0; i < LookAheadObservationCount; i++)
			{
				// Compute look ahead position and set observation
				FVector lookAheadPosition = TrackSplineHelper->GetPositionAtDistanceAlongSpline(AgentId, TrackSpline, lookAheadSplineDistance);
				lookAheadPositionArray.Push(lookAheadPosition);

				// Compute look ahead direction and set observation
				FVector lookAheadDirection = TrackSplineHelper->GetDirectionAtDistanceAlongSpline(AgentId, TrackSpline, lookAheadSplineDistance);
				lookAheadDirectionArray.Push(lookAheadDirection);

				// Increment look ahead distance
				lookAheadSplineDistance += LookAheadDistance;
			}
			TrackLookAheadPositionArrayObservations->SetPlanarPositionArrayObservation(AgentId, lookAheadPositionArray, relativePosition, relativeRotation);
			TrackLookAheadDirectionArrayObservations->SetPlanarDirectionArrayObservation(AgentId, lookAheadDirectionArray, relativeRotation);

			// Compute normalized distance along spline (as angle) and set observation
			float splineParameter = TrackSplineHelper->GetProportionAlongSplineAsAngle(AgentId, TrackSpline, splineDistance);
			TrackPositionParameterObservation->SetAngleObservation(AgentId, splineParameter);

			// Read actor velocity and set observation
			FVector velocity = Agent->GetVelocity();
			CarVelocityObservation->SetPlanarVelocityObservation(AgentId, velocity, relativeRotation);
		}
	}
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
