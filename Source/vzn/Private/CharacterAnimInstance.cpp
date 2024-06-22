// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "vzn/vznCharacter.h"
#include "Components/CustomMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Cast to the pawn owner and get the custom movement component
	vznCharacter = Cast<AvznCharacter>(TryGetPawnOwner());
	if (vznCharacter)
	{
		CustomMovementComponent = vznCharacter->GetCustomMovementComponent();
	}
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Return early if the character or movement component is not valid
	if (!vznCharacter || !CustomMovementComponent) return;

	// Get the properties for the anim blueprint
	GetGroundSpeed();
	GetAirSpeed();
	GetShouldMove();
	GetIsFalling();
	GetIsClimbing();
	GetClimbVelocity();
}

void UCharacterAnimInstance::GetGroundSpeed() 
{
	GroundSpeed = UKismetMathLibrary::VSizeXY(vznCharacter->GetVelocity()); // Gets the horizontal velocity 
}

void UCharacterAnimInstance::GetAirSpeed()
{
	AirSpeed = vznCharacter->GetVelocity().Z; // Gets the vertical speed
}

void UCharacterAnimInstance::GetShouldMove()
{
	// The character should move based on acceleration and ground speed, and if they're not falling
	bShouldMove =
		CustomMovementComponent->GetCurrentAcceleration().Size() > 0 &&
		GroundSpeed > 5.f &&
		!bIsFalling;
}

void UCharacterAnimInstance::GetIsFalling()
{
	bIsFalling = CustomMovementComponent->IsFalling(); // Check if the character is falling
}

void UCharacterAnimInstance::GetIsClimbing()
{
	bIsClimbing = CustomMovementComponent->IsClimbing(); // Check if the character is climbing
}

void UCharacterAnimInstance::GetClimbVelocity()
{
	ClimbVelocity = CustomMovementComponent->GetUnrotatedClimbVelocity(); // Get the climb velocity
}