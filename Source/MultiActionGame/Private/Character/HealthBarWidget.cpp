// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Character/HealthComponent.h"


void UHealthBarWidget::InitializeHealthComponent(UHealthComponent* HealthComp)
{
	if (!HealthComp)
	{
		return;
	}

	HealthComp->OnHealthChanged.AddUniqueDynamic(this, &UHealthBarWidget::UpdateHealthBar);

	float maxHealth = HealthComp->GetMaxHealth();

	UpdateHealthBar(maxHealth, maxHealth);
}


void UHealthBarWidget::UpdateHealthBar(float NewHealth, float MaxHealth)
{
	if (HealthProgressBar && MaxHealth > 0.0f)
	{
		float percent = NewHealth / MaxHealth;
		HealthProgressBar->SetPercent(percent);
	}
}