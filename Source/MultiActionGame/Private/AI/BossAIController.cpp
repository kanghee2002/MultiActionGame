// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BossAIController.h"

#include "Character/BaseCharacter.h"
#include "MainGameMode.h"

#include "BehaviorTree/BlackboardComponent.h"

ABossAIController::ABossAIController()
{

}

void ABossAIController::SetNextPattern()
{
	SetTarget();

	SetAction(ECharacterAction::None);

	SetAction(static_cast<ECharacterAction>(FMath::RandRange(1, 1)));
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
		UE_LOG(LogTemp, Warning, TEXT("BehaviorTree or BlackboardData not set"));
		return;
	}

	ABaseCharacter* baseCharacter = Cast<ABaseCharacter>(InPawn);
	baseCharacter->BP_SetAIStat();

	UBlackboardComponent* BB;
	UseBlackboard(BlackboardData, BB);

	RunBehaviorTree(BehaviorTree);

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

		UE_LOG(LogTemp, Warning, TEXT("%d is %f"), i, targetScores[i]);
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

	AActor* target = heroCharacters[selectedIndex];
	GetBlackboardComponent()->SetValueAsObject(Target, target);
}

void ABossAIController::SetAction(ECharacterAction action)
{
	GetBlackboardComponent()->SetValueAsEnum(NextAction, static_cast<uint8>(action));
}
