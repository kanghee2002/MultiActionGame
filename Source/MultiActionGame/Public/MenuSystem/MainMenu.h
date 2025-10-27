// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuWidget.h"
#include "Components/CheckBox.h"
#include "RadioCheckBox.h"
#include "Character/CharacterType.h"
#include "MainMenu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIACTIONGAME_API UMainMenu : public UMenuWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;

private:
	ECharacterType SelectedCharacterType;

	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* JoinButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* GraphicSettingButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* CancelJoinButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ConfirmJoinButton;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UWidget* MainMenu;

	UPROPERTY(meta = (BindWidget))
	class UWidget* JoinMenu;

	UPROPERTY(meta = (BindWidget))
	class UWidget* GraphicMenu;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* IPAddressField;

	UPROPERTY(meta = (BindWidget))
	class URadioCheckBox* KnightCheckBox;

	UPROPERTY(meta = (BindWidget))
	class URadioCheckBox* ArcherCheckBox;

	UPROPERTY(meta = (BindWidget))
	class URadioCheckBox* HealerCheckBox;

	UPROPERTY()
	TArray<URadioCheckBox*> CharacterCheckBoxes;
	
	UPROPERTY(meta = (BindWidget))
	class UButton* EpicButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* HighButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* MediumButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* LowButton;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* BossHealthInput;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* BossAttackDamageInput;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* BossAttackCostInput;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* BossSkillCooldownInput;


	UPROPERTY(meta = (BindWidget))
	class UButton* SettingConfirmButton;

	UFUNCTION()
	void HostServer();

	UFUNCTION()
	void JoinServer();

	UFUNCTION()
	void OpenJoinMenu();

	UFUNCTION()
	void OpenMainMenu();

	UFUNCTION()
	void UncheckAll(bool bIsChecked);

	UFUNCTION()
	void OpenGraphicMenu();

	UFUNCTION()
	void SetGraphicEpic();

	UFUNCTION()
	void SetGraphicHigh();

	UFUNCTION()
	void SetGraphicMedium();

	UFUNCTION()
	void SetGraphicLow();

	UFUNCTION()
	void ConfirmSetting();

	UFUNCTION()
	void Quit();

	UFUNCTION()
	void SelectCharacter(int32 Index);
};
