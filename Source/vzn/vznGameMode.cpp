// Copyright Epic Games, Inc. All Rights Reserved.

#include "vznGameMode.h"
#include "vznCharacter.h"
#include "UObject/ConstructorHelpers.h"

AvznGameMode::AvznGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
