// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsInteractor.h"
#include "AutonomousCarInteractor.generated.h"

/**
 * 
 */
UCLASS()
class LEARNINGTODRIVE54_API UAutonomousCarInteractor : public ULearningAgentsInteractor
{
	GENERATED_BODY()

	virtual void SpecifyAgentObservation_Implementation(
		FLearningAgentsObservationSchemaElement& OutObservationSchemaElement,
		ULearningAgentsObservationSchema* InObservationSchema) override;

	virtual void GatherAgentObservation_Implementation(
		FLearningAgentsObservationObjectElement& OutObservationObjectElement,
		ULearningAgentsObservationObject* InObservationObject,
		const int32 AgentId) override;
	
	virtual void SpecifyAgentAction_Implementation(
		FLearningAgentsActionSchemaElement& OutActionSchemaElement,
		ULearningAgentsActionSchema* InActionSchema) override;

	virtual void PerformAgentAction_Implementation(
		const ULearningAgentsActionObject* InActionObject,
		const FLearningAgentsActionObjectElement& InActionObjectElement,
		const int32 AgentId) override;

public:
	UAutonomousCarInteractor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
		USplineComponent* TrackSpline;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Observations")
		TArray<float> TrackDistanceSamples;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Observations")
		int OtherAgentCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stick Shift")
		bool bManualTransmission;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stick Shift")
		int MaxTransmissionGear;
};
