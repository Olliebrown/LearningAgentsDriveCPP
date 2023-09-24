// Copyright Epic Games, Inc. All Rights Reserved.

#include "DriveAgentsCPPGameMode.h"
#include "DriveAgentsCPPPlayerController.h"

ADriveAgentsCPPGameMode::ADriveAgentsCPPGameMode()
{
	PlayerControllerClass = ADriveAgentsCPPPlayerController::StaticClass();
}
