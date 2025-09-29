// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuSystem/GameOverWidget.h"

#include "Components/Button.h"

bool UGameOverWidget::Initialize()
{
	bool Success = Super::Initialize();

	if (!Success)
	{
		return false;
	}

	if (!ensure(QuitButton != nullptr)) return false;
	QuitButton->OnClicked.AddDynamic(this, &UGameOverWidget::Quit);

	return true;
}

void UGameOverWidget::InitializeResult(bool IsWin)
{
	if (IsWin)
	{
		ResultText->SetText(FText::FromString(TEXT("ENEMY FELLED")));
		ResultText->SetColorAndOpacity(WinTextColor);
	}
	else
	{
		ResultText->SetText(FText::FromString(TEXT("YOU DIED")));
		ResultText->SetColorAndOpacity(LoseTextColor);
	}
}


void UGameOverWidget::Quit()
{
	if (MenuInterface != nullptr)
	{
		Teardown();

		MenuInterface->LoadMainMenu();
	}
}