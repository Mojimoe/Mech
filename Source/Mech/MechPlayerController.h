// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Mech.h"
#include "MechPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "MechPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MECH_API AMechPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	
	void OnKill();

	/// notify local client about deaths
	void OnMessage(const FString& Message);
	
	/// Post a message about a kill.
	void OnKillMessage(class AMechPlayerState* KillerPlayerState, class AMechPlayerState* KilledPlayerState, const UDamageType* KillerDamageType);
};
