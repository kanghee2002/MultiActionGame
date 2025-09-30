// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"

#include "MainPlayerController.h"
#include "MultiGameInstance.h"
#include "Character/TestCharacter1.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

AMainGameMode::AMainGameMode()
{
	HeroDeathCount = 0;
}

AActor* AMainGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<AActor*> playerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), playerStarts);

	if (IsBossType(Cast<AMainPlayerController>(Player)->SelectedCharacterType))
	{
		for (AActor* playerStart : playerStarts)
		{
			if (playerStart->ActorHasTag("Boss"))
			{
				return playerStart;
			}
		}
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

APawn* AMainGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	AMainPlayerController* PC = Cast<AMainPlayerController>(NewPlayer);
	if (PC == nullptr) return Super::SpawnDefaultPawnFor(NewPlayer, StartSpot);

	TSubclassOf<APawn> PawnToSpawn;
	TSubclassOf<APlayerController> ControllerToUse = PlayerControllerClass;

	static int count = 0;
	count++;
	switch (PC->SelectedCharacterType)
	{
	case ECharacterType::Boss:
		PawnToSpawn = BossCharacter;

#if WITH_EDITOR
		// FOR DEBUG
		if (count % 2 == 1)
		{
			PawnToSpawn = BossCharacter;
			PC->SelectedCharacterType = ECharacterType::Boss;
			UE_LOG(LogTemp, Warning, TEXT("[GameMode] Spawn Boss"));
		}
		else
		{
			PawnToSpawn = KnightCharacter;
			PC->SelectedCharacterType = ECharacterType::Knight;
			UE_LOG(LogTemp, Warning, TEXT("[GameMode] Spawn Knight"));
		}
#endif

		break;
	case ECharacterType::Knight:
		PawnToSpawn = DefaultPawnClass;
		break;
	case ECharacterType::Archer:
	case ECharacterType::Healer:
		PawnToSpawn = ATestCharacter1::StaticClass();

		// TODO
		break;
	default:
		PawnToSpawn = DefaultPawnClass;
		break;
	}

	FVector SpawnLocation = StartSpot->GetActorLocation();
	FRotator SpawnRotation = StartSpot->GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;

	APawn* NewPawn = GetWorld()->SpawnActor<APawn>(PawnToSpawn, SpawnLocation, SpawnRotation, SpawnParams);

	AddNewCharacter(NewPawn, PC->SelectedCharacterType);

	return NewPawn;
}

void AMainGameMode::AddNewCharacter(APawn* NewPawn, ECharacterType CharacterType)
{
	if (ABaseCharacter* newCharacter = Cast<ABaseCharacter>(NewPawn))
	{
		if (IsBossType(CharacterType))
		{
			BossCharacters.Add(newCharacter);
			UE_LOG(LogTemp, Warning, TEXT("[GameMode] Add Boss to Array"));
		}
		else
		{
			HeroCharacters.Add(newCharacter);
			UE_LOG(LogTemp, Warning, TEXT("[GameMode] Add Hero to Array"));
		}
	}
}

void AMainGameMode::IncreaseBossMaxHealth()
{
	if (BossCharacters.IsEmpty() || HeroCharacters.Num() <= 1)
	{
		return;
	}

	for (ABaseCharacter* bossCharacter : BossCharacters)
	{
		if (bossCharacter)
		{
			bossCharacter->GetHealthComponent()->IncreaseMaxHealth();
		}
	}
}

FString AMainGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
	const FString& Options, const FString& Portal)
{
	FString OutError = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	AMainPlayerController* PC = Cast<AMainPlayerController>(NewPlayerController);
	if (!PC) return OutError;

	// Parse Character Type from Options String
	FString CharacterTypeStr;
	if (FParse::Value(*Options, TEXT("CharacterType="), CharacterTypeStr))
	{
		int32 ParsedIndex = FCString::Atoi(*CharacterTypeStr);
		PC->SelectedCharacterType = static_cast<ECharacterType>(ParsedIndex);
	}

	return OutError;
}

void AMainGameMode::ProcessPlayerDeath(ECharacterType CharacterType)
{
	if (IsBossType(CharacterType))
	{
		ProcessGameOver(false);
	}
	else
	{
		HeroDeathCount++;
	}

	if (HeroDeathCount == HeroCharacters.Num())
	{
		ProcessGameOver(true);
	}
}

void AMainGameMode::ProcessGameOver(bool IsBossWin)
{
	int playerDeathCount = GetNumPlayers() - 1;
	for (FConstPlayerControllerIterator iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator)
	{
		AMainPlayerController* controller = Cast<AMainPlayerController>(iterator->Get());

		if (controller->SelectedCharacterType == ECharacterType::Boss)
		{
			controller->Client_CreateGameOverWidget(IsBossWin);
		}
		else
		{
			controller->Client_CreateGameOverWidget(!IsBossWin);
		}
	}
}