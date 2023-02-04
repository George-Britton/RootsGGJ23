// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PaperFlipbookComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "PlayerRoot.generated.h"

// Delegate for the reach the top event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReachTop, TArray<FVector>, SplinePoints);
// Delegate for getting in range of the player
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInRangeOfPlayer, AActor*, OtherActor);
// Delegate for leaving the player
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOutOfRangeOfPlayer, AActor*, OtherActor);

UCLASS()
class ROOTSGGJ23_API APlayerRoot : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerRoot();
	
	// Camera component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraDistance = 500.f;
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	UCameraComponent* PlayerCamera = nullptr;
	
	// Flipbook mesh for the root
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	UPaperFlipbookComponent* FlipbookComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	UPaperFlipbook* Flipbook = nullptr;
	
	// Speed and movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxSpeed = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Acceleration = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CurrentSpeed = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float TurnSpeed = 0.025f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float TurnRate = 10.f;
	float Lerp = 0.f;
	FVector CameraLoc;
	
	// The spline path points
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	TArray<FVector> PathPoints;
	float LastAxis = 0.f;
	
	// The movement blocking collision boxes
	UPROPERTY(Editanywhere, BlueprintReadWrite, Category = "Movement")
	float SideDistance = 513.f;
	bool IsBlockingLeft = false;
	bool IsBlockingRight = false;
	bool IsGoingUp = true;
	
	// The despawn/respawn boxes
	UPROPERTY(Editanywhere, BlueprintReadWrite, Category = "Collision")
	UBoxComponent* BottomBox = nullptr;
	UPROPERTY(Editanywhere, BlueprintReadWrite, Category = "Collision")
	UBoxComponent* TopBox = nullptr;
	UPROPERTY(Editanywhere, BlueprintReadWrite, Category = "Collision")
	float DespawnBoxHeight = 1000.f;
	
	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnReachTop OnReachTop;
	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnOutOfRangeOfPlayer OnOutOfRangeOfPlayer;
	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnInRangeOfPlayer OnInRangeOfPlayer;
	
protected:
	// Called whenever a value is changed
	void OnConstruction(const FTransform& Transform);
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	// Overlaps
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Called to move right on-screen
	UFUNCTION()
		void MoveRight(float AxisValue);
	
	// Called to get the actual Z velocity
	UFUNCTION(BlueprintCallable, Category = "Movement")
		float GetZSpeed() { return CurrentSpeed * (FVector::DotProduct(GetActorUpVector(), FVector(0.f, 0.f, 1.f))); }
		
	// Called to announce reach top
	UFUNCTION(BlueprintCallable, Category = "Events")
	TArray<FVector> GetPathPoints() { return PathPoints; };
	UFUNCTION(BlueprintCallable, Category = "Delegates")
	void CallOnReachTop() { OnReachTop.Broadcast(PathPoints); };
};
