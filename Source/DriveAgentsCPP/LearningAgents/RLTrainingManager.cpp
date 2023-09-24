// Fill out your copyright notice in the Description page of Project Settings.


#include "RLTrainingManager.h"

#include "AutonomousVehiclePawn.h"
#include "AutonomousVehicleInteractor.h"
#include "AutonomousVehiclePolicy.h"
#include "AutonomousVehicleTrainer.h"

#include "LandscapeSplineActor.h"
#include "LandscapeSplinesComponent.h"

#include "Components/SplineComponent.h"

#include "Kismet/GameplayStatics.h"


ARLTrainingManager::ARLTrainingManager()
{
	// Initialize ticking rate
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;

	// Instantiate all the needed components
	UE_LOG(LogTemp, Warning, TEXT("Initializing learning manager components"));
	AutonomousInteractor = CreateDefaultSubobject<UAutonomousVehicleInteractor>("Agent Interactor");
	AutonomousPolicy = CreateDefaultSubobject<UAutonomousVehiclePolicy>("Agent Policy");
	AutonomousTrainer = CreateDefaultSubobject<UAutonomousVehicleTrainer>("Agent Trainer");

	// Initialize our spline component
	TrackSpline = CreateDefaultSubobject<USplineComponent>("Track Spline");
	SetRootComponent(TrackSpline);

	// Initialize remaining members
	NeuralNetworkWeights = nullptr;
	AutonomousCritic = nullptr;
}

void ARLTrainingManager::BeginPlay()
{
	Super::BeginPlay();

	// Check if a data asset exists for the neural network weights
	if (NeuralNetworkWeights == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("WARNING: Manager has no data asset for neural network weights! (training data won't persist)"))
	}

	// Build and set the track spline for all the child components
	if (BuildTrackSplineFromLandscapeSpline())
	{
		AutonomousInteractor->SetTrackSpline(TrackSpline);
		AutonomousPolicy->SetTrackSpline(TrackSpline);
		AutonomousTrainer->SetTrackSpline(TrackSpline);
	}

	// Initialize the learning assets in order
	UE_LOG(LogTemp, Warning, TEXT("Running Component setups"));
	AutonomousInteractor->SetupInteractor();
	AutonomousPolicy->SetupPolicy(AutonomousInteractor, AutonomousPolicySettings, NeuralNetworkWeights);
	AutonomousTrainer->SetupTrainer(AutonomousInteractor, AutonomousPolicy, AutonomousCritic, AutonomousTrainerSettings);

	// Scatter the Agents around the track for the first training session
	TArray<AActor*> AgentVehicles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAutonomousVehiclePawn::StaticClass(), AgentVehicles);
	for (AActor* AgentActor : AgentVehicles)
	{
		AAutonomousVehiclePawn* AgentVehicle = Cast<AAutonomousVehiclePawn>(AgentActor);
		if (AgentVehicle != nullptr)
		{
			AgentVehicle->SetEnableInputVisualizer(true);
			AgentVehicle->ResetToRandomPointOnSpline(TrackSpline);
		}
	}
}

void ARLTrainingManager::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);
	AutonomousTrainer->RunTraining(TrainingSettings, TrainerGameSettings, TrainerPathSettings, CriticSettings, false, true, true);
}

bool ARLTrainingManager::BuildTrackSplineFromLandscapeSpline()
{
	ALandscapeSplineActor* LandscapeSplineActor = Cast<ALandscapeSplineActor>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ALandscapeSplineActor::StaticClass())
	);
	if (LandscapeSplineActor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find Landscape Spline Actor. Unable to create Track Spline."));
		return false;
	}

	ULandscapeSplinesComponent* LandscapeSpline = Cast<ULandscapeSplinesComponent>(LandscapeSplineActor->GetRootComponent());
	if (LandscapeSpline == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Root component of Landscape Spline Actor is not a Landscape Splines Component (should not happen)."));
		return false;
	}

	// Copy to our spline and return success
	LandscapeSpline->CopyToSplineComponent(TrackSpline);
	TrackSpline->SetClosedLoop(true);
	return true;
}
