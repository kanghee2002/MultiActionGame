// Fill out your copyright notice in the Description page of Project Settings.
#include "MainPlayerController.h"

#include "Character/BaseCharacter.h"
#include "InGameHUD.h"        // UInGameHUD 클래스 헤더
#include "Character/HealthComponent.h"        // UHealthComponent 클래스 헤더
#include "Blueprint/UserWidget.h"   // CreateWidget 함수 사용을 위한 헤더
#include "Components/WidgetComponent.h"
#include "Character/HealthBarWidget.h"
#include "MultiGameInstance.h"

#include "Net/UnrealNetwork.h"
#include <EnhancedInputSubsystems.h>

void AMainPlayerController::BeginPlay() {
    Super::BeginPlay();

    if (IsLocalController())
    {
        if (const UMultiGameInstance* gameInstance = Cast<UMultiGameInstance>(GetGameInstance()))
        {
            ServerSetCharacterType(gameInstance->SelectedCharacterType);
        }
    }
}

// 서버 UI 생성
void AMainPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(LogTemp, Warning, TEXT("OnPossess: %s"), *InPawn->GetName());

	if (IsLocalPlayerController())
	{
		CreateInGameHUD();

		HideHealthBarWidget(InPawn);
	}
}

// 클라 UI 생성
void AMainPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	
	if (IsLocalController())
	{
		if (InGameHUD && !InGameHUDWidget)
		{
			CreateInGameHUD();
		}

		HideHealthBarWidget(GetPawn());
	}
}

void AMainPlayerController::CreateInGameHUD()
{
	// Create InGameHUD
	if (InGameHUD)
	{
		if (!InGameHUDWidget)
		{
			InGameHUDWidget = CreateWidget<UInGameHUD>(this, InGameHUD);
			if (InGameHUDWidget)
			{
				InGameHUDWidget->AddToViewport();
				UE_LOG(LogTemp, Warning, TEXT("InGameHUD Created"));
			}
		}
	}

	// Initialize
	if (ABaseCharacter* myChar = Cast<ABaseCharacter>(GetPawn()))
	{
		if (UHealthComponent* healthComp = myChar->FindComponentByClass<UHealthComponent>())
		{
			InGameHUDWidget->InitializeHealthComponent(healthComp);
			UE_LOG(LogTemp, Warning, TEXT("HealthBar bound to replicated Pawn: %s"), *myChar->GetName());

		}

		if (UStaminaComponent* staminaComp = myChar->FindComponentByClass<UStaminaComponent>())
		{
			InGameHUDWidget->InitializeStaminaComponent(staminaComp);
			UE_LOG(LogTemp, Warning, TEXT("StaminaBar bound to replicated Pawn: %s"), *myChar->GetName());
		}

		InGameHUDWidget->InitializeHealCountText(myChar);
	}

	// Set Boss UI
	if (SelectedCharacterType == ECharacterType::Knight)
	{
		InGameHUDWidget->SetBossUI();
	}
}

void AMainPlayerController::HideHealthBarWidget(APawn* MyPawn)
{
	ABaseCharacter* myChar = Cast<ABaseCharacter>(MyPawn);

	if (UWidgetComponent* widgetComponent = myChar->FindComponentByClass<UWidgetComponent>())
	{
		widgetComponent->SetVisibility(false);

		UE_LOG(LogTemp, Warning, TEXT("Hide WidgetComp"));
	}
}

// Called After InputComponent is created
void AMainPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

	UE_LOG(LogTemp, Warning, TEXT("=== PlayerController SetupInputComponent ==="));
	UE_LOG(LogTemp, Warning, TEXT("Controller: %s, IsLocal: %d"), *GetName(), IsLocalPlayerController());

    if (UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        subsystem->ClearAllMappings();
        subsystem->AddMappingContext(MappingContext, 0);

        UE_LOG(LogTemp, Warning, TEXT("InputComponent is Set"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EnhancedInput Subsystem is null"));
    }

    if (!InputComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("InputComponent is still nullptr"));
    }
}

void AMainPlayerController::ServerSetCharacterType_Implementation(ECharacterType CharacterType)
{
    SelectedCharacterType = CharacterType;
}

void AMainPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMainPlayerController, SelectedCharacterType);
}