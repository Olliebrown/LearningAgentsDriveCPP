// Fill out your copyright notice in the Description page of Project Settings.


#include "AutonomousVehicleTrainer.h"

#include "AutonomousVehiclePawn.h"

#include "LearningAgentsRewards.h"
#include "LearningAgentsCompletions.h"
#include "LearningAgentsHelpers.h"

void UAutonomousVehicleTrainer::SetupRewards_Implementation()
{
	// Setup the rewards/penalties
	OffTrackPenalty = UPlanarPositionDifferencePenalty::AddPlanarPositionDifferencePenalty(this, "OffTrackPenalty", 1.0f, 100.0f, 400.0f);
	SpeedReward = UScalarVelocityReward::AddScalarVelocityReward(this, "SpeedReward", 0.1f, 200.0f);

	// Setup the helper
	TrackSplineHelper = USplineComponentHelper::AddSplineComponentHelper(this, "TrackSplineHelper");
}

void UAutonomousVehicleTrainer::SetupCompletions_Implementation()
{
	OffTrackCompletion = UPlanarPositionDifferenceCompletion::AddPlanarPositionDifferenceCompletion(this, "OffTrackCompletion", 600.0f, ELearningAgentsCompletion::Termination);
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
		// Retrieve Agent as Actor
		AActor* Agent = Cast<AActor>(GetAgent(AgentId));
		if (Agent != nullptr)
		{
			// Get actor position and velocity
			FVector actorPosition = Agent->GetActorLocation();
			FVector actorVelocity = Agent->GetVelocity();

			// Find nearest position on spline and compute difference for penalty
			FVector nearestSplinePosition = TrackSplineHelper->GetNearestPositionOnSpline(AgentId, TrackSpline, actorPosition);
			OffTrackPenalty->SetPlanarPositionDifferencePenalty(AgentId, actorPosition, nearestSplinePosition);

			// Project velocity onto spline direction and compute length then apply reward
			float splineScalarVelocity = TrackSplineHelper->GetVelocityAlongSpline(AgentId, TrackSpline, actorPosition, actorVelocity);
			SpeedReward->SetScalarVelocityReward(AgentId, splineScalarVelocity);
		}
	}
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
