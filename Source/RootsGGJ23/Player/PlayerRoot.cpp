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
	PlayerCamera->SetWorldLocation(GetActorLocation() + FVector(-CameraDistance, 0.f, 0.f));
	
	// Flipbook asset component
	FlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Flipbook Component"));
	FlipbookComponent->SetupAttachment(RootComponent);
	FlipbookComponent->SetRelativeRotation(FRotator(0, 90, 0));
	
	// The movement blocking boxes
	LeftBlockingBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Left Blocking Box"));
	RightBlockingBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Right Blocking Box"));
	LeftBlockingBox->SetupAttachment(PlayerCamera);
	RightBlockingBox->SetupAttachment(PlayerCamera);
	LeftBlockingBox->SetBoxExtent(FVector(1.f, 1.f, 1000.f));
	RightBlockingBox->SetBoxExtent(FVector(1.f, 1.f, 1000.f));
	LeftBlockingBox->bHiddenInGame = false;
	RightBlockingBox->bHiddenInGame = false;
}

// Called whenever a value is changed
void APlayerRoot::OnConstruction(const FTransform& Transform)
{
	if(Flipbook) FlipbookComponent->SetFlipbook(Flipbook);
	Acceleration = FMath::Clamp(Acceleration, 0.01, 100.f);
	//MaxSpeed = FMath::Clamp(MaxSpeed, Acceleration, 10000.f);
	TurnSpeed = FMath::Clamp(TurnSpeed, 0.01, 10.f);
	PlayerCamera->SetWorldLocation(GetActorLocation() + FVector(-CameraDistance, 0.f, 0.f));
	LeftBlockingBox->SetWorldLocation(PlayerCamera->GetComponentLocation() + FVector(CameraDistance, -SideDistance, 0.f));
	RightBlockingBox->SetWorldLocation(PlayerCamera->GetComponentLocation() + FVector(CameraDistance, SideDistance, 0.f));
}

// Called when the game starts or when spawned
void APlayerRoot::BeginPlay()
{
	Super::BeginPlay();
	
	// Panic setup
	if(Flipbook) FlipbookComponent->SetFlipbook(Flipbook);
	CameraLoc = PlayerCamera->GetComponentLocation();
	
	// Movement
	IsBlockingLeft = false;
	IsBlockingRight = false;
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
	
	//Blocking
	IsBlockingLeft = GetActorLocation().Y <= PlayerCamera->GetComponentLocation().Y - SideDistance;
	IsBlockingRight = GetActorLocation().Y >= PlayerCamera->GetComponentLocation().Y + SideDistance;
}

// Called to move right on-screen
void APlayerRoot::MoveRight(float AxisValue)
{
	// Log turn point for spline
	if (AxisValue != LastAxis)
	{
		LastAxis = AxisValue;
		PathPoints.Add(GetActorLocation());
	}
	
	// Movement
	Lerp = FMath::Clamp(Lerp + (TurnSpeed * AxisValue), -(int32(!IsBlockingLeft)), int32(!IsBlockingRight));
	if (AxisValue == 0.f) Lerp = Lerp > 0.025f ? Lerp - TurnSpeed : Lerp < -0.025f ? Lerp + TurnSpeed : 0;

}