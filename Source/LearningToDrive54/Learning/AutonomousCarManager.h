// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../SportsCarTrackSpline.h"

#include "LearningAgentsPolicy.h"
#include "LearningAgentsCritic.h"
#include "LearningAgentsTrainer.h"
#include "ManagerModeEnum.h"

#include "AutonomousCarManager.generated.h"

class UAutonomousCarManagerComponent;
class UAutonomousCarInteractor;
class UAutonomousCarTrainer;
class ULearningAgentsNeuralNetwork;

/**
 * 
 */
UCLASS()
class LEARNINGTODRIVE54_API AAutonomousCarManager : public ASportsCarTrackSpline
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UAutonomousCarManagerComponent* LearningAgentsManager;

	// Objects managed by the learning manager
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	UAutonomousCarInteractor* Interactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	ULearningAgentsPolicy* Policy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	ULearningAgentsCritic* Critic;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Learning Objects")
	UAutonomousCarTrainer* Trainer;

	// Respond to spline ready event
	UFUNCTION()
	void OnSplineReady();

	// Internal initialization functions
	void InitializeAgents();
	void InitializeManager();

public:
	AAutonomousCarManager();
	
	virtual void Tick(float DeltaSeconds) override;

	// Settings controlling Manager behavior
	UPROPERTY(EditAnywhere, Category = "Manager Settings")
	TEnumAsByte<EManagerModeEnum> RunMode;

	UPROPERTY(EditAnywhere, Category = "Manager Settings")
	bool bManualTransmission;

	UPROPERTY(EditAnywhere, Category = "Manager Settings")
	int RandomSeed;

	// Settings passed to the learning objects
	UPROPERTY(EditAnywhere, Category = "Learning Settings")
	FLearningAgentsPolicySettings PolicySettings;

	UPROPERTY(EditAnywhere, Category = "Learning Settings")
	FLearningAgentsCriticSettings CriticSettings;

	UPROPERTY(EditAnywhere, Category = "Learning Settings")
	FLearningAgentsTrainerSettings TrainerSettings;

	UPROPERTY(EditAnywhere, Category = "Learning Settings")
	FLearningAgentsTrainerTrainingSettings TrainerTrainingSettings;

	UPROPERTY(EditAnywhere, Category = "Learning Settings")
	FLearningAgentsTrainerGameSettings TrainerGameSettings;

	UPROPERTY(EditAnywhere, Category = "Learning Settings")
	FLearningAgentsTrainerPathSettings TrainerPathSettings;

	// References to the nural network data assets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neural Networks")
	ULearningAgentsNeuralNetwork* EncoderNeuralNetwork;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neural Networks")
	ULearningAgentsNeuralNetwork* PolicyNeuralNetwork;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neural Networks")
	ULearningAgentsNeuralNetwork* DecoderNeuralNetwork;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neural Networks")
	ULearningAgentsNeuralNetwork* CriticNeuralNetwork;
};
