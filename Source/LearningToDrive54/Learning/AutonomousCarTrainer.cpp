// Fill out your copyright notice in the Description page of Project Settings.


#include "AutonomousCarTrainer.h"

#include "LearningAgentsManager.h"
#include "LearningAgentsRewards.h"

#include "Components/SplineComponent.h"
#include "../ResetableVehiclePawn.h"
#include "ChaosWheeledVehicleMovementComponent.h"

UAutonomousCarTrainer::UAutonomousCarTrainer() {
	TrackSpline = nullptr;
	OffTrackThreshold = 1200.0f;
	CollisionThreshold = 5;
	
	bManualTransmission = false;
	UpShiftAt = 3000.0f;
	DownShiftAt = 1500.0f;
}

void UAutonomousCarTrainer::GatherAgentReward_Implementation(float& OutReward, const int32 AgentId)
{
	// Get reference to agent and it's movement component
	AResetableVehiclePawn* Agent = Cast<AResetableVehiclePawn>(Manager->GetAgent(AgentId, AWheeledVehiclePawn::StaticClass()));
	if (Agent == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Trainer (26): Casting of agent failed."));
		return;
	}

	const UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(Agent->GetVehicleMovementComponent());
	if (VehicleMovement == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Trainer (32): Failed to Retrieve Vehicle Movement Component."))
		return;
	}

	// Reward going fast
	const float SpeedReward = ULearningAgentsRewards::MakeRewardFromVelocityAlongSpline(
		TrackSpline, Agent->GetActorLocation(), Agent->GetVelocity(), 1000.0f);

	// Penalize going off track
	const FVector TrackLocation = TrackSpline->FindLocationClosestToWorldLocation(Agent->GetActorLocation(), ESplineCoordinateSpace::World);
	const float OffTrackPenalty = ULearningAgentsRewards::MakeRewardOnLocationDifferenceAboveThreshold(
		Agent->GetActorLocation(), TrackLocation, OffTrackThreshold, -10.0f);

	// Penalize more than 2 collisions with other vehicles (per training cycle)
	const float CollisionPenalty = ULearningAgentsRewards::MakeReward(Agent->GetCollisionCount(), -15.0f);
	
	// Reward keeping RPMs in the sweet spot
	float GearshiftReward = 0.0f;
	if (bManualTransmission) {
		const int TargetGear = VehicleMovement->GetTargetGear();
		const float EngineRPMs = VehicleMovement->GetEngineRotationSpeed();
		GearshiftReward = ULearningAgentsRewards::MakeRewardOnCondition(EngineRPMs > DownShiftAt && EngineRPMs < UpShiftAt, TargetGear * 1.5f);
	}

	// Sum reward and assign to outward bound float
	OutReward = (SpeedReward + OffTrackPenalty + CollisionPenalty + GearshiftReward);
}

void UAutonomousCarTrainer::GatherAgentCompletion_Implementation(ELearningAgentsCompletion& OutCompletion,
	const int32 AgentId) {
	// Get reference to agent
	const AResetableVehiclePawn* Agent = Cast<AResetableVehiclePawn>(Manager->GetAgent(AgentId, AResetableVehiclePawn::StaticClass()));
	if (Agent == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Trainer (62): Casting of agent failed."));
		return;
	}
	
	// Terminate if too many collisions
	const ELearningAgentsCompletion CollisionCompletion = ULearningAgentsCompletions::MakeCompletionOnCondition(
		Agent->GetCollisionCount() > CollisionThreshold);
	if (CollisionCompletion == ELearningAgentsCompletion::Termination)
	{
		OutCompletion = ELearningAgentsCompletion::Termination;
		return;
	}
	
	// Terminate if off track too far
	const FVector TrackLocation = TrackSpline->FindLocationClosestToWorldLocation(Agent->GetActorLocation(), ESplineCoordinateSpace::World);
	const ELearningAgentsCompletion TrackCompletion = ULearningAgentsCompletions::MakeCompletionOnLocationDifferenceAboveThreshold(
		Agent->GetActorLocation(), TrackLocation, OffTrackThreshold);
	if (TrackCompletion == ELearningAgentsCompletion::Termination)
	{
		OutCompletion = ELearningAgentsCompletion::Termination;
	}
}

void UAutonomousCarTrainer::ResetAgentEpisode_Implementation(const int32 AgentId) {
	// Get reference to agent
	AResetableVehiclePawn* Agent = Cast<AResetableVehiclePawn>(Manager->GetAgent(AgentId, AResetableVehiclePawn::StaticClass()));
	if (Agent == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Trainer (76): Casting of agent failed."));
		return;
	}

	// Get all agents
	TArray<UObject*> AllObjects;
	TArray<int32> AllIds;
	Manager->GetAllAgents(AllObjects, AllIds, AResetableVehiclePawn::StaticClass());

	// Cast to array of actors
	TArray<AActor*> AllAgents;
	AllAgents.Reserve(AllObjects.Num());
	for (UObject* Other : AllObjects) {
		AllAgents.Add(StaticCast<AActor*>(Other));
	}

	// Reset to random point on track
	Agent->ResetCollisionCount();
	Agent->ResetToRandomPointOnSpline(TrackSpline, AllAgents);
}
