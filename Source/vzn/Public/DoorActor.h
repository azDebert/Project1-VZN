// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimeLineComponent.h"
#include "DoorActor.generated.h"

UCLASS()
class VZN_API ADoorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoorActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DoorTimelineFloatCurve;
private:

	// Door Meshes
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* DoorFrameMesh;
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* DoorMesh;

	// Animate the door meshes
	UPROPERTY(EditDefaultsOnly)
	UTimelineComponent* DoorTimelineComp;

	// Trigger Box for the door to open
	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* BoxCollider;

	// Float Track Signature that will be used to update the door
	FOnTimelineFloat UpdateFunctionFloat;

	// Updates the door's rotation based on the timeline
	UFUNCTION()
	void UpdateTimelineComp(float Output);

	// Door Trigger Functions
	UFUNCTION()
	void DoorStartTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
				int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void DoorEndTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
