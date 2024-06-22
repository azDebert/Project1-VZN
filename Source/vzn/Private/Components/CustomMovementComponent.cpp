// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/CustomMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "vzn/vznCharacter.h"
#include "vzn/DebugHelper.h"
#include "MotionWarpingComponent.h"

void UCustomMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();

	if (OwningPlayerAnimInstance)
	{
		OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &UCustomMovementComponent::OnClimbMontageEnded);
		OwningPlayerAnimInstance->OnMontageBlendingOut.AddDynamic(this, &UCustomMovementComponent::OnClimbMontageEnded);
	}

	OwningPlayerCharacter = Cast<AvznCharacter>(CharacterOwner);
}

void UCustomMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/*TraceClimbableSurfaces();
	TraceFromEyeHeight(80.f);*/
	//CanClimbDownLedge();

	// Wall Running doesn't work properly, the character doesn't stick to the wall and gravity is messed up - Wall Running is disabled for now
	/*if (!bIsWallRunning() && CanStartWallRunning()) {
		StartWallRunning();
	}
	else if (bIsWallRunning()) {
		if (CheckShouldStopWallRunning()) {
			StopWallRunning();
		}
		else {
			PhysWallRun(DeltaTime, 0);  // Apply wall running physics
		}
	}*/
}

void UCustomMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) 
{
	if (IsClimbing())
	{
		bOrientRotationToMovement = false;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(48.f);

		OnEnterClimbStateDelegate.ExecuteIfBound();
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == ECustomMovementMode::MOVE_Climb)
	{
		bOrientRotationToMovement = true;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.f);

		const FRotator DirtyRotation = UpdatedComponent->GetComponentRotation();
		const FRotator CleanStandRotation = FRotator(0.f, DirtyRotation.Yaw, 0.f);
		UpdatedComponent->SetRelativeRotation(CleanStandRotation);

		StopMovementImmediately();

		OnExitClimbStateDelegate.ExecuteIfBound();
	}

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void UCustomMovementComponent::PhysCustom(float deltaTime, int32 Iterations) 
{
	if (IsClimbing())
	{
		PhysClimb(deltaTime, Iterations);
	}

	Super::PhysCustom(deltaTime, Iterations);
}

float UCustomMovementComponent::GetMaxSpeed() const
{
	if (IsClimbing())
	{
		return MaxClimbSpeed;
	}
	else
	{
		return Super::GetMaxSpeed();
	}
}

float UCustomMovementComponent::GetMaxAcceleration() const
{
	if (IsClimbing())
	{
		return MaxClimbAcceleration;
	}
	else
	{
		return Super::GetMaxAcceleration();
	}
}

FVector UCustomMovementComponent::ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const
{
	const bool bIsPlayingRMMontage =
		IsFalling() && OwningPlayerAnimInstance && OwningPlayerAnimInstance->IsAnyMontagePlaying(); // Check if the player is playing a root motion montage

	if (bIsPlayingRMMontage)
	{
		return RootMotionVelocity;
	}
	else
	{
		return Super::ConstrainAnimRootMotionVelocity(RootMotionVelocity, CurrentVelocity);
	}
}

#pragma region ClimbTraces

// Climb traces to handle raycasts and capsule trace for climbing 
TArray<FHitResult> UCustomMovementComponent::DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShowDebugShape, bool bDrawPersistantShapes)
{
	TArray<FHitResult> OutCapsuleTraceHitResults;

	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;

	if (bShowDebugShape)
	{
		DebugTraceType = EDrawDebugTrace::ForOneFrame;

		if (bDrawPersistantShapes)
		{
			DebugTraceType = EDrawDebugTrace::Persistent;
		}
	}

	UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		this,
		Start,
		End,
		CapsuleTraceRadius,
		CapsuleTraceHalfHeight,
		ClimbableSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		DebugTraceType,
		OutCapsuleTraceHitResults,
		false
	);

	return OutCapsuleTraceHitResults;
}

FHitResult UCustomMovementComponent::DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShowDebugShape, bool bDrawPersistantShapes)
{
	FHitResult OutHit;

	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;

	if (bShowDebugShape)
	{
		DebugTraceType = EDrawDebugTrace::ForOneFrame;

		if (bDrawPersistantShapes)
		{
			DebugTraceType = EDrawDebugTrace::Persistent;
		}
	}

	UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		Start,
		End,
		ClimbableSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		DebugTraceType,
		OutHit,
		false
	);

	return OutHit;
}

#pragma	endregion

// Wall Running doesn't work properly, the character doesn't stick to the wall and gravity is messed up - Wall Running is disabled for now
/*
#pragma region Wall Run Traces 
bool UCustomMovementComponent::TraceWallRunningSurfaces()
{
	FVector Start = CharacterOwner->GetActorLocation() + FVector(0, 0, 50);  // Offset start to approximate chest height
	FVector Direction = CharacterOwner->GetActorForwardVector();
	FVector End = Start + Direction * 1000;  // Extend the trace 1000 units in front

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(CharacterOwner);
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);

	return HitResult.bBlockingHit && HitResult.GetActor() != nullptr; // Ensure the hit is valid and is not the player
}

#pragma endregion
*/

#pragma region ClimbCore

void UCustomMovementComponent::ToggleClimbing(bool bEnableClimb)
{
	if (bEnableClimb)
	{
		if (CanStartClimbing())
		{
			// Enter climb state
			PlayClimbMontage(IdleToClimbMontage);
		}
		else if (CanClimbDownLedge())
		{
			PlayClimbMontage(ClimbDownLedgeMontage);
		}
		else
		{
			TryStartVaulting(); // Check if the character can vault instead of climbing
		}
	}
	
	if(!bEnableClimb) // Toggle to stop climbing
	{
		StopClimbing();
	}
}

bool UCustomMovementComponent::CanStartClimbing()
{
	// Checks for whether the character is falling, if the trace hits a climbable wall, and if the character is close enough to the wall
	if (IsFalling()) return false;
	if (!TraceClimbableSurfaces()) return false;
	if (!TraceFromEyeHeight(100.f).bBlockingHit) return false; 

	return true;
}

bool UCustomMovementComponent::CanClimbDownLedge()
{
	if (IsFalling()) return false;

	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector DownVector = -UpdatedComponent->GetUpVector();

	const FVector WalkableSurfaceTraceStart = ComponentLocation + ComponentForward * ClimbDownWalkableSurfaceTraceOffset;
	const FVector WalkableSurfaceTraceEnd = WalkableSurfaceTraceStart + DownVector * 100.f;

	FHitResult WalkableSurfaceHit = DoLineTraceSingleByObject(WalkableSurfaceTraceStart, WalkableSurfaceTraceEnd); //ds

	const FVector LedgeTraceStart = WalkableSurfaceHit.TraceStart + ComponentForward * ClimbDownLedgeTraceOffset;
	const FVector LedgeTraceEnd = LedgeTraceStart + DownVector * 200.f;

	FHitResult LedgeTraceHit = DoLineTraceSingleByObject(LedgeTraceStart, LedgeTraceEnd); //ds

	if (WalkableSurfaceHit.bBlockingHit && !LedgeTraceHit.bBlockingHit)
	{
		return true;
	}

	return false;
}

void UCustomMovementComponent::StartClimbing()
{
	SetMovementMode(MOVE_Custom, ECustomMovementMode::MOVE_Climb);
}

void UCustomMovementComponent::StopClimbing()
{
	SetMovementMode(MOVE_Falling);
}

void UCustomMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	// Process all the climbable surfaces info
	TraceClimbableSurfaces();
	ProcessClimableSurfaceInfo();

	// Check if the character needs to stop climbing
	if (CheckShouldStopClimbing() || CheckHasReachedFloor())
	{
		StopClimbing();
	}

	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{	// Define the max climb speed and acceleration
		CalcVelocity(deltaTime, 0.f, true, MaxBreakClimbDeceleration);
	}

	ApplyRootMotionToVelocity(deltaTime);

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);

	// Handle climb rotation
	SafeMoveUpdatedComponent(Adjusted, GetClimbRotation(deltaTime), true, Hit);

	if (Hit.Time < 1.f)
	{
		// Adjust and try again
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}

	// Snap movement to climbable surfaces
	SnapMovementToClimableSurfaces(deltaTime);

	if (CheckHasReachedLedge())
	{
		PlayClimbMontage(ClimbToTopMontage);
	}
}

void UCustomMovementComponent::ProcessClimableSurfaceInfo()
{
	CurrentClimbableSurfaceLocation = FVector::ZeroVector;
	CurrentClimbableSurfaceNormal = FVector::ZeroVector;

	if (ClimbableSurfacesTracedResults.IsEmpty()) return;

	for (const FHitResult& TracedHitResult : ClimbableSurfacesTracedResults)
	{
		CurrentClimbableSurfaceLocation += TracedHitResult.ImpactPoint;
		CurrentClimbableSurfaceNormal += TracedHitResult.ImpactNormal;
	}

	CurrentClimbableSurfaceLocation /= ClimbableSurfacesTracedResults.Num();
	CurrentClimbableSurfaceNormal = CurrentClimbableSurfaceNormal.GetSafeNormal();
}

bool UCustomMovementComponent::CheckShouldStopClimbing()
{
	if (ClimbableSurfacesTracedResults.IsEmpty()) return true;

	const float DotResult = FVector::DotProduct(CurrentClimbableSurfaceNormal, FVector::UpVector);
	const float DegreeDiff = FMath::RadiansToDegrees(FMath::Acos(DotResult));

	if (DegreeDiff <= 60.f) // Check for the angle of the surface
	{
		return true;
	}

	return false;
}

bool UCustomMovementComponent::CheckHasReachedFloor()
{
	const FVector DownVector = -UpdatedComponent->GetUpVector();
	const FVector StartOffset = DownVector * 50.f;

	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + DownVector;

	TArray<FHitResult> PossibleFloorHits = DoCapsuleTraceMultiByObject(Start, End); //ds

	if (PossibleFloorHits.IsEmpty()) return false;

	for (const FHitResult& PossibleFloorHit : PossibleFloorHits)
	{
		const bool bFloorReached =
			FVector::Parallel(-PossibleFloorHit.ImpactNormal, FVector::UpVector) &&
			GetUnrotatedClimbVelocity().Z < -10.f;

		if (bFloorReached)
		{
			return true;
		}
	}

	return false;
}

FQuat UCustomMovementComponent::GetClimbRotation(float DeltaTime)
{
	const FQuat CurrentQuat = UpdatedComponent->GetComponentQuat();

	if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
	{
		return CurrentQuat;
	}

	const FQuat TargetQuat = FRotationMatrix::MakeFromX(-CurrentClimbableSurfaceNormal).ToQuat();

	return FMath::QInterpTo(CurrentQuat, TargetQuat, DeltaTime, 5.f);
}

void UCustomMovementComponent::SnapMovementToClimableSurfaces(float DeltaTime)
{
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();

	const FVector ProjectedCharacterToSurface =
		(CurrentClimbableSurfaceLocation - ComponentLocation).ProjectOnTo(ComponentForward);

	const FVector SnapVector = -CurrentClimbableSurfaceNormal * ProjectedCharacterToSurface.Length();

	UpdatedComponent->MoveComponent(
		SnapVector * DeltaTime * MaxClimbSpeed,
		UpdatedComponent->GetComponentQuat(),
		true);
}

bool UCustomMovementComponent::CheckHasReachedLedge()
{
	FHitResult LedgetHitResult = TraceFromEyeHeight(100.f, 50.f);

	if (!LedgetHitResult.bBlockingHit)
	{
		const FVector WalkableSurfaceTraceStart = LedgetHitResult.TraceEnd;

		const FVector DownVector = -UpdatedComponent->GetUpVector();
		const FVector WalkableSurfaceTraceEnd = WalkableSurfaceTraceStart + DownVector * 100.f;

		FHitResult WalkableSurfaceHitResult =
			DoLineTraceSingleByObject(WalkableSurfaceTraceStart, WalkableSurfaceTraceEnd); //ds

		if (WalkableSurfaceHitResult.bBlockingHit && GetUnrotatedClimbVelocity().Z > 10.f)
		{
			const float DotResult = FVector::DotProduct(WalkableSurfaceHitResult.ImpactNormal, FVector::UpVector);
			const float DegreeDifference = FMath::RadiansToDegrees(FMath::Acos(DotResult));

			if (DegreeDifference <= 60.f)
			{
				return true;
			}
		}
	}

	return false;
}


void UCustomMovementComponent::TryStartVaulting()
{
	FVector VaultStartPosition;
	FVector VaultEndPosition;

	if (CanStartVaulting(VaultStartPosition, VaultEndPosition))
	{
		// Start vaulting
		SetMotionWarpTarget(FName("VaultStartPoint"), VaultStartPosition);
		SetMotionWarpTarget(FName("VaultEndPoint"), VaultEndPosition);

		StartClimbing();
		PlayClimbMontage(VaultMontage);
	}
	else
	{
		// Unable to vault
	}
}

bool UCustomMovementComponent::CanStartVaulting(FVector& OutVaultStartPosition, FVector& OutVaultEndPosition)
{
	if (IsFalling()) return false;

	OutVaultStartPosition = FVector::ZeroVector;
	OutVaultEndPosition = FVector::ZeroVector;

	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector UpVector = UpdatedComponent->GetUpVector();
	const FVector DownVector = -UpdatedComponent->GetUpVector();

	for (int32 i = 0; i < 5; i++)
	{
		const FVector Start = ComponentLocation + UpVector * 100.f +
			ComponentForward * 80.f * (i + 1);

		const FVector End = Start + DownVector * 100.f * (i + 1);

		FHitResult VaultTraceHit = DoLineTraceSingleByObject(Start, End); //ds

		if (i == 0 && VaultTraceHit.bBlockingHit)
		{
			OutVaultStartPosition = VaultTraceHit.ImpactPoint;
		}

		if (i == 4 && VaultTraceHit.bBlockingHit) // At which point should the character land
		{
			OutVaultEndPosition = VaultTraceHit.ImpactPoint;
		}
	}

	if (OutVaultStartPosition != FVector::ZeroVector && OutVaultEndPosition != FVector::ZeroVector)
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool UCustomMovementComponent::IsClimbing() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == ECustomMovementMode::MOVE_Climb;
}

// Trace for climbable surfaces, return true if there valid walls
bool UCustomMovementComponent::TraceClimbableSurfaces()
{
	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 30.f;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector();

	ClimbableSurfacesTracedResults = DoCapsuleTraceMultiByObject(Start, End); //ds

	return !ClimbableSurfacesTracedResults.IsEmpty();
}

FHitResult UCustomMovementComponent::TraceFromEyeHeight(float TraceDistance, float TraceStartOffset, bool bShowDebugShape, bool bDrawPersistantShapes)
{
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector EyeHeightOffset = UpdatedComponent->GetUpVector() * (CharacterOwner->BaseEyeHeight + TraceStartOffset);

	const FVector Start = ComponentLocation + EyeHeightOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector() * TraceDistance;

	return DoLineTraceSingleByObject(Start, End, bShowDebugShape, bDrawPersistantShapes); //ds
}

void UCustomMovementComponent::PlayClimbMontage(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay) return;
	if (!OwningPlayerAnimInstance) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
}

void UCustomMovementComponent::OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == IdleToClimbMontage || Montage == ClimbDownLedgeMontage)
	{
		StartClimbing();
		StopMovementImmediately();
	}

	if (Montage == ClimbToTopMontage || Montage == VaultMontage)
	{
		SetMovementMode(MOVE_Walking);
	}
}

void UCustomMovementComponent::RequestHopping()
{
	const FVector UnrotatedLastInputVector =
		UKismetMathLibrary::Quat_UnrotateVector(UpdatedComponent->GetComponentQuat(), GetLastInputVector());

	const float DotResult =
		FVector::DotProduct(UnrotatedLastInputVector.GetSafeNormal(), FVector::UpVector);

	if (DotResult >= 0.9f)
	{
		HandleHopUp();
	}
	else if (DotResult <= -0.9f)
	{
		HandleHopDown();
	}
}

void UCustomMovementComponent::SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition)
{
	if (!OwningPlayerCharacter) return;

	OwningPlayerCharacter->GetMotionWarpingComponent()->AddOrUpdateWarpTargetFromLocation(
		InWarpTargetName,
		InTargetPosition
	);
}

void UCustomMovementComponent::HandleHopUp()
{
	FVector HopUpTargetPoint;

	if (CheckCanHopUp(HopUpTargetPoint))
	{
		SetMotionWarpTarget(FName("HopUpTargetPoint"), HopUpTargetPoint);

		PlayClimbMontage(HopUpMontage);
	}
}

bool UCustomMovementComponent::CheckCanHopUp(FVector& OutHopUpTargetPosition)
{
	FHitResult HopUpHit = TraceFromEyeHeight(100.f, -20.f);
	FHitResult SafetyLedgeHit = TraceFromEyeHeight(100.f, 150.f);

	if (HopUpHit.bBlockingHit && SafetyLedgeHit.bBlockingHit)
	{
		OutHopUpTargetPosition = HopUpHit.ImpactPoint;

		return true;
	}

	return false;
}

void UCustomMovementComponent::HandleHopDown()
{
	FVector HopDownTargetPoint;

	if (CheckCanHopDown(HopDownTargetPoint))
	{
		SetMotionWarpTarget(FName("HopDownTargetPoint"), HopDownTargetPoint);

		PlayClimbMontage(HopDownMontage);
	}
}

bool UCustomMovementComponent::CheckCanHopDown(FVector& OutHopDownTargetPosition)
{
	FHitResult HopDownHit = TraceFromEyeHeight(100.f, -300.f);

	if (HopDownHit.bBlockingHit)
	{
		OutHopDownTargetPosition = HopDownHit.ImpactPoint;

		return true;
	}

	return false;
}


FVector UCustomMovementComponent::GetUnrotatedClimbVelocity() const
{
	return UKismetMathLibrary::Quat_UnrotateVector(UpdatedComponent->GetComponentQuat(), Velocity);
}

#pragma endregion

// Wall Running doesn't work properly, the character doesn't stick to the wall and gravity is messed up - Wall Running is disabled for now
/*/bool UCustomMovementComponent::CanStartWallRunning()
{
	// The player must be falling or jumping, but not climbing or on the ground
	if (!IsFalling()) {
		return false;
	}

	// The trace must detect a suitable wall
	if (!TraceWallRunningSurfaces()) {
		return false;
	}

	return true;
}

void UCustomMovementComponent::StartWallRunning()
{
	if (CanStartWallRunning()) {
		SetMovementMode(MOVE_Custom, ECustomMovementMode::MOVE_WallRun);
		UE_LOG(LogTemp, Warning, TEXT("Wall Running Started!"));
	}
}

void UCustomMovementComponent::StopWallRunning()
{
	if (bIsWallRunning()) {
		SetMovementMode(MOVE_Falling);
		UE_LOG(LogTemp, Warning, TEXT("Wall Running Stopped!"));
	}
}

// Highlight: Implementation of PhysWallRun
void UCustomMovementComponent::PhysWallRun(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME || !CanStartWallRunning()) {
		StopWallRunning();
		return;
	}

	FVector InputDirection = GetLastInputVector().GetClampedToMaxSize(1.0f);  // Normalize input vector
	FVector WallNormal = LastHitResult.ImpactNormal;
	FVector ForwardDirection = FVector::VectorPlaneProject(CharacterOwner->GetActorForwardVector(), WallNormal).GetSafeNormal();

	// Calculate desired direction combining forward direction and wall normal
	FVector DesiredDirection = (ForwardDirection + InputDirection * 0.1f).GetSafeNormal();
	FVector CurrentVelocity = Velocity;
	FVector ParallelVelocity = FVector::VectorPlaneProject(CurrentVelocity, WallNormal);

	// Adjust velocity magnitude to maintain a consistent wall running speed
	ParallelVelocity = DesiredDirection * WallRunSpeed;  // Directly set speed along the desired direction

	Velocity = ParallelVelocity;
	MoveUpdatedComponent(Velocity * deltaTime, UpdatedComponent->GetComponentQuat(), true);
}

// Highlight: Implementation of CheckShouldStopWallRunning
bool UCustomMovementComponent::CheckShouldStopWallRunning()
{
	// Example condition: stop if there is no wall ahead
	if (!TraceWallRunningSurfaces()) {
		return true;
	}

	// Additional conditions can be added here, such as checking for player input to jump off the wall
	return false;
}

void UCustomMovementComponent::HandleJumpInput(float DeltaTime)
{
	if (CharacterOwner->bPressedJump && MovementMode == MOVE_Custom && CustomMovementMode == ECustomMovementMode::MOVE_WallRun)
	{
		FVector JumpDirection = CharacterOwner->GetActorUpVector() + CharacterOwner->GetActorForwardVector();
		Velocity += JumpDirection * WallRunJumpForce;  // Define WallRunJumpForce appropriately
		StopWallRunning();
	}
}


bool UCustomMovementComponent::CheckForWallTransition()
{
	FVector SideDirection = FVector::CrossProduct(LastHitResult.ImpactNormal, FVector::UpVector);
	FVector Start = CharacterOwner->GetActorLocation();
	FVector End = Start + SideDirection * 100;  // Check 100 units to the side for another wall

	FHitResult SideHitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(CharacterOwner);
	bool bHasSideWall = GetWorld()->LineTraceSingleByChannel(SideHitResult, Start, End, ECC_Visibility, CollisionParams);

	if (bHasSideWall)
	{
		// Transition to the new wall
		LastHitResult = SideHitResult;  // Update LastHitResult to the new wall's hit result
		return true;
	}
	return false;
}

bool UCustomMovementComponent::bIsWallRunning() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == ECustomMovementMode::MOVE_WallRun;
} */