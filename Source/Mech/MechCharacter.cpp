// Fill out your copyright notice in the Description page of Project Settings.

#include "MechCharacter.h"
#include "MechCharacterMovementComponent.h"
#include "MechGameModeBase.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
AMechCharacter::AMechCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UMechCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Build New Components

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(FName("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 1000.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent3P = CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent3P"));
	CameraComponent3P->SetupAttachment(SpringArmComponent);
	CameraComponent3P->bAutoActivate = false;

	CameraComponent1P = CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent1P"));
	CameraComponent1P->SetupAttachment(RootComponent);
	CameraComponent1P->bUsePawnControlRotation = true;
	CameraComponent1P->bAutoActivate = true;

	// Initialize Inherited Components
	USkeletalMeshComponent* SkelMesh = GetMesh();
	if (SkelMesh)
	{
		SkelMesh->bOwnerNoSee = true;
	}

	UMovementComponent* MoveComp = GetMovementComponent();
	if (MoveComp)
	{
		MoveComp->SetIsReplicated(true);
		MoveComp->SetNetAddressable();
	}

	// Initialize Components

	//FirstPersonMesh->bOnlyOwnerSee = true;
	//FirstPersonMesh->CastShadow = false;

	// Initialize UPROP Network variables
	Health = 100.0f;

	// Initialize UPROP Tuning Variables
	Deadzone = 0.25f;

	// Initialize UPROPS for animation
	VisualPitch = 0.0f;
	bShouldAnimateSprinting = false;

	// Init Vars
	bIsDying = false;
	bIsInThirdPerson = false;
}

// Called when the game starts or when spawned
void AMechCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMechCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleMovementInput();

	HandleCameraInput();

	if (Role == ROLE_Authority)
	{
		FRotator ControlRot = GetControlRotation();
		ControlRot.Normalize();
		VisualPitch = ControlRot.Pitch;
	}
}

void AMechCharacter::Jump()
{
	Super::Jump();

	if (CanJump())
	{
		if (Role < ROLE_Authority)
		{
			ServerSpawnJumpEffects();
		}
		else
		{
			MulticastSpawnJumpEffects();
		}
	}
}

void AMechCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	//MulticastSpawnJumpEffects();
}

bool AMechCharacter::CanJumpInternal_Implementation() const
{
	bool bCanJump = Super::CanJumpInternal_Implementation();

	UMechCharacterMovementComponent* MyMovementComp = Cast<UMechCharacterMovementComponent>(GetCharacterMovement());
	if (!bCanJump && MyMovementComp)
	{
		bCanJump = MyMovementComp->CanJump();
	}

	return bCanJump;
}

float AMechCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	if (Health <= 0.f)
	{
		return 0.f;
	}

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		Health -= ActualDamage;
		if (Health <= 0)
		{
			Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
		}
		else
		{
			PlayHit(ActualDamage, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
		}
	}

	return ActualDamage;
}

// Called to bind functionality to input
void AMechCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(FName("MoveForwardKeyboard"));
	PlayerInputComponent->BindAxis(FName("MoveRightKeyboard"));
	PlayerInputComponent->BindAxis(FName("MoveForwardGamepad"));
	PlayerInputComponent->BindAxis(FName("MoveRightGamepad"));
	PlayerInputComponent->BindAxis(FName("LookPitchMouse"));
	PlayerInputComponent->BindAxis(FName("LookYawMouse"));
	PlayerInputComponent->BindAxis(FName("LookPitchGamepad"));
	PlayerInputComponent->BindAxis(FName("LookYawGamepad"));

	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(FName("Sprint"), IE_Pressed, this, &AMechCharacter::StartSprinting);
	PlayerInputComponent->BindAction(FName("Dodge"), IE_Pressed, this, &AMechCharacter::Dodge);
	PlayerInputComponent->BindAction(FName("Debug"), IE_Pressed, this, &AMechCharacter::DoDebugMethod);
}

void AMechCharacter::Dodge()
{
	UMechCharacterMovementComponent* MoveComp = Cast<UMechCharacterMovementComponent>(GetCharacterMovement());
	if (MoveComp)
	{
		MoveComp->DoDodge();
	}
}

void AMechCharacter::StartSprinting()
{
	UMechCharacterMovementComponent* MoveComp = Cast<UMechCharacterMovementComponent>(GetCharacterMovement());
	if (MoveComp)
	{
		MoveComp->SetSprinting(true);
	}
}

void AMechCharacter::StopSprinting()
{
	UMechCharacterMovementComponent* MoveComp = Cast<UMechCharacterMovementComponent>(GetCharacterMovement());
	if (MoveComp)
	{
		MoveComp->SetSprinting(false);
	}
}

void AMechCharacter::DoDebugMethod()
{
	SetThirdPersonMode(!bIsInThirdPerson); // Toggle third person
	//ServerSendPing();
}

void AMechCharacter::HandleMovementInput()
{
	FRotator CharacterRotation = GetActorRotation();

	float MoveForwardGamepad = GetInputAxisValue(FName("MoveForwardGamepad"));
	float MoveRightGamepad = GetInputAxisValue(FName("MoveRightGamepad"));

	FVector StickInput = FVector(MoveForwardGamepad, MoveRightGamepad, 0.0f);

	if (StickInput.Size() < Deadzone)
	{
		// Do nothing --- Insufficient stick input!
	}	
	else
	{
		// Scale the stick input such that there's a smooth curve out of the deadzone.
		// This feels buttery smooth~
		FVector StickInputModified = StickInput.GetSafeNormal() * ((StickInput.Size() - Deadzone) / (1 - Deadzone));

		// Now we tell the CharacterMovementComponent about our final input!
		AddMovementInput(CharacterRotation.RotateVector(StickInputModified));
	}

	float MoveForwardKeyboard = GetInputAxisValue(FName("MoveForwardKeyboard"));
	float MoveRightKeyboard = GetInputAxisValue(FName("MoveRightKeyboard"));

	FVector KeyboardInput = FVector(MoveForwardKeyboard, MoveRightKeyboard, 0.0f);

	AddMovementInput(CharacterRotation.RotateVector(KeyboardInput));
}

void AMechCharacter::HandleCameraInput()
{
	// First off, let's apply the gamepad camera input.
	// This is a little complex because we gotta apply some deadzones.

	float LookPitchGamepad = GetInputAxisValue(FName("LookPitchGamepad"));
	float LookYawGamepad = GetInputAxisValue(FName("LookYawGamepad"));

	// Calculate deadzones for gamepad only, so that loose sticks don't bother your look direction.
	FVector LookInputGamepad = FVector(LookPitchGamepad, LookYawGamepad, 0.0f);

	if (LookInputGamepad.Size() < Deadzone)
	{
		// Do nothing --- Insufficient stick input!
	}
	else
	{
		// Scale the stick input such that there's a smooth curve out of the deadzone.
		// This feels buttery smooth~
		FVector LookInputModified = LookInputGamepad.GetSafeNormal() * ((LookInputGamepad.Size() - Deadzone) / (1 - Deadzone));

		// Now we tell the CharacterMovementComponent about our final input!
		AddControllerPitchInput(LookInputModified.X);
		AddControllerYawInput(LookInputModified.Y);
	}

	// Now we just roll right into also adding mouse input.
	// This is way simpler because a mouse doesn't need a deadzone.

	float LookPitchMouse = GetInputAxisValue(FName("LookPitchMouse"));
	float LookYawMouse = GetInputAxisValue(FName("LookYawMouse"));

	AddControllerPitchInput(LookPitchMouse);
	AddControllerYawInput(LookYawMouse);
}

void AMechCharacter::ServerSendPing_Implementation()
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	AMechGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AMechGameModeBase>();
	if (GameMode)
	{
		GameMode->PostChatMessage("Player Pinged.");
	}
}

bool AMechCharacter::ServerSendPing_Validate()
{
	return true;
}

void AMechCharacter::ServerSpawnJumpEffects_Implementation()
{
	MulticastSpawnJumpEffects();
}

bool AMechCharacter::ServerSpawnJumpEffects_Validate()
{
	return true;
}

void AMechCharacter::MulticastSpawnJumpEffects_Implementation()
{
	USkeletalMeshComponent* Mesh = GetMesh();
	if (AssetJumpParticle && Mesh)
	{
		UGameplayStatics::SpawnEmitterAttached(AssetJumpParticle, Mesh, SocketLeftJumpJet);
		UGameplayStatics::SpawnEmitterAttached(AssetJumpParticle, Mesh, SocketRightJumpJet);
	}
	
}

void AMechCharacter::ServerUpdateVisualSprint_Implementation(bool bIsSprinting)
{
	bShouldAnimateSprinting = bIsSprinting;
}

bool AMechCharacter::ServerUpdateVisualSprint_Validate(bool bIsSprinting)
{
	return true; // always trust the client
}

void AMechCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// only to local owner: weapon change requests are locally instigated, other clients don't need it
	//DOREPLIFETIME_CONDITION(AShooterCharacter, Inventory, COND_OwnerOnly);

	//DOREPLIFETIME_CONDITION(AShooterCharacter, LastTakeHitInfo, COND_Custom);

	// everyone
	//DOREPLIFETIME(AShooterCharacter, CurrentWeapon);
	DOREPLIFETIME(AMechCharacter, Health);
	DOREPLIFETIME(AMechCharacter, VisualPitch);
	DOREPLIFETIME(AMechCharacter, bShouldAnimateSprinting);
}

bool AMechCharacter::Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser)
{
	if (Role < ROLE_Authority)
	{
		return false;
	}

	Health = FMath::Min(0.0f, Health);

	// if this is an environmental death then refer to the previous killer so that they receive credit (knocked into lava pits, etc)
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	AController* const KilledPlayer = (Controller != NULL) ? Controller : Cast<AController>(GetOwner());
	GetWorld()->GetAuthGameMode<AMechGameModeBase>()->NotifyAboutKill(Killer, KilledPlayer, this, DamageType);

	UCharacterMovementComponent* CharMove = GetCharacterMovement();
	if (CharMove)
	{
		GetCharacterMovement()->ForceReplicationUpdate();
	}
	
	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);

	return true;
}

void AMechCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser)
{
	if (bIsDying)
	{
		return;
	}

	bReplicateMovement = false;
	bTearOff = true; // Stops replicating.
	bIsDying = true;

	if (Role == ROLE_Authority)
	{
		//ReplicateHit(KillingDamage, DamageEvent, PawnInstigator, DamageCauser, true);

		// force feedback
	}

	// Play sound + death effects

	// remove all weapons
	//DestroyInventory();

	if (GetMesh())
	{
		static FName CollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetCollisionProfileName(CollisionProfileName);
	}
	SetActorEnableCollision(true);

	// disable collisions on capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void AMechCharacter::PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	//notify of enemy hits and replicate. see shootergame.
}

void AMechCharacter::UpdateTeamColors()
{
	// change material settings
}

void AMechCharacter::SetVisualSprint(bool bIsSprinting)
{
	if (Role < ROLE_Authority)
	{
		ServerUpdateVisualSprint(bIsSprinting);
	}
	else
	{
		bShouldAnimateSprinting = bIsSprinting;
	}
}

void AMechCharacter::SetThirdPersonMode(bool bNewThirdPerson)
{
	bIsInThirdPerson = bNewThirdPerson;

	if (bNewThirdPerson)
	{
		if (CameraComponent1P) CameraComponent1P->Deactivate();
		if (CameraComponent1P) CameraComponent3P->Activate();
		if (GetMesh()) GetMesh()->SetOwnerNoSee(false);
	}
	else 
	{
		if (CameraComponent3P) CameraComponent3P->Deactivate();
		if (CameraComponent1P) CameraComponent1P->Activate();
		if (GetMesh()) GetMesh()->SetOwnerNoSee(true);
	}
}