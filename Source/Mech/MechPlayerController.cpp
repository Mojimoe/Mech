// Fill out your copyright notice in the Description page of Project Settings.

#include "MechPlayerController.h"

void AMechPlayerController::OnKill()
{
	// Kill notify
}

void AMechPlayerController::OnDeathMessage(class AMechPlayerState* KillerPlayerState, class AMechPlayerState* KilledPlayerState, const UDamageType* KillerDamageType)
{
	/*
	AShooterHUD* ShooterHUD = GetShooterHUD();
	if (ShooterHUD)
	{
		ShooterHUD->ShowDeathMessage(KillerPlayerState, KilledPlayerState, KillerDamageType);
	}

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer && LocalPlayer->GetCachedUniqueNetId().IsValid() && KilledPlayerState->UniqueId.IsValid())
	{
		// if this controller is the player who died, update the hero stat.
		if (*LocalPlayer->GetCachedUniqueNetId() == *KilledPlayerState->UniqueId)
		{
			const auto Events = Online::GetEventsInterface();
			const auto Identity = Online::GetIdentityInterface();

			if (Events.IsValid() && Identity.IsValid())
			{
				const int32 UserIndex = LocalPlayer->GetControllerId();
				TSharedPtr<const FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);
				if (UniqueID.IsValid())
				{
					AShooterCharacter* ShooterChar = Cast<AShooterCharacter>(GetCharacter());
					AShooterWeapon* Weapon = ShooterChar ? ShooterChar->GetWeapon() : NULL;

					FVector Location = ShooterChar ? ShooterChar->GetActorLocation() : FVector::ZeroVector;
					int32 WeaponType = Weapon ? (int32)Weapon->GetAmmoType() : 0;

					FOnlineEventParms Params;
					Params.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
					Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
					Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

					Params.Add(TEXT("PlayerRoleId"), FVariantData((int32)0)); // unused
					Params.Add(TEXT("PlayerWeaponId"), FVariantData((int32)WeaponType));
					Params.Add(TEXT("EnemyRoleId"), FVariantData((int32)0)); // unused
					Params.Add(TEXT("EnemyWeaponId"), FVariantData((int32)0)); // untracked

					Params.Add(TEXT("LocationX"), FVariantData(Location.X));
					Params.Add(TEXT("LocationY"), FVariantData(Location.Y));
					Params.Add(TEXT("LocationZ"), FVariantData(Location.Z));

					Events->TriggerEvent(*UniqueID, TEXT("PlayerDeath"), Params);
				}
			}
		}
	}
	*/
}