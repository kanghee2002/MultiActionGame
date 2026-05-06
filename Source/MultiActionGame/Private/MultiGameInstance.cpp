// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiGameInstance.h"
#include "BluePrint/UserWidget.h"

#include "MenuSystem/MainMenu.h"
#include "MenuSystem/InGameMenu.h"
#include "MenuSystem/MenuWidget.h"

#include "GameFramework/GameUserSettings.h"

UMultiGameInstance::UMultiGameInstance(const FObjectInitializer& ObjectInitializer)
{
	// Set MenuClass
	ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/MenuSystem/WBP_MainMenu"));
	if (!ensure(MenuBPClass.Class != nullptr)) return;

	MenuClass = MenuBPClass.Class;

	// Set InGameMenuClass
	ConstructorHelpers::FClassFinder<UUserWidget> InGameMenuBPClass(TEXT("/Game/MenuSystem/WBP_InGameMenu"));
	if (!ensure(InGameMenuBPClass.Class != nullptr)) return;

	InGameMenuClass = InGameMenuBPClass.Class;

	BossHealth = -1.0f;
	BossAttackDamage = -1.0f;
	BossAttackCost = -1.0f;
	BossSkillCooldown = -1.0f;

	IsBossAI = false;
}

void UMultiGameInstance::LoadMenu()
{
	if (IsRunningDedicatedServer()) return;

	if (!ensure(MenuClass != nullptr)) return;

	Menu = CreateWidget<UMainMenu>(this, MenuClass);
	if (!ensure(Menu != nullptr)) return;

	Menu->Setup();

	Menu->SetMenuInterface(this);
}

void UMultiGameInstance::LoadInGameMenu()
{
	if (IsRunningDedicatedServer()) return;

	if (!ensure(InGameMenuClass != nullptr)) return;

	InGameMenu = CreateWidget<UInGameMenu>(this, InGameMenuClass);
	if (!ensure(InGameMenu != nullptr)) return;

	InGameMenu->Setup();

	InGameMenu->SetMenuInterface(this);
}

void UMultiGameInstance::Host(ECharacterType CharacterType)
{
	if (Menu != nullptr)
	{
		Menu->Teardown();
	}

	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

	//World->ServerTravel("/Game/ThirdPerson/Lvl_ThirdPerson?listen");

	//SelectedCharacterType = ECharacterType::Boss;

	FString Address = "/Game/TestMap?listen?game=/Game/General/MainGameModeBP.MainGameModeBP_C";

	FString TravelURL = FString::Printf(TEXT("%s?CharacterType=%d"), *Address, static_cast<int32>(CharacterType));

	if (IsBossAI)              TravelURL += TEXT("?BossAI");
	if (BossHealth > 0)        TravelURL += FString::Printf(TEXT("?BossHealth=%.1f"), BossHealth);
	if (BossAttackDamage > 0)  TravelURL += FString::Printf(TEXT("?BossAttackDamage=%.1f"), BossAttackDamage);
	if (BossAttackCost > 0)    TravelURL += FString::Printf(TEXT("?BossAttackCost=%.1f"), BossAttackCost);
	if (BossSkillCooldown > 0) TravelURL += FString::Printf(TEXT("?BossSkillCooldown=%.1f"), BossSkillCooldown);

	UE_LOG(LogTemp, Warning, TEXT("[Host] ServerTravel URL: %s"), *TravelURL);

	World->ServerTravel(TravelURL);
}


void UMultiGameInstance::Join(const FString& Address, ECharacterType CharacterType)
{
	if (Menu != nullptr)
	{
		Menu->Teardown();
	}

	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	SelectedCharacterType = CharacterType;

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));

	FString TravelURL = FString::Printf(TEXT("%s?CharacterType=%d"), *Address, static_cast<int32>(CharacterType));

	PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
}

void UMultiGameInstance::LoadMainMenu()
{
	if (IsRunningDedicatedServer()) return;

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;

	PlayerController->ClientTravel("/Game/MenuSystem/MainMenu", ETravelType::TRAVEL_Absolute);
}

void UMultiGameInstance::SetGraphicSetting(EGraphicSetting GraphicSetting)
{
	UGameUserSettings* Settings = GEngine->GetGameUserSettings();
	Settings->SetOverallScalabilityLevel(static_cast<int32>(GraphicSetting));
	Settings->ApplySettings(false);
	Settings->SaveSettings();
}
