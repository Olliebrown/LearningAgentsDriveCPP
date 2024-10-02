// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "ResetableVehiclePawn.generated.h"

class USplineComponent;

/**
 * 
 */
UCLASS()
class LEARNINGTODRIVE54_API AResetableVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Reset")
		int MaxRetries;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Reset")
		float LocationJitter;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Reset")
		float YawJitter;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Reset")
		float ResetHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Agent Settings")
		int32 AgentID;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Agent Settings")
		bool bFoundManager;

	float CollisionCount;

	void OnHitVehicle (UPrimitiveComponent* MyComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
	
public:
	AResetableVehiclePawn();
	
	virtual void BeginPlay() override;

	float GetCollisionCount() const { return CollisionCount; }
	void DecreaseCollisionCount(const float X) { CollisionCount = FMath::Max(0.0f, CollisionCount - X); }
	void ResetCollisionCount() { CollisionCount = 0.0f; }

	UFUNCTION(BlueprintCallable, Category = "Reset")
		void ResetToRandomPointOnSpline(USplineComponent* Spline, const TArray<AActor*>& Agents);
};
