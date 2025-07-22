// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BaseCharacter.generated.h"

UCLASS()
class MULTIACTIONGAME_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	bool bIsSprinting;
	bool bIsJumping;

	virtual const float GetMaxSprintSpeed() {
		return 800.f;
	}
	virtual const float GetMaxWalkSpeed() {
		return 400.f;
	}

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void Jump() override;
	virtual void StopJumping() override;

	virtual void Move(const FInputActionValue& value);
	virtual void Look(const FInputActionValue& value);
	virtual void StartSprint();
	virtual void StopSprint();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> Camera;

private:
	TObjectPtr<class UInputActionGroup> InputActionGroup;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
