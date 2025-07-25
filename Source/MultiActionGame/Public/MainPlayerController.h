// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

class UPlayerHealthBar;
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

protected:
	// WBP_HealthBar 할당할 BP 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HealthBarClass; 

private:
	// 생성된 위젯 인스턴스 
	UPROPERTY()
	UPlayerHealthBar* HealthBarWidget;

	// 캐릭터의 HealthComponent 참조 
	UPROPERTY()
	UHealthComponent* HealthCompRef;
};
