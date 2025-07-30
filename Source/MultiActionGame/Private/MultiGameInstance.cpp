// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiGameInstance.h"
#include "BluePrint/UserWidget.h"

#include "MenuSystem/MainMenu.h"
//#include "MenuSystem/InGameMenu.h"
#include "MenuSystem/MenuWidget.h"

UMultiGameInstance::UMultiGameInstance(const FObjectInitializer& ObjectInitializer)
{
	// Set MenuClass
	ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/MenuSystem/WBP_MainMenu"));
	if (!ensure(MenuBPClass.Class != nullptr)) return;

	MenuClass = MenuBPClass.Class;

	// Set InGameMenuClass
	//ConstructorHelpers::FClassFinder<UUserWidget> InGameMenuBPClass(TEXT("/Game/MenuSystem/WBP_InGameMenu"));
	//if (!ensure(InGameMenuBPClass.Class != nullptr)) return;

	//InGameMenuClass = InGameMenuBPClass.Class;
}

void UMultiGameInstance::LoadMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("Load Menu"));

	if (!ensure(MenuClass != nullptr)) return;

	Menu = CreateWidget<UMainMenu>(this, MenuClass);
	if (!ensure(Menu != nullptr)) return;

	Menu->Setup();

	Menu->SetMenuInterface(this);
}

void UMultiGameInstance::LoadInGameMenu()
{
	/*if (!ensure(InGameMenuClass != nullptr)) return;

	InGameMenu = CreateWidget<UInGameMenu>(this, InGameMenuClass);
	if (!ensure(InGameMenu != nullptr)) return;

	InGameMenu->Setup();

	InGameMenu->SetMenuInterface(this);*/
}

void UMultiGameInstance::Host()
{
	/*if (Menu != nullptr)
	{
		Menu->Teardown();
	}

	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

	World->ServerTravel("/Game/ThirdPerson/Lvl_ThirdPerson?listen");*/
}


void UMultiGameInstance::Join(const FString& Address)
{
	/*if (Menu != nullptr)
	{
		Menu->Teardown();
	}

	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;

	PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);*/
}

void UMultiGameInstance::LoadMainMenu()
{
	/*APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;

	PlayerController->ClientTravel("/Game/MenuSystem/MainMenu", ETravelType::TRAVEL_Absolute);*/
}
