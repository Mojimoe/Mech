// Fill out your copyright notice in the Description page of Project Settings.

#include "MechCharacterMovementComponent.h"
#include "MechCharacter.h"

UMechCharacterMovementComponent::UMechCharacterMovementComponent()
{
	bReplicates = true;

	MaxWalkSpeed = 1600.0f;
	MaxAcceleration = 4000.0f;
	GravityScale = 4.2;
	JumpZVelocity = 2200.0f;
	AirControl = 0.35f;

	SprintSpeed = 2400.0f;
	SprintAcceleration = 8000.0f;
	DodgeStrength = 2500.0f;
	MaxJumpCount = 2;
	JumpCount = 0;
}

void UMechCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// == COMMON METHODS ==

void UMechCharacterMovementComponent::SetSprinting(bool bSprinting)
{
	bWantsToSprint = bSprinting;

	if (CharacterOwner)
	{
		AMechCharacter* MechOwner = Cast<AMechCharacter>(CharacterOwner);
		if (MechOwner)
		{
			MechOwner->SetVisualSprint(bSprinting);
		}
	}
}

void UMechCharacterMovementComponent::DoDodge()
{
	bWantsToDodge = true;
}

bool UMechCharacterMovementComponent::IsMovingForward() const
{
	if (!PawnOwner)
	{
		return false;
	}

	FVector Forward = PawnOwner->GetActorForwardVector();
	FVector MoveDirection = Velocity.GetSafeNormal();

	//Ignore vertical movement
	Forward.Z = 0.0f;
	MoveDirection.Z = 0.0f;

	float VelocityDot = FVector::DotProduct(Forward, MoveDirection);
	return VelocityDot > 0.7f;//Check to make sure difference between headings is not too great.
}

bool UMechCharacterMovementComponent::CanJump()
{
	return (IsMovingOnGround() || JumpCount < MaxJumpCount) && CanEverJump();
}


// == ANIMATION METHODS ==

// == OVERRIDDEN METHODS ==

float UMechCharacterMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	if (bWantsToSprint && IsMovingForward())
	{
		MaxSpeed = SprintSpeed;
	}

	return MaxSpeed;
}

float UMechCharacterMovementComponent::GetMaxAcceleration() const
{
	float MaxAccel = Super::GetMaxAcceleration();

	if (bWantsToSprint && IsMovingForward())
	{
		MaxAccel = SprintAcceleration;
	}

	return MaxAccel;
}

void UMechCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (!CharacterOwner)
	{
		return;
	}

	//Store movement vector
	if (CharacterOwner->IsLocallyControlled())
	{
		MoveDirection = PawnOwner->GetLastMovementInputVector();
	}
	//Send movement vector to server
	if (CharacterOwner->Role < ROLE_Authority)
	{
		ServerSetMoveDirection(MoveDirection);
	}

	//Update dodge movement
	if (bWantsToDodge)
	{
		MoveDirection.Normalize();
		FVector DodgeVel = MoveDirection*DodgeStrength;
		DodgeVel.Z = 0.0f;

		Launch(DodgeVel);

		bWantsToDodge = false;
	}

	// == Next, we check to see if we should drop out of sprint.
	float MoveSimilarity = FVector::DotProduct(CharacterOwner->GetActorForwardVector(), MoveDirection);

	if (MoveSimilarity < 0.7 || MoveDirection.Size() < 0.8f)
	{
		if (bWantsToSprint)
		{
			SetSprinting(false);
		}
	}
	
}

bool UMechCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
	if (Super::DoJump(bReplayingMoves))
	{
		JumpCount++;

		//Adjust midair velocity using the input direction
		if (JumpCount > 1)
		{
			FVector NewVelocity = MoveDirection*MaxWalkSpeed;
			NewVelocity.Z = 0.0f;

			Velocity = NewVelocity;
			Velocity.Z = JumpZVelocity;
		}

		return true;
	}

	return false;
}

void UMechCharacterMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	JumpCount = 0;

	Super::ProcessLanded(Hit, remainingTime, Iterations);
}

// == REPLICATED METHODS ==
bool UMechCharacterMovementComponent::ServerSetMoveDirection_Validate(const FVector& MoveDir)
{
	return true;
}

void UMechCharacterMovementComponent::ServerSetMoveDirection_Implementation(const FVector& MoveDir)
{
	MoveDirection = MoveDir;
}


// == NETWORK PREDICTION ==
// Abandon all hope ye who enter here...

//Set input flags on character from saved inputs
void UMechCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)//Client only
{
	Super::UpdateFromCompressedFlags(Flags);

	//The Flags parameter contains the compressed input flags that are stored in the saved move.
	//UpdateFromCompressed flags simply copies the flags from the saved move into the movement component.
	//It basically just resets the movement component to the state when the move was made so it can simulate from there.
	bWantsToSprint = (Flags&FSavedMove_Character::FLAG_Custom_0) != 0;
	bWantsToDodge = (Flags&FSavedMove_Character::FLAG_Custom_1) != 0;
}

class FNetworkPredictionData_Client* UMechCharacterMovementComponent::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		UMechCharacterMovementComponent* MutableThis = const_cast<UMechCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_MechMovement(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void FSavedMove_MechMovement::Clear()
{
	Super::Clear();

	// Clear saved flags/vars back to their default values.
	bSavedWantsToSprint = false;
	bSavedWantsToDodge = false;
	SavedMoveDirection = FVector::ZeroVector;
	SavedJumpCount = 0;
}

uint8 FSavedMove_MechMovement::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedWantsToSprint)
	{
		Result |= FLAG_Custom_0;
	}

	if (bSavedWantsToDodge)
	{
		Result |= FLAG_Custom_1;
	}

	return Result;
}

bool FSavedMove_MechMovement::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	//This pretty much just tells the engine if it can optimize by combining saved moves. There doesn't appear to be
	//any problem with leaving it out, but it seems that it's good practice to implement this anyways.
	if (bSavedWantsToSprint != ((FSavedMove_MechMovement*)&NewMove)->bSavedWantsToSprint)
	{
		return false;
	}

	if (bSavedWantsToDodge != ((FSavedMove_MechMovement*)&NewMove)->bSavedWantsToDodge)
	{
		return false;
	}

	if (SavedMoveDirection != ((FSavedMove_MechMovement*)&NewMove)->SavedMoveDirection)
	{
		return false;
	}

	if (SavedJumpCount != ((FSavedMove_MechMovement*)&NewMove)->SavedJumpCount)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FSavedMove_MechMovement::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UMechCharacterMovementComponent* CharMov = Cast<UMechCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharMov)
	{
		//This is literally just the exact opposite of UpdateFromCompressed flags. We're taking the input
		//from the player and storing it in the saved move.
		bSavedWantsToSprint = CharMov->bWantsToSprint;
		bSavedWantsToDodge = CharMov->bWantsToDodge;

		//Again, just taking the player movement component's state and storing it for later it in the saved move.
		SavedMoveDirection = CharMov->MoveDirection;

		SavedJumpCount = CharMov->JumpCount;
	}
}

void FSavedMove_MechMovement::PrepMoveFor(class ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UMechCharacterMovementComponent* CharMov = Cast<UMechCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharMov)
	{
		// This is just the exact opposite of SetMoveFor.It copies the state from the saved move to the movement
		//component before a correction is made to a client.
		CharMov->MoveDirection = SavedMoveDirection;
		CharMov->JumpCount = SavedJumpCount;
		//Don't update flags here. They're automatically setup before corrections using the compressed flag methods.
	}
}

FNetworkPredictionData_Client_MechMovement::FNetworkPredictionData_Client_MechMovement(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FSavedMovePtr FNetworkPredictionData_Client_MechMovement::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_MechMovement());
}


