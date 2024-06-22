// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "vznCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

class UCustomMovementComponent;
class UMotionWarpingComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class AvznCharacter : public ACharacter
{

	GENERATED_BODY()

public:
	AvznCharacter(const FObjectInitializer& ObjectInitializer);

private:

#pragma region Components 
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** First Person Camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera; //

	/** Custom Movement Component for Extra Movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UCustomMovementComponent* CustomMovementComponent;

	/** Motion Warping for movement based on root motion aligning to targets */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UMotionWarpingComponent* MotionWarpingComponent;

#pragma endregion

#pragma region Input

	void OnPlayerEnterClimbState();
	void OnPlayerExitClimbState();

	void AddInputMappingContext(UInputMappingContext* ContextToAdd, int32 InPriority);
	void RemoveInputMappingContext(UInputMappingContext* ContextToAdd);

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* ClimbMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Climb Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ClimbMoveAction;

	/** Climb Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ClimbAction;

	/** Climb Hop Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ClimbHopAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Switch Camera Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchCameraAction;

	/** Walk Input Action*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WalkAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	/** Interact Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

#pragma endregion

#pragma region InputCallbackFunctions

	void HandleGroundMovementInput(const FInputActionValue& Value);
	void HandleClimbMovementInput(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void OnClimbActionStarted(const FInputActionValue& Value);

	void OnClimbHopActionStarted(const FInputActionValue& Value);

	void SwitchCamera(const FInputActionValue& Value);

	/** Implementing fall damage / timeout */
	void EnableMovement(); // Enable Movement after falling
	FTimerHandle UnusedHandle;

	void OnReduceFallDamageStarted(const FInputActionValue& Value);
	void OnReduceFallDamageEnded(const FInputActionValue& Value);

	// To check if the player can reduce fall damage
	bool bIsReducingFallDamage;

	void OnWalkStarted(const FInputActionValue& Value);
	void OnWalkEnded(const FInputActionValue& Value);

	// Crouching/Sliding variables and functions 
	void OnCrouchStarted(const FInputActionValue& Value);
    void OnCrouchEnded(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crouch, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crouch, meta = (AllowPrivateAccess = "true"))
	bool bIsSliding = false;  // Whether the character is currently sliding

	UPROPERTY(EditAnywhere, Category = "Movement")
	float SlideInitialSpeed = 1500.f;  // Initial speed when starting to slide

	UPROPERTY(EditAnywhere, Category = "Movement")
	float SlideDeceleration = 500.f;  // How quickly the slide slows down

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MinSlideSpeed = 500.f;  // Minimum speed before the slide stops

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* DefaultMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* CrouchingMaterial;

	// Interact with objects
	void Interact();
	void StopInteract();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Grappling, meta = (AllowPrivateAccess = "true"))
	class UCableComponent* GrappleCable;

	float MaxLineDistance = 1000.f;
	bool bIsGrappling = false;
	FVector GrapplingPoint;

	// Bobbing effect when moving
	float BobbingSpeed = .5f; // How fast the camera bobs
	float BobbingAmount = 10.f; // How much the camera bobs
	float DefaultZ;
	bool bIsBobbing;
	
#pragma endregion

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay();

	virtual void Tick(float DeltaTime) override;

	// Implementing fall damage/timeout
	virtual void Landed(const FHitResult& Hit) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Custom Movement Component
	FORCEINLINE UCustomMovementComponent* GetCustomMovementComponent() const { return CustomMovementComponent; }

	FORCEINLINE UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }

	// First Person Camera
	FORCEINLINE UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

};