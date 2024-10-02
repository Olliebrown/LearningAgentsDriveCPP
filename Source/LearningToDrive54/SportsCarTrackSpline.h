// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SportsCarTrackSpline.generated.h"

// Forward dec for traditional components
class USplineComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSplineReady);

UCLASS()
class LEARNINGTODRIVE54_API ASportsCarTrackSpline : public AActor
{
	GENERATED_BODY()

protected:
	/** Spline component for the track */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Spline", meta = (AllowPrivateAccess = "true"))
		USplineComponent* TrackSpline;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Builds the spline component from an existing landscape spline
	bool BuildTrackSplineFromLandscapeSpline() const;
	
public:	
	// Sets default values for this actor's properties
	ASportsCarTrackSpline();

	UPROPERTY(BlueprintAssignable)
		FSplineReady SplineReadyEvent;
	
	/** Flag for when the spline is initialized */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track Spline")
		bool bCloseSpline;
};
