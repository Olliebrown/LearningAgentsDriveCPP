// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsInteractor.h"
#include "LearningAgentsObservations.h"
#include "AutonomousVehicleInteractor.generated.h"

// Learning agents and components forward declarations
class UPlanarPositionObservation;
class UPlanarDirectionObservation;
class UAngleObservation;
class UPlanarVelocityObservation;
class UFloatAction;
class USplineComponentHelper;
class USplineComponent;

/**
 * 
 */
UCLASS()
class DRIVEAGENTSCPP_API UAutonomousVehicleInteractor : public ULearningAgentsInteractor
{
	GENERATED_BODY()
	
public:
	UAutonomousVehicleInteractor(FVTableHelper& Helper);

	FORCEINLINE void SetTrackSpline(USplineComponent* NewTrackSpline) { TrackSpline = NewTrackSpline; }

	// Observation events
	virtual void SetupObservations_Implementation() override;
	virtual void SetObservations_Implementation(const TArray<int32>& AgentIds) override;

	// Action events
	virtual void SetupActions_Implementation() override;
	virtual void GetActions_Implementation(const TArray<int32>& AgentIds) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		int LookAheadObservationCount = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
		float LookAheadDistance = 500.0f;

	// Observations
	UPlanarPositionObservation* TrackPositionObservation;
	UPlanarDirectionObservation* TrackDirectionObservation;

	UPlanarPositionArrayObservation* TrackLookAheadPositionArrayObservations;
	UPlanarDirectionArrayObservation* TrackLookAheadDirectionArrayObservations;

	UAngleObservation* TrackPositionParameterObservation;
	UPlanarVelocityObservation* CarVelocityObservation;

	// Actions
	UFloatAction* ThrottleBrakeAction;
	UFloatAction* SteeringAction;

	// Helper for the track spline
	USplineComponentHelper* TrackSplineHelper;

	// The actual track spline
	USplineComponent* TrackSpline;
};
