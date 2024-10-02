// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ManagerModeEnum.generated.h"

UENUM(BlueprintType)
enum EManagerModeEnum : uint8
{
	ReInitialize UMETA(
		DisplayName = "Re-Initialize Neural Networks",
		Description = "Re-start training and initialize the Neural Network Data (must be done once before continuing or running inference)"),

	ContinueTraining UMETA(
		DisplayName = "Continue Training",
		Description = "Continue training from previously initialized Neural Network Data"),

	InferenceMode UMETA(
		DisplayName = "Run Inference",
		Description = "Agents use the computed weights without training")
};
