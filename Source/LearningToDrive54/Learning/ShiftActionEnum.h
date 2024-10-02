// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "ShiftActionEnum.generated.h"

UENUM(BlueprintType)
enum EShiftActionEnum : uint8 {
	ShiftUp UMETA(DisplayName = "Shift Up"),
	ShiftDown UMETA(DisplayName = "Shift Down"),
	DoNothing UMETA(DisplayName = "Do Nothing")
};
