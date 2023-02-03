// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerRoot.h"

// Sets default values
APlayerRoot::APlayerRoot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Possess player
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	
	// Create and attach camera, lock axes
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));
	
	// Flipbook asset component
	FlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Flipbook Component"));
	FlipbookComponent->SetupAttachment(RootComponent);
	FlipbookComponent->SetRelativeRotation(FRotator(0, 90, 0));
}

// Called whenever a value is changed
void APlayerRoot::OnConstruction(const FTransform& Transform)
{
	if(Flipbook) FlipbookComponent->SetFlipbook(Flipbook);
	Acceleration = FMath::Clamp(Acceleration, 0.01, 100.f);
	//MaxSpeed = FMath::Clamp(MaxSpeed, Acceleration, 10000.f);
	TurnSpeed = FMath::Clamp(TurnSpeed, 0.01, 10.f);
}

// Called when the game starts or when spawned
void APlayerRoot::BeginPlay()
{
	Super::BeginPlay();
	if(Flipbook) FlipbookComponent->SetFlipbook(Flipbook);
	CameraLoc = PlayerCamera->GetComponentLocation();
}

// Called to bind functionality to input
void APlayerRoot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind player movement
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerRoot::MoveRight);
}

// Called every frame
void APlayerRoot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Set Speed and turn
	CurrentSpeed = FMath::Clamp(CurrentSpeed + Acceleration, 0.f, MaxSpeed);
	
	// Move
	AddActorLocalOffset(GetActorUpVector() * CurrentSpeed);
	SetActorRotation(FRotator(0.f, 0.f, TurnRate * Lerp));
	PlayerCamera->SetWorldLocation(FVector(CameraLoc.X, CameraLoc.Y, GetActorLocation().Z));
}

// Called to move right on-screen
void APlayerRoot::MoveRight(float AxisValue)
{
	if (AxisValue != LastAxis)
	{
		LastAxis = AxisValue;
		PathPoints.Add(GetActorLocation());
	}
	if (AxisValue != 0.f) Lerp = FMath::Clamp(Lerp + (TurnSpeed * AxisValue), -1, 1);
	else Lerp = Lerp > 0.025f ? Lerp - TurnSpeed : Lerp < -0.025f ? Lerp + TurnSpeed : 0;
}