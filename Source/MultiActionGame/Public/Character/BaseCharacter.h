// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "Net/UnrealNetwork.h"
#include "BaseCharacter.generated.h"

UCLASS()
class MULTIACTIONGAME_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bCanDoComboAttack;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bCanPlayerControl;

	UFUNCTION(BlueprintCallable)
	bool const IsInvincible()
	{
		return bIsInvincible;
	}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> Camera;

	UPROPERTY(BlueprintReadWrite)
	float AttackStaminaCost;

	UPROPERTY(Replicated)
	FRotator ReplicatedRotation;

	FRotator TargetRotation;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bIsInvincible;

	virtual const float GetMaxSprintSpeed() 
	{
		return 800.0f;
	}
	virtual const float GetMaxWalkSpeed() 
	{
		return 400.0f;
	}

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void SynchronizeRotation(float DeltaTime);

	virtual void Move(const FInputActionValue& value);
	virtual void Look(const FInputActionValue& value);
	virtual void StartSprint();
	virtual void StopSprint();

	virtual void LightAttack();
	virtual void HeavyAttack();
	virtual void Roll();
	virtual void SelfHeal();

	// Sprint
	UFUNCTION(Server, Reliable)
	void Server_StartSprint();

	UFUNCTION(Server, Reliable)
	void Server_StopSprint();

	// Rotation
	UFUNCTION(Server, Reliable)
	void Server_SetRotation(FRotator NewRotation);

	// Attack
	// 서버에서만 실행
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Combat")
	void Server_Attack();

	// 모든 클라(서버 포함)에서 실행
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Combat")
	void Multicast_PlayAttackAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	bool BP_CanAttack() const;

	// 공격과 관련된 계산 적용
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_ExecuteAttack();

	// 공격 애니메이션 실행
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_PlayAttackAnimation();

	// Hit
	UFUNCTION(BlueprintNativeEvent)
	void OnDamageReceived(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Combat")
	void Multicast_PlayHitAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_PlayHitAnimation();

	// Roll
	UFUNCTION(Server, Reliable)
	void Server_Roll();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	bool BP_CanRoll() const;

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Combat")
	void Multicast_PlayRollAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_PlayRollAnimation();

	// Self Heal
	UFUNCTION(Server, Reliable)
	void Server_SelfHeal();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	bool BP_CanSelfHeal() const;

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Combat")
	void Multicast_PlaySelfHealAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_PlaySelfHealAnimation();


private:
	TObjectPtr<class UInputActionGroup> InputActionGroup;

	UPROPERTY()
	UHealthComponent* HealthCompRef;

	UStaminaComponent* StaminaCompRef;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
