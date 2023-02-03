// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PaperFlipbookComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "PlayerRoot.generated.h"

UCLASS()
class ROOTSGGJ23_API APlayerRoot : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerRoot();
	
	// Camera component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* PlayerCamera = nullptr;
	
	// Flipbook mesh for the root
	UPaperFlipbookComponent* FlipbookComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	UPaperFlipbook* Flipbook = nullptr;
	
	// Speed and movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxSpeed = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Acceleration = 10.f;
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
	
	// Called to move right on-screen
	UFUNCTION()
		void MoveRight(float AxisValue);
};
