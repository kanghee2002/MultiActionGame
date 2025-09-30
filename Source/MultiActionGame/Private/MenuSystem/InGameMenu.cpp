// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuSystem/InGameMenu.h"
#include "MultiGameInstance.h"

#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"

bool UInGameMenu::Initialize()
{
	bool Success = Super::Initialize();

	if (!Success)
	{
		return false;
	}

	if (!ensure(CancelButton != nullptr)) return false;
	CancelButton->OnClicked.AddDynamic(this, &UInGameMenu::Cancel);

	if (!ensure(QuitButton != nullptr)) return false;
	QuitButton->OnClicked.AddDynamic(this, &UInGameMenu::Quit);

	if (!ensure(GraphicSettingButton != nullptr)) return false;
	GraphicSettingButton->OnClicked.AddDynamic(this, &UInGameMenu::OpenGraphicMenu);

	if (!ensure(GraphicCancelButton != nullptr)) return false;
	GraphicCancelButton->OnClicked.AddDynamic(this, &UInGameMenu::OpenInGameMenu);

	if (!ensure(EpicButton != nullptr)) return false;
	EpicButton->OnClicked.AddDynamic(this, &UInGameMenu::SetGraphicEpic);

	if (!ensure(HighButton != nullptr)) return false;
	HighButton->OnClicked.AddDynamic(this, &UInGameMenu::SetGraphicHigh);

	if (!ensure(MediumButton != nullptr)) return false;
	MediumButton->OnClicked.AddDynamic(this, &UInGameMenu::SetGraphicMedium);

	if (!ensure(LowButton != nullptr)) return false;
	LowButton->OnClicked.AddDynamic(this, &UInGameMenu::SetGraphicLow);

	return true;
}

void UInGameMenu::Cancel()
{
	Teardown();
}

void UInGameMenu::Quit()
{
	if (MenuInterface != nullptr)
	{
		Teardown();

		MenuInterface->LoadMainMenu();
	}
}

void UInGameMenu::OpenGraphicMenu()
{
	if (MenuSwitcher && GraphicMenu)
	{
		MenuSwitcher->SetActiveWidget(GraphicMenu);
	}
}

void UInGameMenu::OpenInGameMenu()
{
	if (MenuSwitcher && InGameMenu)
	{
		MenuSwitcher->SetActiveWidget(InGameMenu);
	}
}

void UInGameMenu::SetGraphicEpic()
{
	if (UMultiGameInstance* multiGameInstance = Cast<UMultiGameInstance>(GetGameInstance()))
	{
		multiGameInstance->SetGraphicSetting(EGraphicSetting::Epic);
	}
}

void UInGameMenu::SetGraphicHigh()
{
	if (UMultiGameInstance* multiGameInstance = Cast<UMultiGameInstance>(GetGameInstance()))
	{
		multiGameInstance->SetGraphicSetting(EGraphicSetting::High);
	}
}

void UInGameMenu::SetGraphicMedium()
{
	if (UMultiGameInstance* multiGameInstance = Cast<UMultiGameInstance>(GetGameInstance()))
	{
		multiGameInstance->SetGraphicSetting(EGraphicSetting::Medium);
	}
}

void UInGameMenu::SetGraphicLow()
{
	if (UMultiGameInstance* multiGameInstance = Cast<UMultiGameInstance>(GetGameInstance()))
	{
		multiGameInstance->SetGraphicSetting(EGraphicSetting::Low);
	}
}