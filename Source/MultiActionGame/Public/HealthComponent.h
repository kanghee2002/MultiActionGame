// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTIACTIONGAME_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHealthComponent();

	// Heal �Լ�
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	// ü�� �ʱ�ȭ �Լ�
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION()
	void TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	/** ��ų �� ������ ���͸� */
	bool ShouldApplyDamage(AActor* DamageCauser, AController* InstigatedBy) const;


public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeath OnDeath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float DefaultMaxHealth;

	UPROPERTY(BlueprintReadOnly)
	float CurrentHealth;
};
