// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MULTIACTIONGAME_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void Move(const FInputActionValue& value);
	void Look(const FInputActionValue& value);
	void StartSprint();
	void StopSprint();
	void StartJump();
	void StopJump();

	UFUNCTION(BlueprintCallable)
	void OnCharacterLanded();

protected:
	bool bIsSprinting;
	bool bIsJumping;
	const float fMaxSpeedSprinting = 800.f;
	const float fMaxSpeedWalking = 400.f;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> MappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputActionGroup> InputActionGroup;
};
