// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterAnimInstance.generated.h"

class AvznCharacter;
class UCustomMovementComponent;

/**
 * 
 */
UCLASS()
class VZN_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	// Overriden functions for the anim blueprint
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:

	// References to the character and movement component for accessing movement state
	UPROPERTY()
	AvznCharacter* vznCharacter;
	UPROPERTY()
	UCustomMovementComponent* CustomMovementComponent;

	// Properties for the animation blueprint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	float GroundSpeed;     // Speed on the ground
	void GetGroundSpeed(); // Getter

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	float AirSpeed;
	void GetAirSpeed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	bool bShouldMove;      // Should the character move 
	void GetShouldMove(); 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	bool bIsFalling;      
	void GetIsFalling();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	bool bIsClimbing;
	void GetIsClimbing();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	FVector ClimbVelocity;   // Climb speed
	void GetClimbVelocity(); 
};