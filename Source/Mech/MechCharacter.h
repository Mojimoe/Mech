// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Mech.h"
#include "GameFramework/Character.h"
#include "MechCharacter.generated.h"

UCLASS()
class MECH_API AMechCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	/// Sets default values for this character's properties
	AMechCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	/// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	/// Called every frame
	virtual void Tick(float DeltaTime) override;

	/// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// OVERRIDDEN METHODS

	/// Performs the actual jump.
	virtual void Jump() override;

	/// Notifies us when the character jumps, to spawn effects.
	virtual void OnJumped_Implementation() override;

	/// Overriden function allows us to define when jumping is possible.
	bool CanJumpInternal_Implementation() const override;
	
	/// Take damage, handle death
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;


	// INPUT METHODS

	/// Called to enter the sprint mode.
	/// You remain in sprint mode until you stop moving.
	void StartSprinting();

	/// Called to exit the sprint mode.
	/// Not bound to input directly, instead it is called through
	/// a per-tick check to see if we're moving straight.
	void StopSprinting();

	/// called to activate the boost-dodge.
	void Dodge();

	/// Called to perform some debug function
	void DoDebugMethod();

	// TICK METHODS

	/// Every tick, this handles the movement input & translates it into
	/// an input vector for the move component.
	void HandleMovementInput();

	/// Every tick, this handles the camera input and translates it into
	/// camera movement.
	void HandleCameraInput();

	// NETWORK METHODS

	/// Tells the server to multicast out a jump effect.
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerSendPing();

	/// Tells the server to multicast out a jump effect.
	UFUNCTION(Server, Unreliable, WithValidation)
		void ServerSpawnJumpEffects();

	/// Spawns jump particles on all clients.
	UFUNCTION(NetMultiCast, Unreliable)
		void MulticastSpawnJumpEffects();

	/// Tells the server to update visual sprint.
	UFUNCTION(Server, Unreliable, WithValidation)
		void ServerUpdateVisualSprint(bool bIsSprinting);

	// OTHER METHODS

	/// Attempts to kill this pawn.
	bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser);

	/// Notification when killed, both for server and client
	void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

	/// Play hit effects
	void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser);

	void UpdateTeamColors();

	/// sets visual sprint
	void SetVisualSprint(bool bIsSprinting);

	/// set third person
	void SetThirdPersonMode(bool bInThirdPerson);

	/// Build dynamic Materials instances, for effect manipulation. This should run at begin play.
	void BuildDynamicMaterialInstances();

	/// Update per-tick dynamic material values from their associated variables.
	void UpdateDynamicMaterialParameters();

	// ASSET REFERENCE

	/// Pointer to the particle template for jumping.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Mech|Assets|Particles")
		UParticleSystem* AssetJumpParticle;

	/// Pointer to the particle template for jumping.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Mech|Assets|Particles")
		UParticleSystem* AssetDeathParticle;

	// NETWORKED PROPS

	/// Current health of the Pawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Mech|Health")
		float Health;

	// TUNING PROPS

	/// The size of the deadzone. Higher values (closer to 1) increase the
	/// size of the deadzone. 
	/// The deadzone has a soft gradient, not a hard edge, so this value
	/// is a little nebulous.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mech|Camera")
		float Deadzone;

	/// FName of the socket for jump particles
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mech|Sockets")
		FName SocketLeftJumpJet;

	/// FName of the socket for jump particles
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mech|Sockets")
		FName SocketRightJumpJet;

	/// FName of the socket for jump particles
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mech|Sockets")
		FName SocketForwardThruster;

	// PROPS FOR ANIMATION

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Mech|Animation")
		float VisualPitch;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Mech|Animation")
		bool bShouldAnimateSprinting;

	// VARS

	bool bIsDying;

	bool bIsInThirdPerson;

	// COMPONENTS

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mech|Components")
		USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mech|Components")
		UCameraComponent* CameraComponent3P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mech|Components")
		UCameraComponent* CameraComponent1P;

protected:

	// ** CODE ONLY VARS ** //

	TArray<UMaterialInstanceDynamic*> DynamicMaterialInstances;

	/// Variable that drives material properties
	float DynamicAppear = 0.0f;
};
