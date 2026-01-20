// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MenuSystem/MenuInterface.h"
#include "Character/CharacterType.h"
#include "MultiGameInstance.generated.h"

UENUM(BlueprintType)
enum class EGraphicSetting : uint8
{
	Low = 0			UMETA(DisplayName = "Low"),
	Medium = 1		UMETA(DisplayName = "Medium"),
	High = 2		UMETA(DisplayName = "High"),
	Epic = 3		UMETA(DisplayName = "Epic"),
	MAX = 4			UMETA(Hidden)
};

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
	virtual void Host(ECharacterType CharacterType) override;

	UFUNCTION(Exec)
	virtual void Join(const FString& Address, ECharacterType CharacterType) override;

	virtual void LoadMainMenu() override;

	void SetGraphicSetting(EGraphicSetting GraphicSetting);

	void SetIsBossAI(bool value)
	{
		IsBossAI = value;
	}

	bool GetIsBossAI() const
	{
		return IsBossAI;
	}

	UPROPERTY(BlueprintReadOnly)
	float BossHealth;

	UPROPERTY(BlueprintReadOnly)
	float BossAttackDamage;

	UPROPERTY(BlueprintReadOnly)
	float BossAttackCost;

	UPROPERTY(BlueprintReadOnly)
	float BossSkillCooldown;

private:
	TSubclassOf<class UUserWidget> MenuClass;

	TSubclassOf<class UUserWidget> InGameMenuClass;

	class UMainMenu* Menu;

	class UInGameMenu* InGameMenu;

	bool IsBossAI;
};
