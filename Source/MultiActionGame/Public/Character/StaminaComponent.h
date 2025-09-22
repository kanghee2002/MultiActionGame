// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "StaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChanged, float, NewStamina);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIACTIONGAME_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStaminaComponent();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStaminaChanged OnStaminaChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float DefaultMaxStamina;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentStamina)
	float CurrentStamina;

	UPROPERTY(BlueprintReadWrite)
	float StaminaRecoveryRate;

	UFUNCTION(BlueprintCallable)
	void StartRecovery();

	UFUNCTION(BlueprintCallable)
	void StopRecovery();

	bool TryUseStamina(float Amount);

	UFUNCTION()
	void OnRep_CurrentStamina();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	bool const HasAuthority()
	{
		if (GetOwner()->HasAuthority())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool IsRecovering;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
