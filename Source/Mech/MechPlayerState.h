// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MechPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MECH_API AMechPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	AMechPlayerState();

	// Begin APlayerState interface
	/** clear scores */
	virtual void Reset() override;

	/**
	* Set the team
	*
	* @param	InController	The controller to initialize state with
	*/
	virtual void ClientInitialize(class AController* InController) override;

	virtual void UnregisterPlayerWithSession() override;

	virtual void CopyProperties(class APlayerState* PlayerState) override;

	// End APlayerState interface

	/**
	* Set new team and update pawn. Also updates player character team colors.
	*
	* @param	NewTeamNumber	Team we want to be on.
	*/
	void SetTeamNum(int32 NewTeamNumber);

	/** player killed someone */
	void ScoreKill(AMechPlayerState* Victim, int32 Points);

	/** player died */
	void ScoreDeath(AMechPlayerState* KilledBy, int32 Points);

	/** get current team */
	int32 GetTeamNum() const;

	/** get number of kills */
	int32 GetKills() const;

	/** get number of deaths */
	int32 GetDeaths() const;

	/** get number of points */
	float GetScore() const;
	
	FString GetName() const;

	/** Get whether the player is a quitter */
	bool IsQuitter() const;

	/** Set whether the player is a quitter */
	void SetQuitter(bool bInQuitter);

	/** Sends kill (excluding self) to clients */
	UFUNCTION(Reliable, Client)
		void InformAboutKill(class AMechPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AMechPlayerState* KilledPlayerState);

	/** broadcast death to local clients */
	UFUNCTION(Reliable, NetMulticast)
		void BroadcastDeath(class AMechPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AMechPlayerState* KilledPlayerState);

	/** replicate team colors. Updated the players mesh colors appropriately */
	UFUNCTION()
		void OnRep_TeamColor();

protected:

	/** Set the mesh colors based on the current teamnum variable */
	void UpdateTeamColors();

	/** team number */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_TeamColor)
		int32 TeamNumber;

	/** number of kills */
	UPROPERTY(Transient, Replicated)
		int32 NumKills;

	/** number of deaths */
	UPROPERTY(Transient, Replicated)
		int32 NumDeaths;

	/** whether the user quit the match */
	UPROPERTY()
		uint8 bQuitter : 1;

	/** helper for scoring points */
	void ScorePoints(int32 Points);

};
