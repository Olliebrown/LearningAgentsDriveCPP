// Fill out your copyright notice in the Description page of Project Settings.

#include "AutonomousVehiclePawn.h"

#include "Kismet/GameplayStatics.h"
#include "LearningAgentsManager.h"

#include "../DriveAgentsCPPWheelFront.h"
#include "../DriveAgentsCPPWheelRear.h"

#include "LandscapeSplineActor.h"

#include "Components/SplineComponent.h"
#include "Components/ArrowComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"

AAutonomousVehiclePawn::AAutonomousVehiclePawn()
{
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 110.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = true;
	FrontCamera->SetConstraintAspectRatio(true);
	FrontCamera->SetAspectRatio(16.0f / 9.0f);
	FrontCamera->SetFieldOfView(45.0f);

	//// construct the back camera boom
	//BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	//BackSpringArm->SetupAttachment(GetMesh());
	//BackSpringArm->TargetArmLength = 650.0f;
	//BackSpringArm->SocketOffset.Z = 150.0f;
	//BackSpringArm->bDoCollisionTest = false;
	//BackSpringArm->bInheritPitch = false;
	//BackSpringArm->bInheritRoll = false;
	//BackSpringArm->bEnableCameraRotationLag = true;
	//BackSpringArm->CameraRotationLagSpeed = 2.0f;
	//BackSpringArm->CameraLagMaxDistance = 50.0f;

	//BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	//BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh physics
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

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

	// Prepare to search for valid spot along racetrack
	float randomLength = 0;
	FVector randomPosition = FVector();
	bool foundTrack = false;

	// Is there a racetrack beneath this point?
	FHitResult Hit;
	while (!foundTrack)
	{
		// Pick random position along the spline (uniformly distant from start)
		randomLength = TrackSpline->GetSplineLength() * FMath::FRand();
		randomPosition = TrackSpline->GetLocationAtDistanceAlongSpline(randomLength, ESplineCoordinateSpace::World);

		// Shift from center of track and raise up a bit off the ground
		randomPosition += FVector(FMath::FRandRange(-400.0f, 400.0f), FMath::FRandRange(-TrackWidth, TrackWidth), 200.0f);

		if (GetWorld()->LineTraceSingleByChannel(Hit, randomPosition + FVector(0, 0, 1000), randomPosition - FVector(0, 0, 1000),
			ECC_Visibility, FCollisionQueryParams("DebugTraceTag", true, this)))
		{
			if (Cast<ALandscapeSplineActor>(Hit.GetActor()) != nullptr) {
				foundTrack = true;
			}
		}
	}

	// Randomize heading within +-45 degrees
	float trackYaw = TrackSpline->GetRotationAtDistanceAlongSpline(randomLength, ESplineCoordinateSpace::World).Yaw;
	FRotator randomHeading = FRotator(0.0f, trackYaw + FMath::FRandRange(-60.0f, 60.0f), 0.0f);

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

	// If disabled, self-destruct
	if (DisableOnPlay) {
		this->Destroy();
		return;
	}

	// Setup overlap tracking
	GetMesh()->OnComponentBeginOverlap.AddDynamic(this, &AAutonomousVehiclePawn::BeginOverlap);
	GetMesh()->OnComponentEndOverlap.AddDynamic(this, &AAutonomousVehiclePawn::EndOverlap);

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

	// If we have flipped, make sure the reset timer is running
	if (!ResetTimer.IsValid() && GetActorUpVector().Z < 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			ResetTimer, this, &AAutonomousVehiclePawn::ResetVehicle, 5.0, true, -1.0f
		);
	}
	// If we are right-side up, make sure the timer is cleared
	else if(ResetTimer.IsValid() && GetActorUpVector().Z >= 0.0f)
	{
		GetWorldTimerManager().ClearTimer(ResetTimer);
		ResetTimer = FTimerHandle();
	}

	// Update the input visualizer if it is enabled
	if (bEnableInputVisualizer)
	{
		UpdateInputVisualization();
	}
}

void AAutonomousVehiclePawn::BeginOverlap(UPrimitiveComponent* Component, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAutonomousVehiclePawn* OtherAgent = Cast<AAutonomousVehiclePawn>(OtherActor);
	if (OtherAgent != nullptr && !OverlappingAgents.Contains(OtherAgent->GetAgentId()))
	{
		OverlappingAgents.Add(OtherAgent->GetAgentId());
	}
}

void AAutonomousVehiclePawn::EndOverlap(UPrimitiveComponent* Component, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	AAutonomousVehiclePawn* OtherAgent = Cast<AAutonomousVehiclePawn>(OtherActor);
	if (OtherAgent != nullptr && OverlappingAgents.Contains(OtherAgent->GetAgentId()))
	{
		OverlappingAgents.Remove(OtherAgent->GetAgentId());
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

void AAutonomousVehiclePawn::ResetVehicle()
{
	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 200.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;

	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
}
