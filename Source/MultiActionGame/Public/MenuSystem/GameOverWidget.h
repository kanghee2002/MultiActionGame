// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuSystem/MenuWidget.h"
#include "Components/TextBlock.h"
#include "GameOverWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTIACTIONGAME_API UGameOverWidget : public UMenuWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;

	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ResultText;

	UFUNCTION()
	void Quit();

public:
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FadeIn;

	void InitializeResult(bool IsWin);

	const FSlateColor  WinTextColor = FLinearColor(0.8f, 0.58f, 0.04f, 1.0f);

	const FSlateColor  LoseTextColor = FLinearColor(0.47f, 0.0f, 0.007f, 1.0f);
};
