// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsTrainer.h"
#include "AutonomousVehicleTrainer.generated.h"

// Learning agents and components forward declarations
class UPlanarPositionDifferencePenalty;
class UScalarVelocityReward;
class UPlanarPositionDifferenceCompletion;
class USplineComponentHelper;
class USplineComponent;

/**
 * 
 */
UCLASS()
class DRIVEAGENTSCPP_API UAutonomousVehicleTrainer : public ULearningAgentsTrainer
{
	GENERATED_BODY()
	
public:
	FORCEINLINE void SetTrackSpline(USplineComponent* NewTrackSpline) { TrackSpline = NewTrackSpline; }

	virtual void SetupRewards_Implementation() override;
	virtual void SetRewards_Implementation(const TArray<int32>& AgentIds) override;

	virtual void SetupCompletions_Implementation() override;
	virtual void SetCompletions_Implementation(const TArray<int32>& AgentIds) override;

	virtual void ResetEpisodes_Implementation(const TArray<int32>& AgentIds) override;

protected:
	UPlanarPositionDifferencePenalty* OffTrackPenalty;
	UScalarVelocityReward* SpeedReward;
	UPlanarPositionDifferenceCompletion* OffTrackCompletion;

	USplineComponentHelper* TrackSplineHelper;
	USplineComponent* TrackSpline;
};
