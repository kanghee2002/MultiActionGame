// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Character/CharacterType.h"
#include "Net/UnrealNetwork.h"
#include "MainPlayerController.generated.h"

class UInGameHUD;
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

	void TryBindPawn();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> InGameHUD;

public:
	UPROPERTY(Replicated)
	ECharacterType SelectedCharacterType;

	UFUNCTION(Server, Reliable)
	void ServerSetCharacterType(ECharacterType CharacterType);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void CreateInGameHUD();

	UPROPERTY()
	UInGameHUD* InGameHUDWidget;
};
