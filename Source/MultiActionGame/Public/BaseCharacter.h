// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "BaseCharacter.generated.h"

UCLASS()
class MULTIACTIONGAME_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsSprinting;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsJumping;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bCanDoComboAttack;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bCanPlayerControl;

protected:
	virtual const float GetMaxSprintSpeed() {
		return 800.f;
	}
	virtual const float GetMaxWalkSpeed() {
		return 400.f;
	}

	virtual void BeginPlay() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void Jump() override;
	virtual void StopJumping() override;

	virtual void Move(const FInputActionValue& value);
	virtual void Look(const FInputActionValue& value);
	virtual void StartSprint();
	virtual void StopSprint();

	virtual void LightAttack();
	virtual void HeavyAttack();

	// Movement
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> Camera;

	// Server
	UFUNCTION(Server, Reliable)
	void Server_StartSprint();

	UFUNCTION(Server, Reliable)
	void Server_StopSprint();

	UFUNCTION(Server, Reliable)
	void Server_SetRotation(FRotator NewRotation);

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

	UFUNCTION()
	void OnDamageReceived(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Combat")
	void Multicast_PlayHitAnimation();

	// 피격 애니메이션 실행
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_PlayHitAnimation();

	UPROPERTY(Replicated)
	FRotator ReplicatedRotation;

	FRotator TargetRotation;

private:
	TObjectPtr<class UInputActionGroup> InputActionGroup;

	UPROPERTY()
	UHealthComponent* HealthCompRef;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
