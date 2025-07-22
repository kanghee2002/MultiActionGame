// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BaseAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MULTIACTIONGAME_API UBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	void JumpStart() {
		bIsLanded = false;
	}
	void Landed() {
		bIsLanded = true;
	}

protected:
	ACharacter* CachedCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim")
	float RunThreshold = 700.f;

	UPROPERTY(BlueprintReadOnly, Category = "Anim")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadonly, Category = "Anim")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadOnly, Category = "Anim")
	bool bIsLanded = false;
};
