// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Mech.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MechCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class MECH_API UMechCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	UMechCharacterMovementComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;

	// Replicated Methods

	/// Sends data to the server updating our current move direction.
	UFUNCTION(Unreliable, Server, WithValidation)
		void ServerSetMoveDirection(const FVector& MoveDir);

	// Overridden Methods
	
	/// Overridden value alows us to situationally alter the max speed.
	/// Sprinting, slowed, etc.
	virtual float GetMaxSpeed() const override;

	/// Overridden value allows us to situationally alter the acceleration.
	/// Sprinting, slowed, etc.
	virtual float GetMaxAcceleration() const override;

	/// Overriden function triggers at the end of a movement update.
	/// Used for dodging and informing the server of our move dir.
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector & OldLocation, const FVector & OldVelocity) override;

	///Override DoJump to trigger the extra jumps.
	virtual bool DoJump(bool bReplayingMoves) override;

	///This is called whenever the character lands on the ground, and will be used to reset the jump counter.
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;

	

	// Methods

	/// Called to activate or deactivate sprint
	void SetSprinting(bool bSprinting);

	/// Called to activate the jet-dodge.
	void DoDodge();

	///Returns whether or not the character is currently moving in a forward direction.
	bool IsMovingForward() const;

	///Whether or not the character can currently jump.
	bool CanJump();

	// Animation Methods

	// Gameplay PROPS

	/// The speed, in units per second, of sprinting.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mech|Movement")
		float SprintSpeed;

	/// The acceleration, in units per second per second, of sprinting.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mech|Movement")
		float SprintAcceleration;

	/// The force imparted by a dodge.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mech|Movement")
		float DodgeStrength;

	UPROPERTY(Category = "Multijump", EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Max Multijump Count"))
		int32 MaxJumpCount;

	UPROPERTY(Category = "Multijump", BlueprintReadWrite, meta = (DisplayName = "Current jump count"))
		int32 JumpCount;

	// Code-Only Variables

	uint8 bWantsToSprint : 1;

	/// Stored Move Direction to update the server with.
	FVector MoveDirection;

	/// Flag for activating dodge here and on the network.
	uint8 bWantsToDodge : 1;

	// Terrifying Replication Voodoo

	friend class FSavedMove_MechMovement;

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
};

class FSavedMove_MechMovement : public FSavedMove_Character
{
public:

	typedef FSavedMove_Character Super;

	///Resets all saved variables.
	virtual void Clear() override;

	///Store input commands in the compressed flags.
	virtual uint8 GetCompressedFlags() const override;

	///This is used to check whether or not two moves can be combined into one.
	///Basically you just check to make sure that the saved variables are the same.
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;

	///Sets up the move before sending it to the server. 
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;
	
	///This is used to copy state from the saved move to the character movement component.
	///This is ONLY used for predictive corrections, the actual data must be sent through RPC.
	virtual void PrepMoveFor(class ACharacter* Character) override;

	/// Saved flag for wanting to sprint.
	uint8 bSavedWantsToSprint : 1;

	/// Stored move direction.
	FVector SavedMoveDirection;

	/// Saved flag for wanting to dodge.
	uint8 bSavedWantsToDodge : 1;

	int32 SavedJumpCount;
};

class FNetworkPredictionData_Client_MechMovement : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_MechMovement(const UCharacterMovementComponent& ClientMovement);

	typedef FNetworkPredictionData_Client_Character Super;

	///@brief Allocates a new copy of our custom saved move
	virtual FSavedMovePtr AllocateNewMove() override;
};
