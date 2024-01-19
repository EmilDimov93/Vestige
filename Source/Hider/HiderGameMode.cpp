// Copyright Epic Games, Inc. All Rights Reserved.

#include "HiderGameMode.h"
#include "HiderCharacter.h"
#include "UObject/ConstructorHelpers.h"

AHiderGameMode::AHiderGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
