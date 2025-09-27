// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"
#include "MainPlayerController.h"
#include "MultiGameInstance.h"
#include "Character/TestCharacter1.h"

AMainGameMode::AMainGameMode()
{
	HeroCount = 0;
	HeroDeathCount = 0;
}

APawn* AMainGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	AMainPlayerController* PC = Cast<AMainPlayerController>(NewPlayer);
	if (PC == nullptr) return Super::SpawnDefaultPawnFor(NewPlayer, StartSpot);

	TSubclassOf<APawn> PawnToSpawn;
	TSubclassOf<APlayerController> ControllerToUse = PlayerControllerClass;

	// Log Character Type ---------------------------------------------------------------------------------------
	const UEnum* EnumPtr = StaticEnum<ECharacterType>();

	if (EnumPtr)
	{
		FString CharacterTypeName = EnumPtr->GetNameStringByValue(static_cast<int64>(PC->SelectedCharacterType));

		//UE_LOG(LogTemp, Warning, TEXT("Selected Character Type: %s"), *CharacterTypeName);
	}
	// ----------------------------------------------------------------------------------------------------------

	static int count = 0;
	count++;
	switch (PC->SelectedCharacterType)
	{
	case ECharacterType::Boss:
		PawnToSpawn = BossCharacter;

#if WITH_EDITOR
		// FOR DEBUG
		if (count % 2 == 0)
		{
			PawnToSpawn = BossCharacter;
			PC->SelectedCharacterType = ECharacterType::Boss;
			UE_LOG(LogTemp, Warning, TEXT("[GameMode] Spawn Boss"));
		}
		else
		{
			PawnToSpawn = KnightCharacter;
			PC->SelectedCharacterType = ECharacterType::Knight;
			HeroCount++;
			UE_LOG(LogTemp, Warning, TEXT("[GameMode] Spawn Knight"));
		}
#endif
		break;
	case ECharacterType::Knight:
		PawnToSpawn = DefaultPawnClass;
		HeroCount++;
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

	if (NewPawn)
	{
		//PC->Possess(NewPawn);
		UE_LOG(LogTemp, Warning, TEXT("Done Setting Character : %s"), *NewPawn->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn pawn"));
	}

	return NewPawn;
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

		//UE_LOG(LogTemp, Warning, TEXT("InitNewPlayer - Parsed CharacterType from URL: %d"), ParsedIndex);
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("InitNewPlayer - No CharacterType in URL"));
	}

	return OutError;
}

void AMainGameMode::ProcessPlayerDeath(ECharacterType CharacterType)
{
	switch (CharacterType)
	{
	case ECharacterType::Boss:
		ProcessGameOver(false);
		break;
	case ECharacterType::Knight:
	case ECharacterType::Archer:
	case ECharacterType::Healer:
		HeroDeathCount++;
		break;
	default:
		break;
	}

	if (HeroDeathCount == HeroCount)
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