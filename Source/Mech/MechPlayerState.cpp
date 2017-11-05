// Fill out your copyright notice in the Description page of Project Settings.

#include "MechPlayerState.h"
#include "MechCharacter.h"
#include "MechPlayerController.h"

AMechPlayerState::AMechPlayerState()
{
	TeamNumber = 0;
	NumKills = 0;
	NumDeaths = 0;
	bQuitter = false;
}

void AMechPlayerState::Reset()
{
	Super::Reset();

	//PlayerStates persist across seamless travel.  Keep the same teams as previous match.
	//SetTeamNum(0);
	NumKills = 0;
	NumDeaths = 0;
	bQuitter = false;
}

void AMechPlayerState::UnregisterPlayerWithSession()
{
	if (!bFromPreviousLevel)
	{
		Super::UnregisterPlayerWithSession();
	}
}

void AMechPlayerState::ClientInitialize(AController* InController)
{
	Super::ClientInitialize(InController);

	UpdateTeamColors();
}

void AMechPlayerState::SetTeamNum(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;

	UpdateTeamColors();
}

void AMechPlayerState::OnRep_TeamColor()
{
	UpdateTeamColors();
}

void AMechPlayerState::SetQuitter(bool bInQuitter)
{
	bQuitter = bInQuitter;
}

void AMechPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	AMechPlayerState* MechPlayer = Cast<AMechPlayerState>(PlayerState);
	if (MechPlayer)
	{
		MechPlayer->TeamNumber = TeamNumber;
	}
}

void AMechPlayerState::UpdateTeamColors()
{
	AController* OwnerController = Cast<AController>(GetOwner());
	if (OwnerController != NULL)
	{
		AMechCharacter* MechCharacter = Cast<AMechCharacter>(OwnerController->GetCharacter());
		if (MechCharacter != NULL)
		{
			MechCharacter->UpdateTeamColors();
		}
	}
}

int32 AMechPlayerState::GetTeamNum() const
{
	return TeamNumber;
}

int32 AMechPlayerState::GetKills() const
{
	return NumKills;
}

int32 AMechPlayerState::GetDeaths() const
{
	return NumDeaths;
}

float AMechPlayerState::GetScore() const
{
	return Score;
}

bool AMechPlayerState::IsQuitter() const
{
	return bQuitter;
}

void AMechPlayerState::ScoreKill(AMechPlayerState* Victim, int32 Points)
{
	NumKills++;
	ScorePoints(Points);
}

void AMechPlayerState::ScoreDeath(AMechPlayerState* KilledBy, int32 Points)
{
	NumDeaths++;
	ScorePoints(Points);
}

void AMechPlayerState::ScorePoints(int32 Points)
{
	/*
	AShooterGameState* const MyGameState = GetWorld()->GetGameState<AShooterGameState>();
	if (MyGameState && TeamNumber >= 0)
	{
		if (TeamNumber >= MyGameState->TeamScores.Num())
		{
			MyGameState->TeamScores.AddZeroed(TeamNumber - MyGameState->TeamScores.Num() + 1);
		}

		MyGameState->TeamScores[TeamNumber] += Points;
	}
	*/

	Score += Points;
}

void AMechPlayerState::InformAboutKill_Implementation(class AMechPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AMechPlayerState* KilledPlayerState)
{
	//id can be null for bots
	if (KillerPlayerState->UniqueId.IsValid())
	{
		//search for the actual killer before calling OnKill()	
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AMechPlayerController* TestPC = Cast<AMechPlayerController>(*It);
			if (TestPC && TestPC->IsLocalController())
			{
				// a local player might not have an ID if it was created with CreateDebugPlayer.
				ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(TestPC->Player);
				TSharedPtr<const FUniqueNetId> LocalID = LocalPlayer->GetCachedUniqueNetId();
				if (LocalID.IsValid() && *LocalPlayer->GetCachedUniqueNetId() == *KillerPlayerState->UniqueId)
				{
					TestPC->OnKill();
				}
			}
		}
	}
}

void AMechPlayerState::BroadcastDeath_Implementation(class AMechPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AMechPlayerState* KilledPlayerState)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		// all local players get death messages so they can update their huds.
		AMechPlayerController* TestPC = Cast<AMechPlayerController>(*It);
		if (TestPC && TestPC->IsLocalController())
		{
			TestPC->OnDeathMessage(KillerPlayerState, this, KillerDamageType);
		}
	}
}

void AMechPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMechPlayerState, TeamNumber);
	DOREPLIFETIME(AMechPlayerState, NumKills);
	DOREPLIFETIME(AMechPlayerState, NumDeaths);
}

FString AMechPlayerState::GetName() const
{
	return PlayerName;
}
