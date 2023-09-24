// Fill out your copyright notice in the Description page of Project Settings.

#include "AutonomousVehiclePawn.h"

#include "Kismet/GameplayStatics.h"
#include "LearningAgentsManager.h"

#include "../DriveAgentsCPPWheelFront.h"
#include "../DriveAgentsCPPWheelRear.h"

#include "Components/SplineComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"

AAutonomousVehiclePawn::AAutonomousVehiclePawn()
{
	// Configure the car mesh physics
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Ignore); // Don't collide with other cars

	// Get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

	// Create and attach the input visualizer components
	InputVisualizer = CreateDefaultSubobject<USceneComponent>("Input Visualizer");
	InputArrow = CreateDefaultSubobject<UArrowComponent>("Input Arrow");
	InputVisualizer->SetupAttachment(GetMesh());
	InputArrow->SetupAttachment(InputVisualizer);

	// Initialize other members
	AgentId = -1;
	ForwardColor = FColor(0U, 152U, 42U);
	BackwardColor = FColor(137U, 0U, 19U);

	// Initialize input visualizer transform properties
	InputArrow->SetRelativeLocation(FVector(0.0f, 0.0f, 160.0f));
	InputArrow->SetHiddenInGame(false);
	InputArrow->ArrowColor = ForwardColor;
	InputArrow->ArrowSize = 10.0f;
	InputArrow->ArrowLength = 80.0f;

	// Use setter to get proper side-effects
	SetEnableInputVisualizer(false);
}

void AAutonomousVehiclePawn::ResetToRandomPointOnSpline(USplineComponent* TrackSpline)
{
	// Sanity check
	if (TrackSpline == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot move to random point on null spline. Did you forget to assign track spline?"));
		return;
	}

	// Pick random position along the spline (uniformly distant from start)
	float randomLength = TrackSpline->GetSplineLength() * FMath::FRand();
	FVector randomPosition = TrackSpline->GetLocationAtDistanceAlongSpline(randomLength, ESplineCoordinateSpace::World);

	// Shift from center of track and raise up a bit off the ground
	randomPosition += FVector(FMath::FRandRange(-600.0f, 600.0f), FMath::FRandRange(-600.0f, 600.0f), 50.0f);

	// Randomize heading within +-45 degrees
	float trackYaw = TrackSpline->GetRotationAtDistanceAlongSpline(randomLength, ESplineCoordinateSpace::World).Yaw;
	FRotator randomHeading = FRotator(0.0f, trackYaw + FMath::FRandRange(-45.0f, 45.0f), 0.0f);

	// DEBUG: Print the position and heading as it randomizes
	// UE_LOG(LogTemp, Warning,
	//	 TEXT("Position: (%.2f, %.2f, %.2f) / Heading: (%.2f, %.2f, %.2f)"),
	//	 randomPosition.X, randomPosition.Y, randomPosition.Z,
	//	 randomHeading.Yaw, randomHeading.Pitch, randomHeading.Roll
	// );

	// Apply transform and teleport actor
	if (!SetActorTransform(FTransform(randomHeading, randomPosition, FVector::One()), false, nullptr, ETeleportType::TeleportPhysics)) {
		UE_LOG(LogTemp, Warning, TEXT("Failed to randomize actor locatoin"));
	}

	// Stop all physics velocities on primary mesh
	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::Zero());
	GetMesh()->SetPhysicsLinearVelocity(FVector::Zero());
}

void AAutonomousVehiclePawn::SetEnableInputVisualizer(bool bNewEnableInputVisualizer)
{
	// Update member variable and propegate to component visibility
	bEnableInputVisualizer = bNewEnableInputVisualizer;
	InputVisualizer->SetHiddenInGame(!bEnableInputVisualizer, true);
	InputVisualizer->SetVisibility(bEnableInputVisualizer, true);
}

void AAutonomousVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	// Attempt to retrieve a learning manager from the scene
	ALearningAgentsManager* learningManager = Cast<ALearningAgentsManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ALearningAgentsManager::StaticClass())
	);
	if (learningManager != nullptr)
	{
		// Add this vehicle as an agent to the learning manager
		AgentId = learningManager->AddAgent(this);
	}
	else
	{
		// Output a warning
		UE_LOG(LogTemp, Warning, TEXT("Autonomous Vehicle Pawn did not find a learning manager in the scene. Did you forget to add one?"));
	}
}

void AAutonomousVehiclePawn::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	// Update the input visualizer if it is enabled
	if (bEnableInputVisualizer)
	{
		UpdateInputVisualization();
	}
}

void AAutonomousVehiclePawn::UpdateInputVisualization_Implementation()
{
	// Get overall throttle value
	float ScaledThrottle = (ChaosVehicleMovement->GetThrottleInput() - ChaosVehicleMovement->GetBrakeInput());

	// DEBUG: Check values computed for ScaledThrottle
	//UE_LOG(LogTemp, Warning, TEXT("Throttle: %.2f / Brake: %.2f / Combined: %.2f"),
	//	ChaosVehicleMovement->GetThrottleInput(),
	//	ChaosVehicleMovement->GetBrakeInput(),
	//	ScaledThrottle
	//);

	// Set arrow length and color (NOTE: Color does not seem to be updating when rendered?)
	InputArrow->ArrowColor = (ScaledThrottle >= 0.0f ? ForwardColor : BackwardColor);
	InputArrow->SetRelativeScale3D(FMath::Sign(ScaledThrottle) * FVector(FMath::Abs(ScaledThrottle), 1.0, 1.0));

	// Get steering input as angle in degrees
	float SteeringAngle = ChaosVehicleMovement->GetSteeringInput() * 90.0f * FMath::Sign(ScaledThrottle);

	// Update Yaw of the input visualizer root
	FRotator SteeringRotation = InputVisualizer->GetRelativeRotation();
	SteeringRotation.Yaw = SteeringAngle;
	InputVisualizer->SetRelativeRotation(SteeringRotation);
}
