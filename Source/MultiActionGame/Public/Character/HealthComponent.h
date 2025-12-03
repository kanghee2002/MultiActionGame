// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFullHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTIACTIONGAME_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFullHealth OnFullHealth;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeath OnDeath;

	float GetMaxHealth() const
	{
		return CurrentMaxHealth;
	}

	float GetCurrentHealth() const
	{
		return CurrentHealth;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float DefaultMaxHealth;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Health")
	float CurrentMaxHealth;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	// Heal 함수
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	// 체력 초기화 함수
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

	// 최대 체력 증가 함수
	UFUNCTION(BlueprintCallable, Category = "Health")
	void IncreaseMaxHealth();

	UFUNCTION()
	void OnRep_CurrentHealth();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	/** 팀킬 및 데미지 필터링 */
	bool ShouldApplyDamage(AActor* DamageCauser, AController* InstigatedBy) const;
};
