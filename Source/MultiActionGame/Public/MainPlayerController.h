// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CharacterType.h"
#include "Net/UnrealNetwork.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MULTIACTIONGAME_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> MappingContext;

public:
	UPROPERTY(Replicated)
	ECharacterType SelectedCharacterType;

	UFUNCTION(Server, Reliable)
	void ServerSetCharacterType(ECharacterType CharacterType);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
