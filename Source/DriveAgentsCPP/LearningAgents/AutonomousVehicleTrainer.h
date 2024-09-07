// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsTrainer.h"
#include "AutonomousVehicleTrainer.generated.h"

// Learning agents and components forward declarations
class UConditionalReward;
class UPlanarPositionDifferencePenalty;
class UScalarVelocityReward;
class UPositionArraySimilarityReward;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
		int32 AgentProximityCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
		float TrackWidth = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
		float OffCenterPenaltyWeight = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
		float SpeedRewardWeight = 1.0f;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	//	float AgentProximityPenaltyWeight = -0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
		float AgentProximityMaxDistance = 999999.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
		float OverlapPenaltyWeight = -0.5f;

	// Used internally to compute agent proximity
	TArray<FVector> FindClosestAgents(const TArray<int32>& AgentIds, int32 CurId, FVector relativePosition) const;

	UPlanarPositionDifferencePenalty* OffTrackPenalty;
	UScalarVelocityReward* SpeedReward;
	UConditionalReward* OverlapPenalty;
	//UPositionArraySimilarityReward* AgentProximityPenalty;

	UPlanarPositionDifferenceCompletion* OffTrackCompletion;

	USplineComponentHelper* TrackSplineHelper;
	USplineComponent* TrackSpline;
};
