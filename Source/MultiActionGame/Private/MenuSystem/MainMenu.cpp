// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuSystem/MainMenu.h"
#include "MenuSystem/MenuInterface.h"
#include "MainPlayerController.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"
#include "Misc/DefaultValueHelper.h"

#include "MultiGameInstance.h"


bool UMainMenu::Initialize()
{
	bool Success = Super::Initialize();

	if (!Success)
	{
		return false;
	}

	// Initialize Buttons
	if (!ensure(HostButton != nullptr)) return false;
	HostButton->OnClicked.Clear();
	HostButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

	if (!ensure(JoinButton != nullptr)) return false;
	JoinButton->OnClicked.Clear();
	JoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

	if (!ensure(QuitButton != nullptr)) return false;
	QuitButton->OnClicked.AddDynamic(this, &UMainMenu::Quit);

	if (!ensure(CancelJoinButton != nullptr)) return false;
	CancelJoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

	if (!ensure(ConfirmJoinButton != nullptr)) return false;
	ConfirmJoinButton->OnClicked.AddDynamic(this, &UMainMenu::JoinServer);

	if (!ensure(GraphicSettingButton != nullptr)) return false;
	GraphicSettingButton->OnClicked.AddDynamic(this, &UMainMenu::OpenGraphicMenu);

	if (!ensure(SettingConfirmButton != nullptr)) return false;
	SettingConfirmButton->OnClicked.AddDynamic(this, &UMainMenu::ConfirmSetting);

	if (!ensure(EpicButton != nullptr)) return false;
	EpicButton->OnClicked.AddDynamic(this, &UMainMenu::SetGraphicEpic);

	if (!ensure(HighButton != nullptr)) return false;
	HighButton->OnClicked.AddDynamic(this, &UMainMenu::SetGraphicHigh);

	if (!ensure(MediumButton != nullptr)) return false;
	MediumButton->OnClicked.AddDynamic(this, &UMainMenu::SetGraphicMedium);

	if (!ensure(LowButton != nullptr)) return false;
	LowButton->OnClicked.AddDynamic(this, &UMainMenu::SetGraphicLow);

	// Initialize Character Select CheckBoxes
	CharacterCheckBoxes.Empty();

	CharacterCheckBoxes.Add(KnightCheckBox);
	CharacterCheckBoxes.Add(ArcherCheckBox);
	CharacterCheckBoxes.Add(HealerCheckBox);
	
	for (URadioCheckBox* CheckBox : CharacterCheckBoxes)
	{
		if (CheckBox)
		{
			CheckBox->Initialize(CharacterCheckBoxes);
			CheckBox->OnSelected.AddDynamic(this, &UMainMenu::SelectCharacter);
		}
	}

	KnightCheckBox->SetCheckedState(ECheckBoxState::Checked);
	SelectedCharacterType = ECharacterType::Knight;

	return true;
}

void UMainMenu::HostServer()
{
	if (MenuInterface != nullptr)
	{
		MenuInterface->Host();
	}
}

void UMainMenu::JoinServer()
{
	if (MenuInterface != nullptr)
	{
		if (!ensure(IPAddressField != nullptr)) return;

		const FString& Address = IPAddressField->GetText().ToString();
		MenuInterface->Join(Address, SelectedCharacterType);
	}
}

void UMainMenu::OpenJoinMenu()
{
	if (!ensure(MenuSwitcher != nullptr)) return;
	if (!ensure(JoinMenu != nullptr)) return;

	MenuSwitcher->SetActiveWidget(JoinMenu);
}

void UMainMenu::OpenMainMenu()
{
	if (!ensure(MenuSwitcher != nullptr)) return;
	if (!ensure(MainMenu != nullptr)) return;

	MenuSwitcher->SetActiveWidget(MainMenu);
}


void UMainMenu::Quit()
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr)) return;

	PlayerController->ConsoleCommand("Quit");
}

void UMainMenu::UncheckAll(bool bIsChecked)
{
	for (UCheckBox* CheckBox : CharacterCheckBoxes)
	{
		CheckBox->SetCheckedState(ECheckBoxState::Unchecked);
	}
}

void UMainMenu::OpenGraphicMenu()
{
	if (MenuSwitcher && GraphicMenu)
	{
		MenuSwitcher->SetActiveWidget(GraphicMenu);
	}
}

void UMainMenu::SetGraphicEpic()
{
	if (UMultiGameInstance* multiGameInstance = Cast<UMultiGameInstance>(GetGameInstance()))
	{
		multiGameInstance->SetGraphicSetting(EGraphicSetting::Epic);
	}
}

void UMainMenu::SetGraphicHigh()
{
	if (UMultiGameInstance* multiGameInstance = Cast<UMultiGameInstance>(GetGameInstance()))
	{
		multiGameInstance->SetGraphicSetting(EGraphicSetting::High);
	}
}

void UMainMenu::SetGraphicMedium()
{
	if (UMultiGameInstance* multiGameInstance = Cast<UMultiGameInstance>(GetGameInstance()))
	{
		multiGameInstance->SetGraphicSetting(EGraphicSetting::Medium);
	}
}

void UMainMenu::SetGraphicLow()
{
	if (UMultiGameInstance* multiGameInstance = Cast<UMultiGameInstance>(GetGameInstance()))
	{
		multiGameInstance->SetGraphicSetting(EGraphicSetting::Low);
	}
}

void UMainMenu::ConfirmSetting()
{
	UMultiGameInstance* multiGameInstace = Cast<UMultiGameInstance>(GetGameInstance());

	const FString& bossHealthString = BossHealthInput->GetText().ToString();
	float NewBossHealth;

	const FString& bossAttackDamageString = BossAttackDamageInput->GetText().ToString();
	float NewBossAttackDamage;

	const FString& bossAttackCostString = BossAttackCostInput->GetText().ToString();
	float NewBossAttackCost;

	const FString& bossSkillCooldownString = BossSkillCooldownInput->GetText().ToString();
	float NewBossSkillCooldown;

	if (FDefaultValueHelper::ParseFloat(bossHealthString, NewBossHealth) && multiGameInstace)
	{
		multiGameInstace->BossHealth = NewBossHealth;
	}

	if (FDefaultValueHelper::ParseFloat(bossAttackDamageString, NewBossAttackDamage) && multiGameInstace)
	{
		multiGameInstace->BossAttackDamage = NewBossAttackDamage;
	}

	if (FDefaultValueHelper::ParseFloat(bossAttackCostString, NewBossAttackCost) && multiGameInstace)
	{
		multiGameInstace->BossAttackCost = NewBossAttackCost;
	}

	if (FDefaultValueHelper::ParseFloat(bossSkillCooldownString, NewBossSkillCooldown) && multiGameInstace)
	{
		multiGameInstace->BossSkillCooldown = NewBossSkillCooldown;
	}

	OpenMainMenu();
}

void UMainMenu::SelectCharacter(int32 Index)
{
	if (Index >= 0 && Index < static_cast<int32>(ECharacterType::MAX))
	{
		ECharacterType CurrentCharacterType = static_cast<ECharacterType>(Index);

		SelectedCharacterType = CurrentCharacterType;

		APlayerController* PC = GetOwningPlayer();
		AMainPlayerController* MainPC = Cast<AMainPlayerController>(PC);

		if (MainPC)
		{
			MainPC->ServerSetCharacterType(CurrentCharacterType);
		}
	}
}