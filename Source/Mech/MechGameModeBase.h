// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Mech.h"
#include "GameFramework/GameModeBase.h"
#include "MechGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class MECH_API AMechGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	void NotifyAboutKill(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);
};
