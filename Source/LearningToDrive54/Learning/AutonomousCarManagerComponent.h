// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsManager.h"
#include "AutonomousCarManagerComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class LEARNINGTODRIVE54_API UAutonomousCarManagerComponent : public ULearningAgentsManager
{
	GENERATED_BODY()

public:
	UAutonomousCarManagerComponent();
	virtual ~UAutonomousCarManagerComponent();

	virtual void PostInitProperties() override;
};
