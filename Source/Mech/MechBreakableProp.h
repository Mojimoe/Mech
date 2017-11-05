// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Mech.h"
#include "GameFramework/Actor.h"
#include "MechBreakableProp.generated.h"

UCLASS()
class MECH_API AMechBreakableProp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMechBreakableProp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// METHODS

	/// On Overlap
	UFUNCTION()
		void OnStaticMeshOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	/// Called to actually break the prop, spawn effects, all that jazz.
	void DoBreak();

	/// Called to dispatch the break trigger across the network.
	void DispatchBreak();
	
	// NETWORKED METHODS

	/// Tells the server to break the prop.
	UFUNCTION(Server, Unreliable, WithValidation)
		void ServerBreak();

	/// Tells the clients to break the prop.
	UFUNCTION(NetMultiCast, Unreliable)
		void MulticastBreak();

	// COMPONENTS

	///  The static mesh of the prop
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mech")
		UStaticMeshComponent* StaticMeshComponent;


	// LOCAL VARS
	bool bIsBroken = false;
};
