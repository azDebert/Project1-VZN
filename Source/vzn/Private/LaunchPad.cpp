// Fill out your copyright notice in the Description page of Project Settings.


#include "LaunchPad.h"
#include "vzn/vznCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ALaunchPad::ALaunchPad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	LaunchPadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Launch Pad Mesh"));

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	BoxCollider->SetupAttachment(LaunchPadMesh);

}

// Called when the game starts or when spawned
void ALaunchPad::BeginPlay()
{
	Super::BeginPlay();
	
	LaunchPadMesh->SetMaterial(0, LaunchPadInactiveMaterial);

	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ALaunchPad::LaunchPadStartTrigger);
	BoxCollider->OnComponentEndOverlap.AddDynamic(this, &ALaunchPad::LaunchPadEndTrigger);

}

// Called every frame
void ALaunchPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALaunchPad::LaunchPadStartTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the actor is the player
	if (AvznCharacter* Player = Cast<AvznCharacter>(OtherActor))
	{
		// Make sure its the capsule component
		if (Cast<UCapsuleComponent>(OtherComp) == Player->GetCapsuleComponent())
		{
			// Change material and prepare for launch
			LaunchPadMesh->SetMaterial(0, LaunchPadActiveMaterial);
			Player->GetCharacterMovement()->JumpZVelocity *= 2.5f;
		}
	}
}

void ALaunchPad::LaunchPadEndTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if the actor is the player
	if (AvznCharacter* Player = Cast<AvznCharacter>(OtherActor))
	{
		// Make sure its the capsule component
		if (Cast<UCapsuleComponent>(OtherComp) == Player->GetCapsuleComponent())
		{
			// Change material back and reset jump velocity
			LaunchPadMesh->SetMaterial(0, LaunchPadInactiveMaterial);
			Player->GetCharacterMovement()->JumpZVelocity /= 2.5f;
		}
	}
}
