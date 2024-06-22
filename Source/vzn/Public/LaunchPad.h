// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaunchPad.generated.h"

UCLASS()
class VZN_API ALaunchPad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaunchPad();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* LaunchPadMesh;

	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* BoxCollider;

	// For changing material based on whether the player is able to use it or not (Changes when player steps on it)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface* LaunchPadInactiveMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials", meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface* LaunchPadActiveMaterial;

	// Box collider for the launch pad, will detect when player steps on it
	UFUNCTION()
	void LaunchPadStartTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void LaunchPadEndTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};