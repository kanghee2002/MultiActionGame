// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/StaminaComponent.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
	DefaultMaxStamina = 100;
	CurrentStamina = DefaultMaxStamina;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

bool UStaminaComponent::TryUseStamina(float Amount)
{
	if (CurrentStamina >= Amount)
	{
		CurrentStamina -= Amount;

		OnStaminaChanged.Broadcast(CurrentStamina);

		return true;
	}

	return false;
}

void UStaminaComponent::OnRep_CurrentStamina()
{
	OnStaminaChanged.Broadcast(CurrentStamina);
}

void UStaminaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStaminaComponent, CurrentStamina);
}

