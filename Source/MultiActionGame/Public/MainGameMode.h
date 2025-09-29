// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Character/CharacterType.h"
#include "Character/BaseCharacter.h"
#include "MainGameMode.generated.h"

UCLASS()
class MULTIACTIONGAME_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()
private:
	void ProcessGameOver(bool IsBossWin);

	void AddNewCharacter(APawn* NewPawn, ECharacterType CharacterType);

	void IncreaseBossMaxHealth();

protected:
	AMainGameMode();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Characters")
	TSubclassOf<ACharacter> KnightCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Characters")
	TSubclassOf<ACharacter> BossCharacter;

	UPROPERTY()
	TArray<ABaseCharacter*> BossCharacters;

	UPROPERTY()
	TArray<ABaseCharacter*> HeroCharacters;

	int HeroDeathCount;

	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot);

	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
		const FString& Options, const FString& Portal = TEXT("")) override;

public:
	void ProcessPlayerDeath(ECharacterType CharacterType);
};
