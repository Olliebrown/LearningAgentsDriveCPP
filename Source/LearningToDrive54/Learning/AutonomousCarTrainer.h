// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsTrainer.h"
#include "AutonomousCarTrainer.generated.h"

class USplineComponent;

/**
 * 
 */
UCLASS()
class LEARNINGTODRIVE54_API UAutonomousCarTrainer : public ULearningAgentsTrainer
{
	GENERATED_BODY()

	virtual void GatherAgentReward_Implementation(float& OutReward, const int32 AgentId) override;

	virtual void GatherAgentCompletion_Implementation(ELearningAgentsCompletion& OutCompletion, const int32 AgentId) override;

	virtual void ResetAgentEpisode_Implementation(const int32 AgentId) override;

public:
	UAutonomousCarTrainer();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
		USplineComponent* TrackSpline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
		float OffTrackThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
		int CollisionThreshold;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stick Shift")
		bool bManualTransmission;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stick Shift")
		float UpShiftAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stick Shift")
		float DownShiftAt;
};
