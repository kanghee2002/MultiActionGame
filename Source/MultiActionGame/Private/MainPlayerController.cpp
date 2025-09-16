// Fill out your copyright notice in the Description page of Project Settings.
#include "MainPlayerController.h"

#include "BaseCharacter.h"
#include "PlayerHealthBar.h"        // UPlayerHealthBar 클래스 헤더
#include "HealthComponent.h"        // UHealthComponent 클래스 헤더
#include "Blueprint/UserWidget.h"   // CreateWidget 함수 사용을 위한 헤더
#include "MultiGameInstance.h"

#include "Net/UnrealNetwork.h"
#include <EnhancedInputSubsystems.h>

void AMainPlayerController::BeginPlay() {
    Super::BeginPlay();

    if (IsLocalController())
    {
        if (const UMultiGameInstance* GI = Cast<UMultiGameInstance>(GetGameInstance()))
        {
            ServerSetCharacterType(GI->SelectedCharacterType);
        }
    }
}

void AMainPlayerController::CreateHealthBar()
{
	if (HealthBarClass)
	{
		if (!HealthBarWidget)
		{
			HealthBarWidget = CreateWidget<UPlayerHealthBar>(this, HealthBarClass);
			if (HealthBarWidget)
			{
				HealthBarWidget->AddToViewport();
				UE_LOG(LogTemp, Warning, TEXT("HealthBar Created"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to create HealthBar"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HealthBar is already Created"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HealthBarClass is not valid!"));
	}

	if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(GetPawn()))
	{
		if (UHealthComponent* HealthComp = MyChar->FindComponentByClass<UHealthComponent>())
		{
			HealthBarWidget->InitializeWithHealthComponent(HealthComp);
			UE_LOG(LogTemp, Warning, TEXT("HealthBar bound to replicated Pawn: %s"), *MyChar->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to Find Health Component"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Character Cast"));
	}
}

// 서버 UI 생성
void AMainPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(LogTemp, Warning, TEXT("OnPossess: %s"), *InPawn->GetName());

	if (IsLocalPlayerController())
	{
		CreateHealthBar();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Local Controller on OnPossess"));
	}
}

// 클라 UI 생성
void AMainPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (IsLocalController())
	{
		if (HealthBarClass && !HealthBarWidget)
		{
			CreateHealthBar();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Local Controller on OnRep_Pawn"));
	}
}


void AMainPlayerController::TryBindPawn()
{
	if (!HealthBarWidget) // ✅ 위젯 생성 안됐으면 리턴
	{
		UE_LOG(LogTemp, Warning, TEXT("TryBindPawn called but HealthBarWidget is nullptr"));
		return;
	}

	if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(GetPawn()))
	{
		if (UHealthComponent* HealthComp = MyChar->FindComponentByClass<UHealthComponent>())
		{
			HealthBarWidget->InitializeWithHealthComponent(HealthComp);
			UE_LOG(LogTemp, Warning, TEXT("HealthBar bound in TryBindPawn: %s"), *MyChar->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TryBindPawn: Pawn is nullptr"));
	}
}


// Called After InputComponent is created
void AMainPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

	UE_LOG(LogTemp, Warning, TEXT("=== PlayerController SetupInputComponent ==="));
	UE_LOG(LogTemp, Warning, TEXT("Controller: %s, IsLocal: %d"), *GetName(), IsLocalPlayerController());

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        Subsystem->AddMappingContext(MappingContext, 0);

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