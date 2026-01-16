// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BossAIController.h"

#include "Character/BaseCharacter.h"
#include "MainGameMode.h"

#include "TimerManager.h"
#include "BehaviorTree/BlackboardComponent.h"

ABossAIController::ABossAIController()
{

}

void ABossAIController::SetNextPattern()
{
	SetTarget();

	SetAction(ECharacterAction::None);
	
	TArray<ECharacterAction> patterns;
	patterns.Init(ECharacterAction::None, 0);

	AddPattern(patterns, ECharacterAction::LightAttack, 2);
	AddPattern(patterns, ECharacterAction::HeavyAttack, 2);
	AddPattern(patterns, ECharacterAction::Jump, 1);

	if (Cast<ABaseCharacter>(GetPawn())->IsSkillReady())
	{
		AddPattern(patterns, ECharacterAction::Skill, 2);
	}

	float distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());

	if (distance >= 1000.0f)
	{
		AddPattern(patterns, ECharacterAction::Jump, 1);
	}

	if (distance >= 1500.0f)
	{
		AddPattern(patterns, ECharacterAction::Jump, 3);
	}

	ECharacterAction action = ECharacterAction::LightAttack;
	if (!patterns.IsEmpty())
	{
		int32 index = FMath::RandRange(0, patterns.Num() - 1);
		action = patterns[index];
	}

	SetAction(action);
}

float ABossAIController::GetCharacterTypeScore(ECharacterType characterType)
{
	float weight = 0.0f;
	switch (characterType)
	{
	case ECharacterType::Boss:
		break;
	case ECharacterType::Knight:
		weight = KnightWeight;
		break;
	case ECharacterType::Archer:
		weight = ArcherWeight;
		break;
	case ECharacterType::Wizard:
		weight = WizardWeight;
		break;
	case ECharacterType::MAX:
		break;
	default:
		break;
	}

	float score = weight * CharacterTypeMultiplier;

	return score;
}

float ABossAIController::GetHealthScore(float currentHealth, float maxHealth)
{
	float healthRatio = currentHealth / maxHealth;

	float weight = 1.0f - healthRatio;

	float score = weight * HealthMultiplier;

	if (healthRatio <= 0.3f)
	{
		score += 3.0f;
	}

	return score;
}

float ABossAIController::GetDistanceScore(float distance)
{
	float weight = 0.25f;
	if (distance <= 700.0f)
	{
		weight = 1.0f;
	}
	else if (distance <= 1000.0f)
	{
		weight = 0.5f;
	}

	float score = weight * DistanceMultiplier;
	
	return score;
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!BehaviorTree || !BlackboardData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Boss AI] BehaviorTree or BlackboardData not set"));
		return;
	}

	ABaseCharacter* baseCharacter = Cast<ABaseCharacter>(InPawn);
	baseCharacter->BP_SetAIStat();

	UBlackboardComponent* BB;
	UseBlackboard(BlackboardData, BB);

	RunBehaviorTree(BehaviorTree);

	FTimerHandle delayTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		delayTimerHandle,
		this,
		&ABossAIController::RunAI,
		3.0f,
		false   // 반복 여부
	);
}

void ABossAIController::RunAI()
{
	SetTarget();

	SetAction(ECharacterAction::LightAttack);
}

void ABossAIController::SetTarget()
{
	AGameModeBase* gameMode = GetWorld()->GetAuthGameMode();
	AMainGameMode* mainGameMode = Cast<AMainGameMode>(gameMode);

	TArray<ABaseCharacter*> heroCharacters = mainGameMode->GetHeroCharacters();
	
	for (int32 i = heroCharacters.Num() - 1; i >= 0; i--)
	{
		if (heroCharacters[i]->IsFaint())
		{
			heroCharacters.RemoveAt(i);
		}
	}

	if (heroCharacters.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Boss AI] heroCharacters is empty"));
		return;
	}

	if (heroCharacters.Num() == 1)
	{
		CurrentTarget = heroCharacters[0];
		AActor* target = CurrentTarget;
		GetBlackboardComponent()->SetValueAsObject(Target, target);
		return;
	}

	// Make Target Scores
	TArray<float> targetScores;
	targetScores.Init(0.0f, heroCharacters.Num());

	float scoreSum = 0.0f;
	for (int32 i = 0; i < targetScores.Num(); i++)
	{
		ECharacterType characterType = heroCharacters[i]->GetCharacterType();
		float currentHealth = heroCharacters[i]->GetHealthComponent()->GetCurrentHealth();
		float maxHealth = heroCharacters[i]->GetHealthComponent()->GetMaxHealth();
		float distance = FVector::Dist(GetPawn()->GetActorLocation(), heroCharacters[i]->GetActorLocation());

		targetScores[i] += GetCharacterTypeScore(characterType);
		targetScores[i] += GetHealthScore(currentHealth, maxHealth);
		targetScores[i] += GetDistanceScore(distance);

		scoreSum += targetScores[i];
	}

	// Random Selection using Cumulative Sum
	float randomValue = FMath::FRandRange(0.0f, scoreSum);
	float tmpSum = 0.0f;
	int32 selectedIndex = 0;
	for (int32 i = 0; i < targetScores.Num(); i++)
	{
		tmpSum += targetScores[i];
		if (randomValue <= tmpSum)
		{
			selectedIndex = i;
			break;
		}
	}

	// Set Target on Blackboard
	CurrentTarget = heroCharacters[selectedIndex];
	AActor* target = CurrentTarget;
	GetBlackboardComponent()->SetValueAsObject(Target, target);
}

void ABossAIController::SetAction(ECharacterAction action)
{
	GetBlackboardComponent()->SetValueAsEnum(NextAction, static_cast<uint8>(action));
}
