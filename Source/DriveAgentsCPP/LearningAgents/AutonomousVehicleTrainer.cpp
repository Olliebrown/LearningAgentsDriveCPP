// Fill out your copyright notice in the Description page of Project Settings.

#define UE_LEARNING_ARRAY_CHECK_VALUES 0

#include "AutonomousVehicleTrainer.h"

#include "AutonomousVehiclePawn.h"

#include "LearningAgentsRewards.h"
#include "LearningAgentsCompletions.h"
#include "LearningAgentsHelpers.h"

void UAutonomousVehicleTrainer::SetupRewards_Implementation()
{
	// Setup the rewards/penalties
	OffTrackPenalty = UPlanarPositionDifferencePenalty::AddPlanarPositionDifferencePenalty(this, "OffTrackPenalty", OffCenterPenaltyWeight, 100.0f, TrackWidth / 3.0f * 2.0f);
	SpeedReward = UScalarVelocityReward::AddScalarVelocityReward(this, "SpeedReward", SpeedRewardWeight, 200.0f);
	OverlapPenalty = UConditionalReward::AddConditionalReward(this, "OverlapPenalty", OverlapPenaltyWeight);
	//AgentProximityPenalty = UPositionArraySimilarityReward::AddPositionArraySimilarityReward(this,
	//	"AgentProximityPenalty", AgentProximityCount, AgentProximityPenaltyWeight, 100.0f
	//);

	// Setup the helper
	TrackSplineHelper = USplineComponentHelper::AddSplineComponentHelper(this, "TrackSplineHelper");
}

void UAutonomousVehicleTrainer::SetupCompletions_Implementation()
{
	OffTrackCompletion = UPlanarPositionDifferenceCompletion::AddPlanarPositionDifferenceCompletion(
		this, "OffTrackCompletion", TrackWidth, ELearningAgentsCompletion::Termination,
		FVector::ForwardVector, FVector::RightVector
	);
}

void UAutonomousVehicleTrainer::SetRewards_Implementation(const TArray<int32>& AgentIds)
{
	// Sanity Check
	if (TrackSpline == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot train with a null track spline. Did you remember to assign the spline?"));
		return;
	}

	// Loop over all agent IDs
	for (int32 AgentId : AgentIds)
	{
		// Retrieve Agent as Vehicle Pawn
		AAutonomousVehiclePawn* Agent = Cast<AAutonomousVehiclePawn>(GetAgent(AgentId));
		if (Agent != nullptr)
		{
			// Get actor position and velocity
			FVector actorPosition = Agent->GetActorLocation();
			FRotator actorRotation = Agent->GetActorRotation();
			FVector actorVelocity = Agent->GetVelocity();

			// Find nearest position on spline and compute difference for penalty
			FVector nearestSplinePosition = TrackSplineHelper->GetNearestPositionOnSpline(AgentId, TrackSpline, actorPosition);
			OffTrackPenalty->SetPlanarPositionDifferencePenalty(AgentId, actorPosition, nearestSplinePosition);

			// Project velocity onto spline direction and compute length then apply reward
			float splineScalarVelocity = TrackSplineHelper->GetVelocityAlongSpline(AgentId, TrackSpline, actorPosition, actorVelocity);
			SpeedReward->SetScalarVelocityReward(AgentId, splineScalarVelocity);

			// Check for overlapping agents
			OverlapPenalty->SetConditionalReward(AgentId, Agent->GetOverlappingAgentCount() > 0);

			//// Get array of nearby agents
			//TArray<FVector> AgentPositions = FindClosestAgents(AgentIds, AgentId, actorPosition);

			//// Build array of copies of actor position
			//TArray<FVector> ActorPositions;
			//ActorPositions.Init(actorPosition, AgentProximityCount);

			//// Set penalty for similarty between agent positions and actor position
			//// (e.g. Peanlize for getting to close to other vehicles)
			//AgentProximityPenalty->SetPositionArraySimilarityReward(AgentId,
			//	ActorPositions, AgentPositions, actorPosition, actorPosition,
			//	actorRotation, actorRotation
			//);
		}
	}
}

TArray<FVector> UAutonomousVehicleTrainer::FindClosestAgents(const TArray<int32>& AgentIds, int32 CurId, FVector relativePosition) const
{
	// Initialize arrays with max allowed distance
	TArray<FVector> minPos;
	TArray<float> minDist;

	minPos.Init(FVector(-AgentProximityMaxDistance, 0.0f, 0.0f), AgentProximityCount);
	minDist.Init(AgentProximityMaxDistance * AgentProximityMaxDistance, AgentProximityCount);

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
			// Compute distance to agent
			float sqDistance = FVector::DistSquared(relativePosition, Agent->GetActorLocation());

			// Look through min index array for position
			int32 minIndex = 0;
			while (minIndex < AgentProximityCount && minDist[minIndex] > sqDistance) {
				minIndex++;
			}

			// Does this belong in list?
			if (minIndex < AgentProximityCount) {
				minDist.Insert(sqDistance, minIndex);
				minPos.Insert(Agent->GetActorLocation(), minIndex);
			}

			// Remove any extra entries
			while (minDist.Num() > AgentProximityCount) {
				minDist.Pop();
				minPos.Pop();
			}
		}
	}

	// Return back the list of positions
	return minPos;
}

void UAutonomousVehicleTrainer::SetCompletions_Implementation(const TArray<int32>& AgentIds)
{
	// Sanity Check
	if (TrackSpline == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot train with a null track spline. Did you remember to assign the spline?"));
		return;
	}

	// Loop over all agent IDs
	for (int32 AgentId : AgentIds)
	{
		// Retrieve Agent as Actor
		AActor* Agent = Cast<AActor>(GetAgent(AgentId));
		if (Agent != nullptr)
		{
			// Get actor position
			FVector actorPosition = Agent->GetActorLocation();

			// Find nearest position on spline and compute difference for completion
			FVector nearestSplinePosition = TrackSplineHelper->GetNearestPositionOnSpline(AgentId, TrackSpline, actorPosition);
			OffTrackCompletion->SetPlanarPositionDifferenceCompletion(AgentId, actorPosition, nearestSplinePosition);
		}
	}
}

void UAutonomousVehicleTrainer::ResetEpisodes_Implementation(const TArray<int32>& AgentIds)
{
	// Loop over all agent IDs
	for (int32 AgentId : AgentIds)
	{
		// Retrieve Agent as our specific pawn type (for access to vehicle movement component)
		AAutonomousVehiclePawn* Agent = Cast<AAutonomousVehiclePawn>(GetAgent(AgentId));
		if (Agent != nullptr)
		{
			// Move to a random position along the track when reset
			Agent->ResetToRandomPointOnSpline(TrackSpline);
		}
	}
}
