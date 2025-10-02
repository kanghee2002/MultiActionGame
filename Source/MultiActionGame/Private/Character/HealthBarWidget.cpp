// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HealthBarWidget.h"
#include "UI/BackProgressBar.h"
#include "Components/ProgressBar.h"
#include "Character/HealthComponent.h"

void UHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsHealthIncreased)
	{
		UpdateHealthSmoothing(InDeltaTime);
	}
}

void UHealthBarWidget::UpdateHealthSmoothing(float Delta)
{
	float currentPercent = HealthProgressBar->GetPercent();

	if (currentPercent >= TargetHealthPercent)
	{
		bIsHealthIncreased = false;

		HealthProgressBar->SetPercent(TargetHealthPercent);
		HealthBackProgressBar->SetPercent(TargetHealthPercent);
		return;
	}

	float increasingSpeed = 1.0f;

	float newPercent = currentPercent + increasingSpeed * Delta;
	HealthProgressBar->SetPercent(newPercent);
	HealthBackProgressBar->SetPercent(newPercent);
}


void UHealthBarWidget::InitializeHealthComponent(UHealthComponent* HealthComp)
{
	if (!HealthComp)
	{
		return;
	}

	HealthComp->OnHealthChanged.AddUniqueDynamic(this, &UHealthBarWidget::UpdateHealthBar);

	float maxHealth = HealthComp->GetMaxHealth();

	UpdateHealthBar(maxHealth, maxHealth);

	HealthBackProgressBar->SetPercent(1.0f);
}


void UHealthBarWidget::UpdateHealthBar(float NewHealth, float MaxHealth)
{

	if (HealthProgressBar && MaxHealth > 0.0f)
	{
		float percent = NewHealth / MaxHealth;

		// 체력 감소한 경우
		if (HealthProgressBar->GetPercent() >= percent)
		{
			HealthBackProgressBar->SetTargetPercent(percent);
			HealthProgressBar->SetPercent(percent);
			bIsHealthIncreased = false;
		}
		// 체력 증가한 경우
		else
		{
			// 체력 부드럽게 증가
			TargetHealthPercent = percent;
			bIsHealthIncreased = true;
		}
	}
}