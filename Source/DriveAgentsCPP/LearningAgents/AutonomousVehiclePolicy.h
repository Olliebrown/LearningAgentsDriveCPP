// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsPolicy.h"
#include "AutonomousVehiclePolicy.generated.h"

/**
 * 
 */
UCLASS()
class DRIVEAGENTSCPP_API UAutonomousVehiclePolicy : public ULearningAgentsPolicy
{
	GENERATED_BODY()

public:
	FORCEINLINE void SetTrackSpline(USplineComponent* NewTrackSpline) { TrackSpline = NewTrackSpline; }

protected:
	// The actual track spline
	USplineComponent* TrackSpline;
};
