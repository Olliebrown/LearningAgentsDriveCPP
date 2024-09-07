#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "AutonomousVehiclePawn.generated.h"

class UChaosWheeledVehicleMovementComponent;
class UPrimitiveComponent;
class USpringArmComponent;
class UCameraComponent;
class USplineComponent;
class UArrowComponent;

/**
 *  Autonomous Vehicle Pawn class
 *  Same as the Vehicle Pawn but with all camera and input components
 *  removed. Intended only for AI or LearningAgent control.
 */
UCLASS()
class AAutonomousVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

protected:
	/** Spring Arm for the front camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* FrontSpringArm;

	/** Front Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FrontCamera;

	///** Spring Arm for the back camera */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	//	USpringArmComponent* BackSpringArm;

	///** Back Camera component */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	//	UCameraComponent* BackCamera;

	/** Scene Component to hold our input visualization pieces */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Visualization", meta = (AllowPrivateAccess = "true"))
		USceneComponent* InputVisualizer;

	/** Scene Component to hold our input visualization pieces */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Visualization", meta = (AllowPrivateAccess = "true"))
		UArrowComponent* InputArrow;

	/** Cast pointer to the Chaos Vehicle movement component */
	TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

public:
	AAutonomousVehiclePawn();

	/** Move the vehicle to a random point along the given spline */
	void ResetToRandomPointOnSpline(USplineComponent* TrackSpline);

	/** Enable or disable the input visualizer (will also hide or show the visualizer components when called) */
	void SetEnableInputVisualizer(bool bNewEnableInputVisualizer);

	/** Right the vehicle after it has flipped. */
	void ResetVehicle();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters")
		FORCEINLINE int32 GetAgentId() const { return AgentId; }

	/** Retrieve reference to the first person camera */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CarCamera")
		FORCEINLINE UCameraComponent* GetFrontCamera() const { return FrontCamera;  }

	///** Retrieve reference to the third person camera */
	//UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CarCamera")
	//	UCameraComponent* GetBackCamera() const { return BackCamera; }

	/** Returns the cast Chaos Vehicle Movement subobject */
	FORCEINLINE const TObjectPtr<UChaosWheeledVehicleMovementComponent>& GetChaosVehicleMovement() const { return ChaosVehicleMovement; }

	/** Returns the number of currently overlapping autonomouse agents */
	FORCEINLINE int32 GetOverlappingAgentCount() const { return OverlappingAgents.Num(); }

protected:
	// Basic Actor lifecycle events
	virtual void BeginPlay() override;
	virtual void Tick(float deltaSeconds) override;

	// Callbacks for the overlap delegates on the mesh
	UFUNCTION()
		void BeginOverlap(UPrimitiveComponent* Component, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void EndOverlap(UPrimitiveComponent* Component, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	/** Called when the brake lights are turned on or off */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle")
		void BrakeLights(bool bBraking);

	/** Called from Tick if EnableInputVisualizer is true */
	UFUNCTION(BlueprintNativeEvent, Category = "Input Visualization")
		void UpdateInputVisualization();

	/** Set to true to remove this agents during begin play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
		bool DisableOnPlay = false;

	/** Used to randomize position around the track */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
		float TrackWidth = 400.0f;

	/** Enable or disable the visualization of throttle/brake input and steering */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Visualization")
		bool bEnableInputVisualizer;

	/** Material color to use when the input is forward (positive throttle, zero brake) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Visualization")
		FColor ForwardColor;

	/** Material color to use when the input is backward (negative throttle, positive brake) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Visualization")
		FColor BackwardColor;

	/** Identifier after this agent is registered with the learning manager */
	int32 AgentId;

	/** Unique list of overlapping agents */
	TSet<int32> OverlappingAgents;

	/** Handle to a timer to auto-reset the vehicle when it flips */
	FTimerHandle ResetTimer;
};
