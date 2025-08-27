// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuSystem/MainMenu.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"
#include "MenuSystem/MenuInterface.h"
#include "MainPlayerController.h"


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

	// Initialize Character Select CheckBoxes
	CharacterCheckBoxes.Empty();

	CharacterCheckBoxes.Add(KnightCheckBox);
	CharacterCheckBoxes.Add(ArcherCheckBox);
	CharacterCheckBoxes.Add(HealerCheckBox);
	
	for (URadioCheckBox* CheckBox : CharacterCheckBoxes)
	{
		if (CheckBox)
		{
			//CheckBox->OnCheckStateChanged.Clear();
			//CheckBox->OnCheckStateChanged.AddDynamic(this, &UMainMenu::UncheckAll);
			CheckBox->Initialize(CharacterCheckBoxes);
			CheckBox->OnSelected.AddDynamic(this, &UMainMenu::SelectCharacter);
		}
	}

	//KnightCheckBox->OnCheckStateChanged.AddDynamic(this, &UMainMenu::SelectKnight);
	//ArcherCheckBox->OnCheckStateChanged.AddDynamic(this, &UMainMenu::SelectArcher);
	//HealerCheckBox->OnCheckStateChanged.AddDynamic(this, &UMainMenu::SelectHealer);

	KnightCheckBox->SetCheckedState(ECheckBoxState::Checked);
	SelectedCharacterType = ECharacterType::Knight;

	return true;
}

void UMainMenu::HostServer()
{
	if (MenuInterface != nullptr)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Host"));
		MenuInterface->Host();
	}
}

void UMainMenu::JoinServer()
{
	if (MenuInterface != nullptr)
	{
		if (!ensure(IPAddressField != nullptr)) return;

		const FString& Address = IPAddressField->GetText().ToString();
		//UE_LOG(LogTemp, Warning, TEXT("Join with %s, Index %d"), *Address, static_cast<int32>(SelectedCharacterType));
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

void UMainMenu::SelectKnight(bool bIsChecked)
{
	UE_LOG(LogTemp, Warning, TEXT("Select Knight"));
	//SelectedCharacterType = ECharacterType::Knight;
	KnightCheckBox->SetCheckedState(ECheckBoxState::Checked);
}

void UMainMenu::SelectArcher(bool bIsChecked)
{
	UE_LOG(LogTemp, Warning, TEXT("Select Archer"));
	//SelectedCharacterType = ECharacterType::Archer;
	//ArcherCheckBox->SetCheckedState(ECheckBoxState::Checked);
}

void UMainMenu::SelectHealer(bool bIsChecked)
{
	UE_LOG(LogTemp, Warning, TEXT("Select Healer"));
	//SelectedCharacterType = ECharacterType::Healer;
	HealerCheckBox->SetCheckedState(ECheckBoxState::Checked);
}

void UMainMenu::UncheckAll(bool bIsChecked)
{
	for (UCheckBox* CheckBox : CharacterCheckBoxes)
	{
		CheckBox->SetCheckedState(ECheckBoxState::Unchecked);
	}
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

			//UE_LOG(LogTemp, Warning, TEXT("Set Character Type (Server): %d"), Index);
		}
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Invalid CharacterType: %d"), Index);
	}
}