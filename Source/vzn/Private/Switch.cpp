// Fill out your copyright notice in the Description page of Project Settings.


#include "Switch.h"
#include "Components/BoxComponent.h"
#include "MovingPlatform.h"

// Sets default values
ASwitch::ASwitch()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	RootComponent = BoxCollider;

	SwitchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Switch Mesh"));
	SwitchMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ASwitch::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASwitch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called when the switch is activated
void ASwitch::OnActivate()
{
    ConnectedPlatform->ToggleMovement();
}