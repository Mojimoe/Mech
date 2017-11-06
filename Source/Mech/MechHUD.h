// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MechHUD.generated.h"

/**
 * 
 */
UCLASS()
class MECH_API AMechHUD : public AHUD
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent)
		void ShowMessage(const FString& Message);
	
	
};
