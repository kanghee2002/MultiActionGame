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

	MaxHealth = HealthComp->DefaultMaxHealth;

	HealthComp->OnHealthChanged.AddUniqueDynamic(this, &UHealthBarWidget::UpdateHealthBar);

	UpdateHealthBar(HealthComp->DefaultMaxHealth, 0.f);
}


void UHealthBarWidget::UpdateHealthBar(float NewHealth, float Delta)
{
	if (HealthProgressBar && MaxHealth > 0)
	{
		float percent = NewHealth / MaxHealth;
		HealthProgressBar->SetPercent(percent);
	}
}