// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "CharacterType.h"
#include "Net/UnrealNetwork.h"
#include "BaseCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealCountChanged, int, NewHealthCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillCooldownChanged, float, NewCooldown, float, Cooldown);

UCLASS()
class MULTIACTIONGAME_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealCountChanged OnHealCountChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSkillCooldownChanged OnSkillCooldownChanged;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bCanDoComboAttack;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bCanPlayerControl;

	UFUNCTION(BlueprintCallable)
	bool const IsInvincible()
	{
		return bIsInvincible;
	}

	UHealthComponent* GetHealthComponent() const
	{
		return HealthCompRef;
	}

	ECharacterType const GetCharacterType()
	{
		return CharacterType;
	}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UHealthComponent* HealthCompRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UStaminaComponent* StaminaCompRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	TObjectPtr<class UCapsuleComponent> BodyCapsule;

	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> HitEnemies;

	UPROPERTY(EditDefaultsOnly)
	ECharacterType CharacterType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BasicAttackDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float LightAttackStaminaCost;

	UPROPERTY(EditDefaultsOnly)
	float HeavyAttackStaminaCost;

	UPROPERTY(EditDefaultsOnly)
	float SkillStaminaCost;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float SkillCooldown;

	UPROPERTY(EditDefaultsOnly)
	float RollStaminaCost;

	UPROPERTY(EditDefaultsOnly)
	float SprintSpeed;

	UPROPERTY(EditDefaultsOnly)
	float WalkSpeed;

	UPROPERTY(EditDefaultsOnly)
	float RotationSpeed;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentHealCount)
	int CurrentHealCount;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentMaxSpeed)
	float CurrentSpeed;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentSkillCooldown)
	float CurrentSkillCooldown;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_ReplicatedRotation)
	FRotator ReplicatedRotation;

	UPROPERTY(BlueprintReadWrite)
	FRotator LocalRotation;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bIsInvincible;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bIsSuperArmored;

	UPROPERTY(BlueprintReadWrite)
	bool bIsFaint;

	UFUNCTION(BlueprintCallable)
	const void StartRecoveryStamina()
	{
		if (StaminaCompRef)
		{
			StaminaCompRef->StartRecovery();
		}
	}

	UFUNCTION(BlueprintCallable)
	const void StopRecoveryStamina()
	{
		if (StaminaCompRef)
		{
			StaminaCompRef->StopRecovery();
		}
	}

	UFUNCTION(BlueprintCallable)
	const void ClearHitEnemies()
	{
		HitEnemies.Empty();
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
	virtual void UseSkill();
	virtual void JumpAction();
	virtual void SelfHeal();

	UFUNCTION()
	void OnRep_CurrentHealCount();

	UFUNCTION()
	void OnRep_CurrentMaxSpeed();

	UFUNCTION()
	void OnRep_CurrentSkillCooldown();

	UFUNCTION()
	void OnRep_ReplicatedRotation();

	// Sprint
	UFUNCTION(Server, Reliable)
	void Server_StartSprint();

	UFUNCTION(Server, Reliable)
	void Server_StopSprint();

	// Rotation
	UFUNCTION(Server, Reliable)
	void Server_SetRotation(FRotator NewRotation);

	// Light Attack
	// 서버에서만 실행
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Combat")
	void Server_LightAttack();

	// 모든 클라(서버 포함)에서 실행
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Combat")
	void Multicast_PlayLightAttackAnimation();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Combat")
	void Server_HeavyAttack();

	// 모든 클라(서버 포함)에서 실행
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Combat")
	void Multicast_PlayHeavyAttackAnimation();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Combat")
	void Server_UseSkill();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Combat")
	void Multicast_PlaySkillAnimation();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Combat")
	bool BP_CanAttack() const;

	// 공격과 관련된 계산 적용
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_ExecuteLightAttack();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_ExecuteHeavyAttack();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_ExecuteSkill();

	// 공격 애니메이션 실행
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_PlayLightAttackAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_PlayHeavyAttackAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_PlaySkillAnimation();

	// 스킬 쿨다운 시작
	UFUNCTION(BlueprintCallable)
	void StartSkillCooldown();

	// 스킬 쿨다운 감소
	void DecreaseSkillCooldown();

	// Hit
	UFUNCTION(BlueprintNativeEvent)
	void OnDamageReceived(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Combat")
	void Multicast_PlayHitAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_PlayHitAnimation();

	// Jump (Roll)
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_JumpAction();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Combat")
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

	// Faint
	UFUNCTION()
	void OnFullHealth();

	UFUNCTION()
	void Revive();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Combat")
	void Multicast_PlayReviveAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_PlayReviveAnimation();

	// Death
	UFUNCTION(Server, Reliable)
	void OnDeath();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Combat")
	void Multicast_PlayDeathAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_PlayDeathAnimation();

private:
	TObjectPtr<class UInputActionGroup> InputActionGroup;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FTimerHandle SkillCooldownTimerHandle;   // 스킬 쿨다운 업데이트

	const float CooldownUpdateInterval = 0.1f;

	bool bUseReplicatedRotation;
};
