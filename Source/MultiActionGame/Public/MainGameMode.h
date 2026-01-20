// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Character/CharacterType.h"
#include "Character/BaseCharacter.h"
#include "MainGameMode.generated.h"

class AAIController;

UCLASS()
class MULTIACTIONGAME_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()
private:
	virtual void BeginPlay() override;

	void ProcessGameOver(bool IsBossWin);

	void AddNewCharacter(APawn* NewPawn, ECharacterType CharacterType);

	void IncreaseBossMaxHealth();

protected:
	AMainGameMode();

	bool IsBossType(ECharacterType CharacterType) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Characters")
	TSubclassOf<ACharacter> KnightCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Characters")
	TSubclassOf<ACharacter> ArcherCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Characters")
	TSubclassOf<ACharacter> WizardCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Characters")
	TSubclassOf<ACharacter> BossCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TSubclassOf<AAIController> GruxAIController;

	UPROPERTY(BlueprintReadOnly)
	TArray<ABaseCharacter*> BossCharacters;

	UPROPERTY(BlueprintReadOnly)
	TArray<ABaseCharacter*> HeroCharacters;

	int HeroDeathCount;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player);

	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot);

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
		const FString& Options, const FString& Portal = TEXT("")) override;

public:
	TArray<ABaseCharacter*> GetHeroCharacters() const
	{
		return HeroCharacters;
	}

	void ProcessPlayerDeath(ECharacterType CharacterType);

	void ProcessPlayerRevive();
};
