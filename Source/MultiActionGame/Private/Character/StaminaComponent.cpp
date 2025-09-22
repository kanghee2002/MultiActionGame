// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/StaminaComponent.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
	DefaultMaxStamina = 100;
	CurrentStamina = DefaultMaxStamina;
	StaminaRecoveryRate = 10.0f;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		IsRecovering = true;
	}
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (HasAuthority())
	{
		if (IsRecovering && CurrentStamina < DefaultMaxStamina)
		{
			CurrentStamina += StaminaRecoveryRate * DeltaTime;
			
			OnStaminaChanged.Broadcast(CurrentStamina);
		}
	}
}

void UStaminaComponent::StartRecovery()
{
	if (!HasAuthority())
	{
		return;
	}

	IsRecovering = true;
}

void UStaminaComponent::StopRecovery()
{
	if (!HasAuthority())
	{
		return;
	}

	IsRecovering = false;
}

bool UStaminaComponent::TryUseStamina(float Amount)
{
	if (!HasAuthority())
	{
		return false;
	}

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
	DOREPLIFETIME(UStaminaComponent, IsRecovering);
}

