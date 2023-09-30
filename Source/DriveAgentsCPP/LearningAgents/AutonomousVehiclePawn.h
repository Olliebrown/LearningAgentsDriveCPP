#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "AutonomousVehiclePawn.generated.h"

class UChaosWheeledVehicleMovementComponent;
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

	/** Returns the cast Chaos Vehicle Movement subobject */
	FORCEINLINE const TObjectPtr<UChaosWheeledVehicleMovementComponent>& GetChaosVehicleMovement() const { return ChaosVehicleMovement; }

protected:
	/** Called when the brake lights are turned on or off */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle")
		void BrakeLights(bool bBraking);

	virtual void BeginPlay() override;
	virtual void Tick(float deltaSeconds) override;

	/** Called from Tick if EnableInputVisualizer is true */
	UFUNCTION(BlueprintNativeEvent, Category = "Input Visualization")
		void UpdateInputVisualization();

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

	/** Handle to a timer to auto-reset the vehicle when it flips */
	FTimerHandle ResetTimer;
};
