// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MenuSystem/MenuInterface.h"
#include "Character/CharacterType.h"
#include "MultiGameInstance.generated.h"

/**
 * 
 */

UCLASS()
class MULTIACTIONGAME_API UMultiGameInstance : public UGameInstance, public IMenuInterface
{
	GENERATED_BODY()
	
public:
	ECharacterType SelectedCharacterType;

	UMultiGameInstance(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	void LoadMenu();

	UFUNCTION(BlueprintCallable)
	void LoadInGameMenu();

	UFUNCTION(Exec)
	virtual void Host() override;

	UFUNCTION(Exec)
	virtual void Join(const FString& Address, ECharacterType CharacterType) override;

	virtual void LoadMainMenu() override;

private:
	TSubclassOf<class UUserWidget> MenuClass;

	TSubclassOf<class UUserWidget> InGameMenuClass;

	class UMainMenu* Menu;

	class UInGameMenu* InGameMenu;

};
