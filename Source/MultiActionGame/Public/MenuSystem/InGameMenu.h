// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuSystem/MenuWidget.h"
#include "InGameMenu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIACTIONGAME_API UInGameMenu : public UMenuWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;

private:
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UWidget* InGameMenu;

	UPROPERTY(meta = (BindWidget))
	class UWidget* GraphicMenu;

	UPROPERTY(meta = (BindWidget))
	class UButton* CancelButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* GraphicSettingButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EpicButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* HighButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* MediumButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* LowButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* GraphicCancelButton;

	UFUNCTION()
	void Cancel();

	UFUNCTION()
	void Quit();

	UFUNCTION()
	void OpenGraphicMenu();

	UFUNCTION()
	void OpenInGameMenu();

	UFUNCTION()
	void SetGraphicEpic();

	UFUNCTION()
	void SetGraphicHigh();

	UFUNCTION()
	void SetGraphicMedium();

	UFUNCTION()
	void SetGraphicLow();
};
