// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Player_Controller.generated.h"

/**
 * 
 */
UCLASS()
class HIDER_API APlayer_Controller : public APlayerController
{

	int32 health = 100;

	GENERATED_BODY()
	
};
