// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DriveAgentsCPPPawn.h"
#include "DriveAgentsCPPSportsCar.generated.h"

/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class DRIVEAGENTSCPP_API ADriveAgentsCPPSportsCar : public ADriveAgentsCPPPawn
{
	GENERATED_BODY()
	
public:

	ADriveAgentsCPPSportsCar();
};
