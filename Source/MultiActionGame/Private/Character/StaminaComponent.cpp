// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/StaminaComponent.h"
#include "Character/BaseCharacter.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
	DefaultMaxStamina = 100;
	CurrentStamina = DefaultMaxStamina;
	StaminaRecoveryRate = 25.0f;
	ExhaustionRecoveryThreshold = 10.0f;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		StaminaState = EStaminaState::Normal;
	}
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (HasAuthority())
	{
		UpdateStamina(DeltaTime);
	}
}

void UStaminaComponent::UpdateStamina(float DeltaTime)
{
	if (CurrentStamina < 1.0f)
	{
		StaminaState = EStaminaState::Exhausted;
	}

	float deltaStamina = StaminaRecoveryRate * DeltaTime;

	switch (StaminaState)
	{
	case EStaminaState::Normal:
		if (CurrentStamina < DefaultMaxStamina)
		{
			CurrentStamina += deltaStamina;

			OnStaminaChanged.Broadcast(CurrentStamina);
		}

		//UE_LOG(LogTemp, Warning, TEXT("Stamina State: Normal"));
		break;
	case EStaminaState::Acting:

		//UE_LOG(LogTemp, Warning, TEXT("Stamina State: Acting"));
		break;
	case EStaminaState::Sprinting:
		if (CurrentStamina > 0.0f)
		{
			CurrentStamina -= deltaStamina * 0.5f;

			OnStaminaChanged.Broadcast(CurrentStamina);
		}

		if (CurrentStamina < 0.5f)
		{
			ABaseCharacter* character = Cast<ABaseCharacter>(GetOwner());
			if (character)
			{
				character->Server_StopSprint_Implementation();
			}
		}

		//UE_LOG(LogTemp, Warning, TEXT("Stamina State: Sprinting"));
		break;
	case EStaminaState::Exhausted:
		if (CurrentStamina < ExhaustionRecoveryThreshold)
		{
			CurrentStamina += deltaStamina / 4.0f;

			OnStaminaChanged.Broadcast(CurrentStamina);
		}
		else
		{
			StaminaState = EStaminaState::Normal;
		}

		//UE_LOG(LogTemp, Warning, TEXT("Stamina State: Exhausted"));
		break;
	case EStaminaState::MAX:
		break;
	default:
		break;
	}
}

void UStaminaComponent::StartRecovery()
{
	if (!HasAuthority())
	{
		return;
	}

	if (StaminaState != EStaminaState::Exhausted)
	{
		StaminaState = EStaminaState::Normal;
	}
}

void UStaminaComponent::StopRecovery()
{
	if (!HasAuthority())
	{
		return;
	}

	if (StaminaState != EStaminaState::Exhausted)
	{
		StaminaState = EStaminaState::Acting;
	}
}

bool UStaminaComponent::CanSprint()
{
	if (StaminaState == EStaminaState::Acting || StaminaState == EStaminaState::Exhausted)
	{
		return false;
	}

	if (CurrentStamina < 0.1f)
	{
		return false;
	}

	return true;
}

void UStaminaComponent::StartSprint()
{
	if (!HasAuthority())
	{
		return;
	}

	StaminaState = EStaminaState::Sprinting;
}

void UStaminaComponent::StopSprint()
{
	if (!HasAuthority())
	{
		return;
	}

	if (StaminaState == EStaminaState::Sprinting)
	{
		StaminaState = EStaminaState::Normal;
	}
}

bool UStaminaComponent::TryUseStamina(float Amount)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (StaminaState == EStaminaState::Exhausted)
	{
		return false;
	}

	if (CurrentStamina >= 0.0f)
	{
		CurrentStamina -= Amount;

		if (CurrentStamina <= 0.0f)
		{
			CurrentStamina = 0.0f;
		}

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
	DOREPLIFETIME(UStaminaComponent, StaminaState);
}

