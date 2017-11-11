// Fill out your copyright notice in the Description page of Project Settings.

#include "MechPlayerController.h"
#include "MechHUD.h"

void AMechPlayerController::OnKill()
{
	// Kill notify
}

void AMechPlayerController::OnMessage(const FString& Message)
{
	// This is called on the server.
	
	ClientHUDMessage(Message);
}

void AMechPlayerController::ClientHUDMessage_Implementation(const FString& Message)
{
	AHUD* HUD = GetHUD();
	if (HUD)
	{
		AMechHUD* MechHUD = Cast<AMechHUD>(HUD);
		if (MechHUD)
		{
			MechHUD->ShowMessage(Message);
		}
	}
}

void AMechPlayerController::OnKillMessage(class AMechPlayerState* KillerPlayerState, class AMechPlayerState* KilledPlayerState, const UDamageType* KillerDamageType)
{

}