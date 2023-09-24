// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsManager.h"
#include "RLTrainingManager.generated.h"

// Different Learning Agent Manager Components
class UAutonomousVehicleInteractor;
class UAutonomouseVehiclePolicy;
class UAutonomouseVehicleTrainer;

// Various Learning Agent setting structs
struct FLearningAgentsTrainerTrainingSettings;
struct FLearningAgentsTrainerGameSettings;
struct FLearningAgentsTrainerPathSettings;
struct FLearningAgentsCriticSettings;

// Forward dec for traditional components
class USplineComponent;

/**
 * A reinforcement learning manager that can train vehicles from the default vehicle template how to drive. Based on the tutorial available from Epic at:
 * https://dev.epicgames.com/community/learning/courses/M3D/unreal-engine-learning-agents-getting-started/8OWY/unreal-engine-learning-agents-introduction
 */
UCLASS()
class DRIVEAGENTSCPP_API ARLTrainingManager : public ALearningAgentsManager
{
	GENERATED_BODY()

	/** Autonomous Vehicle Interactor */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Components", meta = (AllowPrivateAccess = "true"))
		UAutonomousVehicleInteractor* AutonomousInteractor;

	/** Autonomous Vehicle Policy */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Components", meta = (AllowPrivateAccess = "true"))
		UAutonomousVehiclePolicy* AutonomousPolicy;

	/** Autonomous Vehicle Trainer */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Components", meta = (AllowPrivateAccess = "true"))
		UAutonomousVehicleTrainer* AutonomousTrainer;

	/** Spline component for the track */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Spline", meta = (AllowPrivateAccess = "true"))
		USplineComponent* TrackSpline;

public:
	ARLTrainingManager();

	/** Training settings used by the Trainer and passed to RunTraining(). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning Settings")
		FLearningAgentsTrainerTrainingSettings TrainingSettings;

	/** Trainer Game settings used by the Trainer and passed to RunTraining(). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning Settings")
		FLearningAgentsTrainerGameSettings TrainerGameSettings;
	
	/** Trainer Path settings used by the Trainer and passed to RunTraining(). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning Settings")
		FLearningAgentsTrainerPathSettings TrainerPathSettings;
	
	/** Critic settings used by the Trainer and passed to RunTraining(). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning Settings")
		FLearningAgentsCriticSettings CriticSettings;

protected:
	// Basic lifecycle events
	virtual void BeginPlay() override;
	virtual void Tick(float deltaSeconds) override;

	// Internal method for converting the Landscape Spline to a USplineComponent
	bool BuildTrackSplineFromLandscapeSpline();

	/** Reinforcement Learning Policy Settings passed to SetupPolicy() .*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning Settings")
		FLearningAgentsPolicySettings AutonomousPolicySettings;

	/** Reinforcement Learning Trainer Settings passed to SetupTrainer(). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning Settings")
		FLearningAgentsTrainerSettings AutonomousTrainerSettings;

	/** Optional Learning Agent Critic passed to SetupTrainer() (null by default). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning Settings")
		ULearningAgentsCritic* AutonomousCritic;

	/** Data Asset for Neural Network Weights (be sure to assign to a DataAsset in your project content). */
	UPROPERTY(EditAnywhere, Category = "Learning Data")
		ULearningAgentsNeuralNetwork* NeuralNetworkWeights;
};
