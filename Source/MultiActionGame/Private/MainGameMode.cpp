// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "MainPlayerController.h"
#include "MultiGameInstance.h"
#include "Character/TestCharacter1.h"
#include "AIController.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

AMainGameMode::AMainGameMode()
{
	HeroDeathCount = 0;
}

bool AMainGameMode::IsBossType(ECharacterType CharacterType) const
{
	return CharacterType == ECharacterType::Boss;
}

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	UMultiGameInstance* multiGameInstance = GetGameInstance<UMultiGameInstance>();

	if (multiGameInstance->GetIsBossAI())
	{
		ABaseCharacter* boss = GetWorld()->SpawnActor<ABaseCharacter>(BossCharacter);

		TArray<AActor*> playerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), playerStarts);

		for (AActor* playerStart : playerStarts)
		{
			if (playerStart->ActorHasTag("Boss"))
			{
				boss->SetActorLocation(playerStart->GetActorLocation());
				break;
			}
		}

		AAIController* AIController = GetWorld()->SpawnActor<AAIController>(GruxAIController);

		if (AIController)
		{
			AIController->Possess(boss);
		}
	}
	//ABaseCharacter* spawnedBoss = GetWorld()->SpawnActor<ABaseCharacter>(BossCharacter);
	
	//spawnedBoss->SetOwner(GetWorld()->GetGameState());

	UE_LOG(LogTemp, Warning, TEXT("[GameMode] End Begin Play"));
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
				UE_LOG(LogTemp, Warning, TEXT("[GameMode] Choose Player Start -> Boss"));
				return playerStart;
			}
		}
	}
	else
	{
		TArray<AActor*> heroStarts;

		for (AActor* playerStart : playerStarts)
		{
			if (playerStart->ActorHasTag("Hero"))
			{
				TArray<AActor*> overlappingActors;
				playerStart->GetOverlappingActors(overlappingActors, APawn::StaticClass());

				if (overlappingActors.Num() == 0) // 주변에 오브젝트 없으면
				{
					heroStarts.Add(playerStart);
				}
			}
		}

		if (heroStarts.Num() > 0)
		{
			int32 randomIndex = FMath::RandRange(0, heroStarts.Num() - 1);
			AActor* randomStart = heroStarts[randomIndex];

			UE_LOG(LogTemp, Warning, TEXT("[GameMode] Choose Player Start -> Knight"));

			return randomStart;
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
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Spawn Boss"));
		break;
	case ECharacterType::Knight:
		PawnToSpawn = DefaultPawnClass;
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Spawn Knight"));
		break;
	case ECharacterType::Archer:
		PawnToSpawn = ArcherCharacter;
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Spawn Archer"));
		break;
	case ECharacterType::Wizard:
		PawnToSpawn = WizardCharacter;
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Spawn Wizard"));
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

			IncreaseBossMaxHealth();
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
			UE_LOG(LogTemp, Warning, TEXT("[GameMode] Boss Health Increased"));

			bossCharacter->GetHealthComponent()->IncreaseMaxHealth();
		}
	}
}

FString AMainGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
	const FString& Options, const FString& Portal)
{
	AMainPlayerController* PC = Cast<AMainPlayerController>(NewPlayerController);

	if (PC)
	{
		// Parse Character Type from Options String
		FString CharacterTypeStr;
		if (FParse::Value(*Options, TEXT("CharacterType="), CharacterTypeStr))
		{
			int32 ParsedIndex = FCString::Atoi(*CharacterTypeStr);
			PC->SelectedCharacterType = static_cast<ECharacterType>(ParsedIndex);
		}
	}

#if WITH_EDITOR
	// FOR DEBUG
	static int startCount = 0;
	/*if (startCount % 2 == 0)
	{
		PC->SelectedCharacterType = ECharacterType::Boss;
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Init Boss Player"));
	}
	else if (startCount % 2 == 1)
	{
		PC->SelectedCharacterType = ECharacterType::Knight;
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Init Knight Player"));
	}*/
	/*if (startCount % 3 == 0)
	{
		PC->SelectedCharacterType = ECharacterType::Boss;
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Init Boss Player"));
	}
	else if (startCount % 3 == 1)
	{
		PC->SelectedCharacterType = ECharacterType::Archer;
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Init Knight Player"));
	}
	else
	{
		PC->SelectedCharacterType = ECharacterType::Knight;
	}*/
	startCount++;
#endif

	FString OutError = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

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

void AMainGameMode::ProcessPlayerRevive()
{
	HeroDeathCount--;
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