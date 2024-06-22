// Copyright Epic Games, Inc. All Rights Reserved.

#include "vznCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CustomMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "DebugHelper.h"
#include "MotionWarpingComponent.h"
#include "CableComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Switch.h"
#include "MovingPlatform.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AvznCharacter


AvznCharacter::AvznCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CustomMovementComponent = Cast<UCustomMovementComponent>(GetCharacterMovement());

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 540.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = 678.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a first person camera
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), FName("headSocket"));
	//FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0, 0, 0)); // Position the camera
	FirstPersonCamera->FieldOfView = 103.f;
	FirstPersonCamera->bUsePawnControlRotation = true;

	// Fall damage reduction
	bIsReducingFallDamage = false;

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComp"));

	// Bobbing for first person camera
	bIsBobbing = false;
	if (FirstPersonCamera)
	{
		DefaultZ = FirstPersonCamera->GetRelativeLocation().Z;
	}

	GrappleCable = CreateDefaultSubobject<UCableComponent>(TEXT("Grappling Line"));
	GrappleCable->SetupAttachment(FirstPersonCamera);
	GrappleCable->SetVisibility(false);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AvznCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Initialize camera state
	FirstPersonCamera->SetActive(true); // First person camera is the first one by default
	FollowCamera->SetActive(false); // And the other one is inactive

	AddInputMappingContext(DefaultMappingContext, 0);

	if (CustomMovementComponent)
	{
		CustomMovementComponent->OnEnterClimbStateDelegate.BindUObject(this, &ThisClass::OnPlayerEnterClimbState);
		CustomMovementComponent->OnExitClimbStateDelegate.BindUObject(this, &ThisClass::OnPlayerExitClimbState);
	}

	//Debug::Print(TEXT("Debug working"));
}

void AvznCharacter::AddInputMappingContext(UInputMappingContext* ContextToAdd, int32 InPriority)
{
	if (!ContextToAdd) return;

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ContextToAdd, InPriority);
		}
	}
}

void AvznCharacter::RemoveInputMappingContext(UInputMappingContext* ContextToAdd)
{
	if (!ContextToAdd) return;

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(ContextToAdd);
		}
	}
}

void AvznCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Set the new location of the camera for the bobbing effect
	FVector CurrentLocation = FirstPersonCamera->GetRelativeLocation();

	// No bobbing effect when standing still
	float TargetZ = DefaultZ;

	// When the player is moving, turn the bobbing effect on and calculate the new Z location
	if (GetVelocity().SizeSquared() > 0 && GetCharacterMovement()->IsMovingOnGround())
	{
		bIsBobbing = true;
		float Time = GetWorld()->GetTimeSeconds();
		// Calculate target Z with bobbing using a sine wave
		TargetZ += FMath::Sin(Time * BobbingSpeed) * BobbingAmount;
	}
	else
	{
		bIsBobbing = false;
	}

	// Camera smoothly transitions from standing still to movement using lerp
	float NewZ = FMath::Lerp(CurrentLocation.Z, TargetZ, DeltaTime * BobbingSpeed);
	FirstPersonCamera->SetRelativeLocation(FVector(CurrentLocation.X, CurrentLocation.Y, NewZ));

	if (bIsGrappling)
	{
		GrappleCable->EndLocation = GetActorTransform().InverseTransformPosition(GrapplingPoint);

		GetCharacterMovement()->AddForce((GrapplingPoint - GetActorLocation()).GetSafeNormal() * 200000);
	}

	// Slide
	if (bIsSliding)
	{
		FVector CurrentVelocity = GetVelocity();
		float Speed = CurrentVelocity.Size();
		float SpeedReduction = SlideDeceleration * DeltaTime;
		Speed = FMath::Max(0.f, Speed - SpeedReduction);

		if (Speed > MinSlideSpeed)
		{
			CurrentVelocity = CurrentVelocity.GetSafeNormal() * Speed;
			GetCharacterMovement()->Velocity = CurrentVelocity;
		}
		else
		{
			OnCrouchEnded(FInputActionValue());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AvznCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		//EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AvznCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AvznCharacter::HandleGroundMovementInput);
		EnhancedInputComponent->BindAction(ClimbMoveAction, ETriggerEvent::Triggered, this, &AvznCharacter::HandleClimbMovementInput);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AvznCharacter::Look);

		// Climb
		EnhancedInputComponent->BindAction(ClimbAction, ETriggerEvent::Started, this, &AvznCharacter::OnClimbActionStarted); //

		EnhancedInputComponent->BindAction(ClimbHopAction, ETriggerEvent::Started, this, &AvznCharacter::OnClimbHopActionStarted);

		// Switch Camera
		EnhancedInputComponent->BindAction(SwitchCameraAction, ETriggerEvent::Started, this, &AvznCharacter::SwitchCamera); //

		// Fall Damage Reduction - Roll
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AvznCharacter::OnReduceFallDamageStarted); // Added for reduced damage
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AvznCharacter::OnReduceFallDamageEnded); //

		// Walk
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &AvznCharacter::OnWalkStarted);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Completed, this, &AvznCharacter::OnWalkEnded);

		// Crouch / Slide
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AvznCharacter::OnCrouchStarted); // Crouch/Slide as well as reduced damage
        EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AvznCharacter::OnCrouchEnded);

		// Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AvznCharacter::Interact);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AvznCharacter::StopInteract);

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AvznCharacter::HandleGroundMovementInput(const FInputActionValue& Value)
{
	// Ground movement
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Get rotation to find out forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// Get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// Get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// Add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AvznCharacter::HandleClimbMovementInput(const FInputActionValue& Value)
{
	// Climbing movement
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FVector ForwardDirection = FVector::CrossProduct(
		-CustomMovementComponent->GetClimbableSurfaceNormal(),
		GetActorRightVector()
	);

	const FVector RightDirection = FVector::CrossProduct(
		-CustomMovementComponent->GetClimbableSurfaceNormal(),
		-GetActorUpVector()
	);

	// Add movement 
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AvznCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AvznCharacter::OnClimbActionStarted(const FInputActionValue& Value)
{
	if (!CustomMovementComponent) return;

	// Toggle climbing
	if (!CustomMovementComponent->IsClimbing())
	{
		CustomMovementComponent->ToggleClimbing(true);
	}
	else
	{
		CustomMovementComponent->ToggleClimbing(false);
	}
}

// Player enters and exits the climb state, adding and removing the climb context
void AvznCharacter::OnPlayerEnterClimbState()
{
	AddInputMappingContext(ClimbMappingContext, 1);
}

void AvznCharacter::OnPlayerExitClimbState()
{
	RemoveInputMappingContext(ClimbMappingContext);
}

// Hopping action when climbing
void AvznCharacter::OnClimbHopActionStarted(const FInputActionValue& Value)
{
	if (CustomMovementComponent)
	{
		CustomMovementComponent->RequestHopping();
	}
}

// Switch between first person and third person cameras
void AvznCharacter::SwitchCamera(const FInputActionValue& Value)
{
	if (FirstPersonCamera && FollowCamera) // Make sure both cameras exist
	{
		//Debug::Print(TEXT("Camera switch"));
	    bool bIsFirstPerson = FirstPersonCamera->IsActive();
		FirstPersonCamera->SetActive(!bIsFirstPerson); // Switch the active camera
		FollowCamera->SetActive(bIsFirstPerson);
	}
}

// Called when the character lands after a fall, calculates fall damage
void AvznCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Calculate the fall damage based on velocity
	float FallSpeed = GetCharacterMovement()->Velocity.Z;

	if (bIsReducingFallDamage)
	{
		// Reduced fall damage, doesn't slow player down
	}
	else if (FallSpeed < -1000.0f)
	{
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &AvznCharacter::EnableMovement, 2.0f, false);
		GetCharacterMovement()->DisableMovement();
	}
}

// Enable Movement after falling
void AvznCharacter::EnableMovement()
{
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AvznCharacter::OnReduceFallDamageStarted(const FInputActionValue& Value) 
{
	bIsReducingFallDamage = true;
}

void AvznCharacter::OnReduceFallDamageEnded(const FInputActionValue& Value)
{
	bIsReducingFallDamage = false;
}

void AvznCharacter::OnWalkStarted(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = 250.f;
}

void AvznCharacter::OnWalkEnded(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = 678.f;
}

// Crouch / Slide, when the player is moving at a certain speed they will slide and keep some of their speed, if they slow down they stop sliding
void AvznCharacter::OnCrouchStarted(const FInputActionValue& Value)
{
	if (!bIsCrouching && GetCharacterMovement()->IsMovingOnGround() && GetVelocity().Size() >= GetCharacterMovement()->MaxWalkSpeed)
	{
		bIsCrouching = true;
		bIsSliding = true;
		GetCapsuleComponent()->SetCapsuleHalfHeight(48.f, true);
		GetCharacterMovement()->GroundFriction = 0; // No friction when sliding to keep speed
		FVector SlideDirection = GetVelocity().GetSafeNormal();
		GetCharacterMovement()->Velocity = SlideDirection * SlideInitialSpeed;

		GetMesh()->SetMaterial(0, CrouchingMaterial); // Change material when sliding, lack of animation
	}
	else if (bIsCrouching)
	{
		OnCrouchEnded(Value);
	}
}

void AvznCharacter::OnCrouchEnded(const FInputActionValue& Value)
{
	if (bIsCrouching)
	{
		bIsCrouching = false;
		bIsSliding = false;
		GetCapsuleComponent()->SetCapsuleHalfHeight(96.f, true);
		GetCharacterMovement()->GroundFriction = 8.0;
		GetCharacterMovement()->BrakingDecelerationWalking = 2048.f;

	    GetMesh()->SetMaterial(0, DefaultMaterial);
		
	}
}

// Interact / Grapple, able to use switches to move platforms and grapple to move around the level
void AvznCharacter::Interact()
{
	FVector Start = GetCapsuleComponent()->GetComponentLocation();
	FVector End = Start + (MaxLineDistance * UKismetMathLibrary::GetForwardVector 
	   (FirstPersonCamera->GetComponentRotation()));
	DrawDebugLine(GetWorld(), Start, End, FColor::Cyan);

	FHitResult HitResult;
	bool hasHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel1,
		FCollisionShape::MakeSphere(100.f)); // ECC_GameTraceChannel1 is the custom channel for Interactables

	// Firstly check for the switch object and activate it also grapple
	if (hasHit)
	{
		ASwitch* HitActor = Cast<ASwitch>(HitResult.GetActor());
		if (HitActor != nullptr)
		{
			HitActor->OnActivate();
		}
		else
		{
			bIsGrappling = true;
		}
		bIsGrappling = true;
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		GrappleCable->SetVisibility(true);
		GrapplingPoint = HitResult.ImpactPoint;
	}
}

// Stop grappling, player falls
void AvznCharacter::StopInteract()
{
	bIsGrappling = false;
	if (!GetCharacterMovement()->IsFalling())
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	}
	GrappleCable->SetVisibility(false);
}