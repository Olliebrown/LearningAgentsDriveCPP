// Fill out your copyright notice in the Description page of Project Settings.


#include "SportsCarTrackSpline.h"

#include "LandscapeSplineActor.h"
#include "LandscapeSplinesComponent.h"

#include "Components/SplineComponent.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
ASportsCarTrackSpline::ASportsCarTrackSpline()
{
	// Set default values
	bCloseSpline = true;
	
	// Set this actor to Tick()
	PrimaryActorTick.bCanEverTick = true;

	// Initialize our spline component
	TrackSpline = CreateDefaultSubobject<USplineComponent>("Track Spline");
	SetRootComponent(TrackSpline);
}

// Called when the game starts or when spawned
void ASportsCarTrackSpline::BeginPlay()
{
	Super::BeginPlay();
	
	// Build and set the track spline
	if (!BuildTrackSplineFromLandscapeSpline())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to make the track spline object"));
	}
}

bool ASportsCarTrackSpline::BuildTrackSplineFromLandscapeSpline() const
{
	// Try to find the landscape spline actor
	const ALandscapeSplineActor* LandscapeSplineActor = Cast<ALandscapeSplineActor>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ALandscapeSplineActor::StaticClass()));
	if (LandscapeSplineActor == nullptr)
	{
		// Indicate failure
		UE_LOG(LogTemp, Error, TEXT("Failed to find Landscape Spline Actor. Unable to create Track Spline."));
		return false;
	}

	// Retrieve the landscape spline component
	ULandscapeSplinesComponent* LandscapeSpline = Cast<ULandscapeSplinesComponent>(LandscapeSplineActor->GetRootComponent());
	if (LandscapeSpline == nullptr)
	{
		// Indicate failure
		UE_LOG(LogTemp, Error, TEXT("Root component of Landscape Spline Actor is not a Landscape Splines Component (should not happen)."));
		return false;
	}

	// Copy from the landscape spline to our spline and return success
	LandscapeSpline->CopyToSplineComponent(TrackSpline);
	TrackSpline->SetClosedLoop(bCloseSpline);
	for (int i = 0; i < TrackSpline->GetNumberOfSplinePoints(); i++)
	{
		TrackSpline->SetSplinePointType(i, ESplinePointType::Type::Curve, false);
	}

	// Process the spline and fire the event
	TrackSpline->UpdateSpline();

	UE_LOG(LogTemp, Warning, TEXT("Broadcasting SplineReady event."));
	SplineReadyEvent.Broadcast();

	// Indicate success
	return true;
}
