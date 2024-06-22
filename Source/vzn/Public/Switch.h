// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Switch.generated.h"

UCLASS()
class VZN_API ASwitch : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASwitch();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// For selecting the platform that will be affected by the switch
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
	class AMovingPlatform* ConnectedPlatform;
	
	void OnActivate();

private: 
	UPROPERTY(EditDefaultsOnly, Category = "Components") class UBoxComponent* BoxCollider;
	UPROPERTY(EditDefaultsOnly, Category = "Components") class UStaticMeshComponent* SwitchMesh;
};
