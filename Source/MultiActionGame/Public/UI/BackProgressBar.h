// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ProgressBar.h"
#include "Components/Widget.h"
#include "BackProgressBar.generated.h"

/**
 * 
 */
UCLASS()
class MULTIACTIONGAME_API UBackProgressBar : public UProgressBar
{
	GENERATED_BODY()

public:
	UBackProgressBar();

	UFUNCTION(BlueprintCallable)
	void SetTargetPercent(float NewPercent);

	UFUNCTION(BlueprintCallable)
	void StopUpdatePercent();

	float DecreaseDelay = 0.75f;
	float UpdateInterval = 0.016f;
	float DecreaseSpeed = 1.0f;

private:
	FTimerHandle DelayTimerHandle;    // 대기용
	FTimerHandle UpdateTimerHandle;   // 업데이트용

	float TargetPercent;
	bool bIsDecreasing;

	void UpdatePercent();

};
