// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "HealthBarWidget.generated.h"

class UProgressBar;
class UBackProgressBar;
class UHealthComponent;

UCLASS()
class MULTIACTIONGAME_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeHealthComponent(UHealthComponent* HealthComp);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 부드러운 체력 증가
	void UpdateHealthSmoothing(float Delta);

	UFUNCTION(BlueprintCallable)
	void UpdateHealthBar(float NewHealth, float MaxHealth);

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* HealthProgressBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UBackProgressBar* HealthBackProgressBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* NameText;

private:
	bool bIsHealthIncreased;
	float TargetHealthPercent;
};
