// Fill out your copyright notice in the Description page of Project Settings.


#include "MovingPlatform.h"
#include "Components/BoxComponent.h"
#include "Components/InterpToMovementComponent.h"
#include "Switch.h"


// Sets default values
AMovingPlatform::AMovingPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	RootComponent = BoxCollider;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform Mesh"));
	PlatformMesh->SetupAttachment(RootComponent);

	MovementComponent = CreateDefaultSubobject<UInterpToMovementComponent>(TEXT("Movement Component"));

	// Default values, the platform should slowly move from point to point in a ping pong fashion
	MovementComponent->Duration = 5.f;
	MovementComponent->bSweep = true;
	MovementComponent->BehaviourType = EInterpToBehaviourType::PingPong;

}

// Called when the game starts or when spawned
void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	// Checks for the switches connected to the platform
	MovementComponent->ControlPoints.Add(FInterpControlPoint(FVector(0.f, 0.f, 0.f), true));
	for (int i = 0; i < PathPoints.Num(); i++)
	{
		MovementComponent->ControlPoints.Add(FInterpControlPoint(PathPoints[i], true));
	}
	MovementComponent->FinaliseControlPoints();

	if (!bIsMoving)
	{
		MovementComponent->StopMovementImmediately();
	}
	
}

// Called every frame
void AMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Toggle for the switch to call to start or stop the movement of the platform
void AMovingPlatform::ToggleMovement()
{
	if (bIsMoving)
	{
		MovementComponent->StopMovementImmediately();
		bIsMoving = false;
	}
	else if (!bIsMoving)
	{
		MovementComponent->RestartMovement(1.f);
		bIsMoving = true;
	}
}
