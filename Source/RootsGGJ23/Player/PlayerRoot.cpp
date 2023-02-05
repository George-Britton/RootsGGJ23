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
	HeadFlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Flipbook Component"));
	HeadFlipbookComponent->SetupAttachment(RootComponent);
	HeadFlipbookComponent->SetRelativeRotation(FRotator(0, 90, 0));
	HeadFlipbookComponent->SetRelativeScale3D(FVector(0.3, 1.f, 0.3));

	DrillFlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Drill Component"));
	DrillFlipbookComponent->SetupAttachment(RootComponent);
	DrillFlipbookComponent->SetRelativeRotation(FRotator(0, 90, 0));
	DrillFlipbookComponent->SetRelativeScale3D(FVector(0.f, 1.f, 0.f));
	SoundComp = CreateDefaultSubobject<UAudioComponent>(TEXT("Sound"));

	
	// Tail component
	// TailSplineComponent = CreateDefaultSubobject<USplineMeshComponent>(TEXT("Tail Component"));
	// TailSplineComponent->SetupAttachment(RootComponent);
	// TailSplineComponent->AddWorldOffset(FVector(2.f, 0.f, 0.f));
	
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
	if(HeadFlipbook) HeadFlipbookComponent->SetFlipbook(HeadFlipbook);

	if(DrillFlipbook) DrillFlipbookComponent->SetFlipbook(DrillFlipbook);

	Acceleration = FMath::Clamp(Acceleration, 0.01, 100.f);
	TurnSpeed = FMath::Clamp(TurnSpeed, 0.01, 10.f);
	PlayerCamera->SetWorldLocation(GetActorLocation() + FVector(-CameraDistance, 0.f, 100.f));
	BottomBox->SetWorldLocation(PlayerCamera->GetComponentLocation() - FVector(0.f, 0.f, DespawnBoxHeight));
	TopBox->SetWorldLocation(PlayerCamera->GetComponentLocation() + FVector(0.f, 0.f, DespawnBoxHeight));
}

// Called when the game starts or when spawned
void APlayerRoot::BeginPlay()
{
	Super::BeginPlay();
	ResetMaxSpeed = MaxSpeed;
	
	// Panic setup
	if(HeadFlipbook) HeadFlipbookComponent->SetFlipbook(HeadFlipbook);

	if(DrillFlipbook) DrillFlipbookComponent->SetFlipbook(DrillFlipbook);

	CameraLoc = PlayerCamera->GetComponentLocation();
	
	// Set top and bottom box bindings
	BottomBox->OnComponentEndOverlap.AddDynamic(this, &APlayerRoot::OnOverlapEnd);
	TopBox->OnComponentBeginOverlap.AddDynamic(this, &APlayerRoot::OnOverlapBegin);
	
	// Add first spline point
	PathPoints.Add(GetActorLocation());
	PathPoints.Add(GetActorLocation() + FVector(0.f, 0.f, 1.f));
	
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

// Called to slow the player and tell it it's been hit
void APlayerRoot::Bonk(AActor* HitActor)
{

	if (IsProtected)
	{
		ResetFace();
		if (DrillSound)
		{
			DrillFlipbookComponent->SetRelativeScale3D(FVector(0.f, 1.f, 0.f));
			SoundComp->SetSound(DrillSound);
			SoundComp->Play();
			HitActor->Destroy();
		}
	} else{
		Ouchie = true;
		HeadFlipbookComponent->SetFlipbook(SadFlipbook);
		CurrentSpeed = 0.f;
		FTimerHandle OuchTimer;
		GetWorld()->GetTimerManager().SetTimer(OuchTimer, this, &APlayerRoot::ResetFace, 1.5f);
	}
}
void APlayerRoot::Chomp(AActor* HitActor)
{

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

// Pickups
void APlayerRoot::Protect()
{
	if (!IsProtected)
	{
		DrillFlipbookComponent->SetRelativeScale3D(FVector(0.3, 1.f, 0.3));
		HeadFlipbookComponent->SetFlipbook(AngeryFlipbook);
		IsProtected = true;
	}
}
void APlayerRoot::Zoom()
{
	if (!IsFast)
	{
		MaxSpeed = ResetMaxSpeed * 2;
		HeadFlipbookComponent->SetFlipbook(FastFlipbook);
		IsFast = true;
		FTimerHandle FastTimer;
		GetWorld()->GetTimerManager().SetTimer(FastTimer, this, &APlayerRoot::ResetFace, 5.f);
	}
}
void APlayerRoot::ResetFace()
{
	HeadFlipbookComponent->SetFlipbook(HeadFlipbook);
	if (Ouchie)
	{
		if (IsFast)
		{
			HeadFlipbookComponent->SetFlipbook(FastFlipbook);
		}
		else{
			HeadFlipbookComponent->SetFlipbook(HeadFlipbook);
		}
	}
	else if (IsFast)
	{
		IsFast = false;
		MaxSpeed = ResetMaxSpeed;
	}
}

// Called every frame
void APlayerRoot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if(IsGoingUp)
	{
		// Set Speed and turn
		CurrentSpeed = FMath::Clamp(CurrentSpeed + Acceleration, 0.f, MaxSpeed);
		
		// Move
		AddActorLocalOffset(GetActorUpVector() * CurrentSpeed);
		SetActorRotation(FRotator(0.f, 0.f, TurnRate * Lerp));
		PlayerCamera->SetWorldLocation(FVector(CameraLoc.X, CameraLoc.Y, GetActorLocation().Z + 100.f));
		
		//Blocking
		IsBlockingLeft = GetActorLocation().Y <= PlayerCamera->GetComponentLocation().Y - SideDistance;
		IsBlockingRight = GetActorLocation().Y >= PlayerCamera->GetComponentLocation().Y + SideDistance;
	} else CurrentSpeed = 0.f;
}

// Called to move right on-screen
void APlayerRoot::MoveRight(float AxisValue)
{
	if(IsGoingUp)
	{

		if (!GotFirstSpline)
		{
			PathPoints.Add(GetActorLocation());
			GotFirstSpline = true;
		}

		if (AxisValue != LastAxis)
		{
			LastAxis = AxisValue;
			PathPoints.Add(GetActorLocation());
		}

		if (FVector::Distance(PathPoints[PathPoints.Num() - 1], GetActorLocation()) >= 261)
		{
			PathPoints.Add(GetActorLocation());
		}

		
		// Movement
		Lerp = FMath::Clamp(Lerp + (TurnSpeed * AxisValue), -(int32(!IsBlockingLeft)), int32(!IsBlockingRight));
		if (AxisValue == 0.f) Lerp = Lerp > 0.025f ? Lerp - TurnSpeed : Lerp < -0.025f ? Lerp + TurnSpeed : 0;
	}
}