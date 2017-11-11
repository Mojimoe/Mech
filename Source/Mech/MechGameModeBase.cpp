// Fill out your copyright notice in the Description page of Project Settings.

#include "MechGameModeBase.h"
#include "MechPlayerController.h"

void AMechGameModeBase::NotifyAboutKill(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	/*
	AShooterPlayerState* KillerPlayerState = Killer ? Cast<AShooterPlayerState>(Killer->PlayerState) : NULL;
	AShooterPlayerState* VictimPlayerState = KilledPlayer ? Cast<AShooterPlayerState>(KilledPlayer->PlayerState) : NULL;

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->ScoreKill(VictimPlayerState, KillScore);
		KillerPlayerState->InformAboutKill(KillerPlayerState, DamageType, VictimPlayerState);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->ScoreDeath(KillerPlayerState, DeathScore);
		VictimPlayerState->BroadcastDeath(KillerPlayerState, DamageType, VictimPlayerState);
	}
	*/
}

void AMechGameModeBase::PostChatMessage(const FString& ChatMessage)
{
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AMechPlayerController* PC = Cast<AMechPlayerController>(*It);
		if (PC)
		{
			PC->OnMessage(ChatMessage);
		}
	}
}