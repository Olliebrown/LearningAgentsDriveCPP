// Copyright Epic Games, Inc. All Rights Reserved.

#include "DriveAgentsCPPWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UDriveAgentsCPPWheelFront::UDriveAgentsCPPWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}