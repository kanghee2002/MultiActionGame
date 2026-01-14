// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Character/CharacterType.h"
#include "CharacterAction.h"
#include "BossAIController.generated.h"

UCLASS()
class MULTIACTIONGAME_API ABossAIController : public AAIController
{
	GENERATED_BODY()

public:
	ABossAIController();

	UFUNCTION(BlueprintCallable)
	void SetNextPattern();

protected:
	// AI Target Select Formula
	const float CharacterTypeMultiplier = 1.0f;
	const float KnightWeight = 0.2f;
	const float ArcherWeight = 0.8f;
	const float WizardWeight = 0.8f;

	const float HealthMultiplier = 3.0f;
	const float LowHealthBonus = 3.0f;
	
	const float DistanceMultiplier = 1.0f;

	float GetCharacterTypeScore(ECharacterType characterType);
	float GetHealthScore(float health, float maxHealth);
	float GetDistanceScore(float distance);

protected:
	const FName Target = "Target";
	const FName NextAction = "NextAction";

	virtual void OnPossess(APawn* InPawn) override;

	void SetTarget();

	void SetAction(ECharacterAction action);

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBlackboardData* BlackboardData;
};
