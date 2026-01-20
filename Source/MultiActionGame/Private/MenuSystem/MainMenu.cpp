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
	HostButton->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);

	if (!ensure(JoinButton != nullptr)) return false;
	JoinButton->OnClicked.Clear();
	JoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

	if (!ensure(QuitButton != nullptr)) return false;
	QuitButton->OnClicked.AddDynamic(this, &UMainMenu::Quit);

	if (!ensure(PvPButton != nullptr)) return false;
	PvPButton->OnClicked.Clear();
	PvPButton->OnClicked.AddDynamic(this, &UMainMenu::HostPvPServer);

	if (!ensure(PvEButton != nullptr)) return false;
	PvEButton->OnClicked.Clear();
	PvEButton->OnClicked.AddDynamic(this, &UMainMenu::HostPvEServer);

	if (!ensure(ConfirmHostButton != nullptr)) return false;
	ConfirmHostButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

	if (!ensure(CancelHostButton != nullptr)) return false;
	CancelHostButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

	if (!ensure(CancelSelectCharacterButton != nullptr)) return false;
	CancelSelectCharacterButton->OnClicked.AddDynamic(this, &UMainMenu::OpenSelectModeMenu);

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

	// Initialize Host Character Select CheckBoxes
	HostCharacterCheckBoxes.Empty();

	HostCharacterCheckBoxes.Add(HostBossCheckBox);
	HostCharacterCheckBoxes.Add(HostKnightCheckBox);
	HostCharacterCheckBoxes.Add(HostArcherCheckBox);
	HostCharacterCheckBoxes.Add(HostWizardCheckBox);

	for (URadioCheckBox* CheckBox : HostCharacterCheckBoxes)
	{
		if (CheckBox)
		{
			CheckBox->Initialize(HostCharacterCheckBoxes);
			CheckBox->OnSelected.AddDynamic(this, &UMainMenu::SelectCharacter);
		}
	}

	if (HostKnightCheckBox)
	{
		HostKnightCheckBox->SetCheckedState(ECheckBoxState::Checked);
		SelectedCharacterType = ECharacterType::Knight;
	}

	// Initialize Guest Character Select CheckBoxes
	GuestCharacterCheckBoxes.Empty();

	GuestCharacterCheckBoxes.Add(GuestBossCheckBox);
	GuestCharacterCheckBoxes.Add(GuestKnightCheckBox);
	GuestCharacterCheckBoxes.Add(GuestArcherCheckBox);
	GuestCharacterCheckBoxes.Add(GuestWizardCheckBox);
	
	for (URadioCheckBox* CheckBox : GuestCharacterCheckBoxes)
	{
		if (CheckBox)
		{
			CheckBox->Initialize(GuestCharacterCheckBoxes);
			CheckBox->OnSelected.AddDynamic(this, &UMainMenu::SelectCharacter);
		}
	}

	if (GuestKnightCheckBox)
	{
		GuestKnightCheckBox->SetCheckedState(ECheckBoxState::Checked);
		SelectedCharacterType = ECharacterType::Knight;
	}

	return true;
}

void UMainMenu::HostPvPServer()
{
	SelectedModeType = EModeType::PvP;

	OffsetSizeBox->SetVisibility(ESlateVisibility::Visible);
	SelectBossBox->SetVisibility(ESlateVisibility::Visible);
	HostBossCheckBox->SetVisibility(ESlateVisibility::Visible);

	for (URadioCheckBox* CheckBox : HostCharacterCheckBoxes)
	{
		if (CheckBox)
		{
			CheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		}
	}

	if (HostBossCheckBox)
	{
		HostBossCheckBox->SetCheckedState(ECheckBoxState::Checked);
		SelectedCharacterType = ECharacterType::Boss;
	}

	HostMenuSwitcher->SetActiveWidget(SelectCharacterMenu);
}

void UMainMenu::HostPvEServer()
{
	SelectedModeType = EModeType::PvE;

	OffsetSizeBox->SetVisibility(ESlateVisibility::Collapsed);
	SelectBossBox->SetVisibility(ESlateVisibility::Collapsed);
	HostBossCheckBox->SetVisibility(ESlateVisibility::Collapsed);

	for (URadioCheckBox* CheckBox : HostCharacterCheckBoxes)
	{
		if (CheckBox)
		{
			CheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		}
	}

	if (HostKnightCheckBox)
	{
		HostKnightCheckBox->SetCheckedState(ECheckBoxState::Checked);
		SelectedCharacterType = ECharacterType::Knight;
	}

	HostMenuSwitcher->SetActiveWidget(SelectCharacterMenu);
}

void UMainMenu::HostServer()
{
	UMultiGameInstance* multiGameInstace = Cast<UMultiGameInstance>(GetGameInstance());

	if (multiGameInstace != nullptr)
	{
		if (SelectedModeType == EModeType::PvE)
		{
			multiGameInstace->SetIsBossAI(true);
		}
		else
		{
			multiGameInstace->SetIsBossAI(false);
		}
	}

	if (MenuInterface != nullptr)
	{
		MenuInterface->Host(SelectedCharacterType);
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

void UMainMenu::OpenHostMenu()
{
	if (!ensure(MenuSwitcher != nullptr)) return;
	if (!ensure(HostMenu != nullptr)) return;

	MenuSwitcher->SetActiveWidget(HostMenu);
}

void UMainMenu::OpenJoinMenu()
{
	for (URadioCheckBox* CheckBox : GuestCharacterCheckBoxes)
	{
		if (CheckBox)
		{
			CheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		}
	}

	if (GuestKnightCheckBox)
	{
		GuestKnightCheckBox->SetCheckedState(ECheckBoxState::Checked);
		SelectedCharacterType = ECharacterType::Knight;
	}

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

void UMainMenu::OpenSelectModeMenu()
{
	if (!ensure(HostMenuSwitcher != nullptr)) return;
	if (!ensure(SelectModeMenu != nullptr)) return;

	HostMenuSwitcher->SetActiveWidget(SelectModeMenu);
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
	/*for (UCheckBox* CheckBox : CharacterCheckBoxes)
	{
		CheckBox->SetCheckedState(ECheckBoxState::Unchecked);
	}*/
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