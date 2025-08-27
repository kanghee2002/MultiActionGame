// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "BaseCharacter.generated.h"

UCLASS()
class MULTIACTIONGAME_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsSprinting;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsJumping;

protected:
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

	// Movement
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> Camera;

	// Server
	UFUNCTION(Server, Reliable)
	void ServerStartSprint();

	UFUNCTION(Server, Reliable)
	void ServerStopSprint();

	UFUNCTION(Server, Reliable)
	void ServerSetRotation(FRotator NewRotation);

	UPROPERTY(Replicated)
	FRotator ReplicatedRotation;

private:
	TObjectPtr<class UInputActionGroup> InputActionGroup;

	UPROPERTY()
	UHealthComponent* HealthCompRef;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
