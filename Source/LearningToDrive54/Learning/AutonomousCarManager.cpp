// Fill out your copyright notice in the Description page of Project Settings.


#include "AutonomousCarManager.h"
#include "LearningAgentsManager.h"

#include "Kismet/GameplayStatics.h"

#include "../ResetableVehiclePawn.h"
#include "AutonomousCarManagerComponent.h"
#include "AutonomousCarInteractor.h"
#include "AutonomousCarTrainer.h"

AAutonomousCarManager::AAutonomousCarManager()
{
	LearningAgentsManager = CreateDefaultSubobject<UAutonomousCarManagerComponent>("Learning Agents Manager");
}

void AAutonomousCarManager::BeginPlay()
{
	Super::BeginPlay();
	OnSplineReady();
}

void AAutonomousCarManager::OnSplineReady()
{
	// Spline is ready so do the initialization
	InitializeAgents();
	InitializeManager();
}

void AAutonomousCarManager::InitializeAgents()
{
	// Get all the vehicles as agents
	TArray<AActor*> Agents;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResetableVehiclePawn::StaticClass(), Agents);

	for (AActor* Agent : Agents)
	{
		// Make sure manager ticks first
		Agent->AddTickPrerequisiteActor(this);

		// If in inference mode, move to random spot now
		if (RunMode == EManagerModeEnum::InferenceMode)
		{
			if (AResetableVehiclePawn* VehiclePawn = Cast<AResetableVehiclePawn>(Agent); VehiclePawn != nullptr)
			{
				VehiclePawn->ResetToRandomPointOnSpline(TrackSpline, Agents);
			}
		}
	}
}

void AAutonomousCarManager::InitializeManager()
{
	// Should neural networks be re-initialized
	const bool ReInitialize = (RunMode == EManagerModeEnum::ReInitialize);
	
	// Make Interactor Instance
	Interactor = Cast<UAutonomousCarInteractor>(ULearningAgentsInteractor::MakeInteractor(
		LearningAgentsManager, UAutonomousCarInteractor::StaticClass(), "Autonomous Car Interactor"));
	if (Interactor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Autonomous Car Manager: Failed to make interactor object."));
		return;
	}

	Interactor->TrackSpline = TrackSpline;
	Interactor->bManualTransmission = bManualTransmission;

	// Make Policy Instance
	Policy = ULearningAgentsPolicy::MakePolicy(LearningAgentsManager, Interactor,
		ULearningAgentsPolicy::StaticClass(), "Learning Agents Policy",
		EncoderNeuralNetwork, PolicyNeuralNetwork,DecoderNeuralNetwork,
		ReInitialize, ReInitialize, ReInitialize,
		PolicySettings, RandomSeed);
	if (Policy == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Autonomous Car Manager: Failed to make policy object."));
		return;
	}

	// Make Critic Instance
	Critic = ULearningAgentsCritic::MakeCritic(LearningAgentsManager, Interactor, Policy,
		ULearningAgentsCritic::StaticClass(), "Learning Agents Critic",
		CriticNeuralNetwork, ReInitialize, CriticSettings, RandomSeed);
	if (Critic == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Autonomous Car Manager: Failed to make critic object."));
		return;
	}

	// Make Trainer Instance
	Trainer = Cast<UAutonomousCarTrainer>(ULearningAgentsTrainer::MakeTrainer(
		LearningAgentsManager, Interactor, Policy, Critic,
		UAutonomousCarTrainer::StaticClass(), "Autonomous Car Trainer",
		TrainerSettings));
	if (Trainer == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Autonomous Car Manager: Failed to make autonomous car trainer object."));
		return;
	}

	Trainer->TrackSpline = TrackSpline;
	Trainer->bManualTransmission = bManualTransmission;
}

void AAutonomousCarManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (RunMode == EManagerModeEnum::InferenceMode)
	{
		if (Policy != nullptr)
		{
			Policy->RunInference();
		}
	}
	else
	{
		if (Trainer != nullptr)
		{
			Trainer->RunTraining(TrainerTrainingSettings, TrainerGameSettings, TrainerPathSettings,
				true, true);
		}
	}	
}
