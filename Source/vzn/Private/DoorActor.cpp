// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorActor.h"
#include "Components/BoxComponent.h"


// Sets default values
ADoorActor::ADoorActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DoorFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door Frame Mesh"));
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door Mesh"));

	DoorTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("Door Timeline"));
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));

	DoorFrameMesh->SetupAttachment(RootComponent);
	DoorMesh->AttachToComponent(DoorFrameMesh, FAttachmentTransformRules::KeepRelativeTransform);
	BoxCollider->AttachToComponent(DoorFrameMesh, FAttachmentTransformRules::KeepRelativeTransform);

}

// Called when the game starts or when spawned
void ADoorActor::BeginPlay()
{
    Super::BeginPlay();

	UpdateFunctionFloat.BindDynamic(this, &ADoorActor::UpdateTimelineComp);

	if (DoorTimelineFloatCurve)
	{
		DoorTimelineComp->AddInterpFloat(DoorTimelineFloatCurve, UpdateFunctionFloat);
	}

	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ADoorActor::DoorStartTrigger);
	BoxCollider->OnComponentEndOverlap.AddDynamic(this, &ADoorActor::DoorEndTrigger);
}

// Called every frame
void ADoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADoorActor::UpdateTimelineComp(float Output)
{
	FRotator NewRotation = FRotator(0.f, Output, 0.f);
	DoorMesh->SetRelativeRotation(NewRotation);
}

void ADoorActor::DoorStartTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	DoorTimelineComp->Play();
}

void ADoorActor::DoorEndTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	DoorTimelineComp->Reverse();
}