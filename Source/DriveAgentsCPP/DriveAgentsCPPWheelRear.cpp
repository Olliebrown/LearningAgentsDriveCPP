// Copyright Epic Games, Inc. All Rights Reserved.

#include "DriveAgentsCPPWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UDriveAgentsCPPWheelRear::UDriveAgentsCPPWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}