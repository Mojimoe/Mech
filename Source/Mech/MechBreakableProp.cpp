// Fill out your copyright notice in the Description page of Project Settings.

#include "MechBreakableProp.h"


// Sets default values
AMechBreakableProp::AMechBreakableProp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;

	StaticMeshComponent->SetCollisionProfileName(FName("MechBreakable"));
	StaticMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AMechBreakableProp::OnStaticMeshOverlapBegin);

	bIsBroken = false;
}

// Called when the game starts or when spawned
void AMechBreakableProp::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMechBreakableProp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMechBreakableProp::OnStaticMeshOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (!bIsBroken)
	{
		DispatchBreak();
	}
}

void AMechBreakableProp::DoBreak()
{
	if (!bIsBroken)
	{
		SetLifeSpan(2.0f);
		if (StaticMeshComponent)
		{
			StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			StaticMeshComponent->SetHiddenInGame(true);
		}
		bIsBroken = true;
	}
}

void AMechBreakableProp::DispatchBreak()
{
	if (Role < ROLE_Authority)
	{
		// Client
		DoBreak();
		ServerBreak();
	}
	else
	{
		// Server
		DoBreak();
		MulticastBreak();
	}
}

bool AMechBreakableProp::ServerBreak_Validate()
{
	return true;
}

void AMechBreakableProp::ServerBreak_Implementation()
{
	// Tell everyone about the break!
	MulticastBreak();
}

void AMechBreakableProp::MulticastBreak_Implementation()
{
	DoBreak();
}
