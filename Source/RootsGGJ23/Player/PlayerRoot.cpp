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
	BottomBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Bottom Box"));
	TopBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Top Box"));
	BottomBox->SetupAttachment(PlayerCamera);
	TopBox->SetupAttachment(PlayerCamera);
	BottomBox->SetBoxExtent(FVector(10000.f, 10000.f, 1.f));
	TopBox->SetBoxExtent(FVector(10000.f, 10000.f, 1.f));
	BottomBox->bHiddenInGame = false;
	TopBox->bHiddenInGame = false;
}

// Called whenever a value is changed
void APlayerRoot::OnConstruction(const FTransform& Transform)
{
	if(Flipbook) FlipbookComponent->SetFlipbook(Flipbook);
	Acceleration = FMath::Clamp(Acceleration, 0.01, 100.f);
	TurnSpeed = FMath::Clamp(TurnSpeed, 0.01, 10.f);
	PlayerCamera->SetWorldLocation(GetActorLocation() + FVector(-CameraDistance, 0.f, 200.f));
	BottomBox->SetWorldLocation(PlayerCamera->GetComponentLocation() - FVector(0.f, 0.f, DespawnBoxHeight));
	TopBox->SetWorldLocation(PlayerCamera->GetComponentLocation() + FVector(0.f, 0.f, DespawnBoxHeight));
}

// Called when the game starts or when spawned
void APlayerRoot::BeginPlay()
{
	Super::BeginPlay();
	
	// Panic setup
	if(Flipbook) FlipbookComponent->SetFlipbook(Flipbook);
	CameraLoc = PlayerCamera->GetComponentLocation();
	
	// Set top and bottom box bindings
	BottomBox->OnComponentEndOverlap.AddDynamic(this, &APlayerRoot::OnOverlapEnd);
	TopBox->OnComponentBeginOverlap.AddDynamic(this, &APlayerRoot::OnOverlapBegin);
	
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

// Overlaps
void APlayerRoot::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComp == TopBox && IsGoingUp) OnInRangeOfPlayer.Broadcast(OtherActor);
}
void APlayerRoot::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OverlappedComp == BottomBox && IsGoingUp) OnOutOfRangeOfPlayer.Broadcast(OtherActor);
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
	PlayerCamera->SetWorldLocation(FVector(CameraLoc.X, CameraLoc.Y, GetActorLocation().Z + 200));
	
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