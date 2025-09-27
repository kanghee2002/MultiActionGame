// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Character/CharacterType.h"
#include "Net/UnrealNetwork.h"
#include "MainPlayerController.generated.h"

class UInGameHUD;
class UGameOverWidget;
class UHealthComponent;

/**
 *
 */
UCLASS(Blueprintable)
class MULTIACTIONGAME_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> MappingContext;
	
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnRep_Pawn() override;

	void HideHealthBarWidget(APawn* MyPawn);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> InGameHUD;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidget;

public:
	UPROPERTY(Replicated)
	ECharacterType SelectedCharacterType;

	UFUNCTION(Server, Reliable)
	void ServerSetCharacterType(ECharacterType CharacterType);

	UFUNCTION(BlueprintCallable)
	void InitializeBossHealth(UHealthComponent* HealthComp);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void CreateInGameHUD();

	UFUNCTION(Client, Reliable)
	void Client_CreateGameOverWidget(bool IsWin);

	UPROPERTY()
	UInGameHUD* InGameHUDWidget;

	UPROPERTY()
	UGameOverWidget* GameOverWidgetRef;
};
