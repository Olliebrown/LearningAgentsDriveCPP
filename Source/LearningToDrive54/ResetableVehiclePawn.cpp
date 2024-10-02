// Fill out your copyright notice in the Description page of Project Settings.


#include "ResetableVehiclePawn.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "LearningAgentsManager.h"
#include "Components/SplineComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"


AResetableVehiclePawn::AResetableVehiclePawn() {
	MaxRetries = 100;
	LocationJitter = 1200.0f;
	YawJitter = 90.0f;
	ResetHeight = 50.0f;

	bFoundManager = false;
	AgentID = -1;
}

void AResetableVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	// Setup hit callback
	GetMesh()->OnComponentHit.AddDynamic(this, &AResetableVehiclePawn::OnHitVehicle);
	
	// Register with any existing Managers
	TArray<AActor*> Managers;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), "LearningAgentsManager", Managers);
	for (const AActor* CurActor : Managers)
	{
		ULearningAgentsManager* Manager = Cast<ULearningAgentsManager>(
			CurActor->GetComponentByClass(ULearningAgentsManager::StaticClass()));
		if (Manager != nullptr)
		{
			AgentID = Manager->AddAgent(this);
			bFoundManager = true;
		}
	}

	if (!bFoundManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Vehicle Pawn: Failed to find agent manager (did you remember to add to scene and tag it?)"));
	}
}

void AResetableVehiclePawn::OnHitVehicle(UPrimitiveComponent* MyComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (const AResetableVehiclePawn* OtherVehicle = Cast<AResetableVehiclePawn>(OtherActor); OtherVehicle != nullptr)
	{
		CollisionCount++;
	}
}

void AResetableVehiclePawn::ResetToRandomPointOnSpline(USplineComponent* Spline, const TArray<AActor*>& Agents)
{
	// Sanity check
	if (Spline == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Resetable Vehicle (23): Spline component is null."))
		return;
	}

	// Initialize local variables
	bool InvalidTransform = true;
	FTransform CandidateTransform;
	int Attempts = 0;
	
	while (InvalidTransform && Attempts <= MaxRetries) {
		// Increment attempts (to avoid infinite loop)
		Attempts++;
		
		// Compute random offsets for jittering
		const FVector RandomOffset = FVector(
			(UKismetMathLibrary::RandomFloat() - 0.5f) * LocationJitter,
			(UKismetMathLibrary::RandomFloat() - 0.5f) * LocationJitter,
			ResetHeight);
		const float YawOffset = (UKismetMathLibrary::RandomFloat() - 0.5f) * YawJitter;

		// Compute random location and rotation on spline
		const float RandomDistance = Spline->GetSplineLength() * UKismetMathLibrary::RandomFloat();
		const FVector RandomLocation = Spline->GetLocationAtDistanceAlongSpline(RandomDistance, ESplineCoordinateSpace::World);
		const FRotator RandomRotation = Spline->GetRotationAtDistanceAlongSpline(RandomDistance, ESplineCoordinateSpace::World);
		
		// Combine into a transform object
		CandidateTransform = FTransform(
			FRotator(RandomRotation.Pitch, RandomRotation.Yaw + YawOffset, RandomRotation.Roll),
			RandomLocation + RandomOffset);
		
		// Check proximity to all other agents
		InvalidTransform = false;
		for (UObject* Agent : Agents) {
			const AActor* AgentActor = Cast<AActor>(Agent);
			if (AgentActor != nullptr && AgentActor != this) {
				if (FVector::Distance(CandidateTransform.GetLocation(), AgentActor->GetActorLocation()) < 1500.0) {
					InvalidTransform = true;
					break;
				}
			}
		}
	}

	if (Attempts > MaxRetries)
	{
		UE_LOG(LogTemp, Error, TEXT("Reset failed due to too many retries"));
		return;
	}

	// Halt all current movement then reset transform
	UPrimitiveComponent* PhysicsComponent = Cast<UPrimitiveComponent>(this->GetRootComponent());
	if (PhysicsComponent != nullptr) {
		PhysicsComponent->SetPhysicsAngularVelocityInDegrees(FVector::Zero());
		PhysicsComponent->SetPhysicsLinearVelocity(FVector::Zero());

		PhysicsComponent->SetWorldTransform(CandidateTransform, false, nullptr, ETeleportType::TeleportPhysics);
		GetVehicleMovementComponent()->SetTargetGear(1, true);
	} else {
		UE_LOG(LogTemp, Error, TEXT("Resetable Vehicle (84): Failed to retrieve Primitive Component."))
	}
}
