// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"

DECLARE_DELEGATE(FOnEnterClimbState)
DECLARE_DELEGATE(FOnExitClimbState)

class UAnimMontage;
class UAnimInstance;
class AvznCharacter;

UENUM(BlueprintType)
namespace ECustomMovementMode
{
	enum Type
	{
		MOVE_Climb UMETA(DisplayName = "Climb Mode")
		//MOVE_WallRun UMETA(DisplayName = "Wall Run Mode")
	};
}

/**
 * 
 */
UCLASS()
class VZN_API UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	FOnEnterClimbState OnEnterClimbStateDelegate; // Delegates to be called when the character enters and exits the climbing state
	FOnExitClimbState OnExitClimbStateDelegate; 

protected:

#pragma region OverridenFunctions

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	
	virtual FVector ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const override; // Root motion velocity constraint

#pragma endregion

private:

#pragma region ClimbTraces

	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape = false, bool bDrawPersistantShapes = false);
	FHitResult DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebugShape = false, bool bDrawPersistantShapes = false);

#pragma	endregion

#pragma region ClimbCore

	bool TraceClimbableSurfaces();

	FHitResult TraceFromEyeHeight(float TraceDistance, float TraceStartOffset = 0.f, bool bShowDebugShape = false, bool bDrawPersistantShapes = false);

	bool CanStartClimbing();

	bool CanClimbDownLedge();

	void StartClimbing();

	void StopClimbing();

	void PhysClimb(float deltaTime, int32 Iterations);

	void ProcessClimableSurfaceInfo();

	bool CheckShouldStopClimbing();

	bool CheckHasReachedFloor(); // Check if the character has reached the floor

	FQuat GetClimbRotation(float DeltaTime);

	void SnapMovementToClimableSurfaces(float DeltaTime);

	bool CheckHasReachedLedge();

	void TryStartVaulting(); // Check if the character can vault

	bool CanStartVaulting(FVector& OutVaultStartPosition, FVector& OutVaultEndPosition); // Check if the character can vault

	void PlayClimbMontage(UAnimMontage* MontageToPlay);

	UFUNCTION()
	void OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition); 

	void HandleHopUp(); // Hopping up the wall

	bool CheckCanHopUp(FVector& OutHopUpTargetPosition); // Check if the character can hop up the wall

	void HandleHopDown(); // Hopping down the wall

	bool CheckCanHopDown(FVector& OutHopDownTargetPosition); // Check if the character can hop down the wall

#pragma endregion

	// // Wall Running doesn't work properly, the character doesn't stick to the wall and gravity is messed up - Wall Running is disabled for now
	/*
	bool TraceWallRunningSurfaces();
	bool CanStartWallRunning();

	void StartWallRunning();

	void StopWallRunning();

	void PhysWallRun(float deltaTime, int32 Iterations);

	bool CheckShouldStopWallRunning();

	void HandleJumpInput(float DeltaTime);

	bool CheckForWallTransition();

	FHitResult LastHitResult;

	float WallRunSpeed = 600.0f;
	float WallRunJumpForce = 980.0f;

	bool bIsWallRunning() const; */

#pragma region ClimbCoreVariables

	TArray<FHitResult> ClimbableSurfacesTracedResults;

	FVector CurrentClimbableSurfaceLocation;

	FVector CurrentClimbableSurfaceNormal;

	UPROPERTY()
	UAnimInstance* OwningPlayerAnimInstance;

	UPROPERTY()
	AvznCharacter* OwningPlayerCharacter;

#pragma endregion

#pragma region ClimbBPVariables 

	// Variables and animations for climbing
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	TArray < TEnumAsByte<EObjectTypeQuery> > ClimbableSurfaceTraceTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float CapsuleTraceRadius = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float CapsuleTraceHalfHeight = 72.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxBreakClimbDeceleration = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbSpeed = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float MaxClimbAcceleration = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float ClimbDownWalkableSurfaceTraceOffset = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	float ClimbDownLedgeTraceOffset = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* IdleToClimbMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbToTopMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbDownLedgeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* VaultMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HopUpMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HopDownMontage;

#pragma endregion

public: 

	void ToggleClimbing(bool bEnableClimb); // Toggle climbing
	void RequestHopping(); // Check if the character can hop up or down the wall
	bool IsClimbing() const;

	FORCEINLINE FVector GetClimbableSurfaceNormal() const { return CurrentClimbableSurfaceNormal; } // Get the normal of the climbable surface

	FVector GetUnrotatedClimbVelocity() const;
};
