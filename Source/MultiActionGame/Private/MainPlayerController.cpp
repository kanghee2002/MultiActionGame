// Fill out your copyright notice in the Description page of Project Settings.
#include "MainPlayerController.h"

#include "BaseCharacter.h"
#include "PlayerHealthBar.h"        // UPlayerHealthBar 클래스 헤더
#include "HealthComponent.h"        // UHealthComponent 클래스 헤더
#include "Blueprint/UserWidget.h"   // CreateWidget 함수 사용을 위한 헤더

#include <EnhancedInputSubsystems.h>
#include "MultiGameInstance.h"
#include "Net/UnrealNetwork.h"

void AMainPlayerController::BeginPlay() {
    Super::BeginPlay();

    /*UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

    if (ensure(subsystem != nullptr))
    {
        subsystem->ClearAllMappings();
        subsystem->AddMappingContext(MappingContext, 0);
    }


    if (!InputComponent) {
        UE_LOG(LogTemp, Warning, TEXT("InputComponent is nullptr"));
        return;
    }*/

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
				UE_LOG(LogTemp, Warning, TEXT("HealthBar Created in OnPossess"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to create HealthBarWidget in BeginPlay"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HealthBarWidget is already Created"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HealthBarClass is not valid! Check packaging settings."));
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

void AMainPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (IsLocalController())
	{
		if (HealthBarClass && !HealthBarWidget)
		{
			CreateHealthBar();
		}
		UE_LOG(LogTemp, Warning, TEXT("No HealthBarClass"));
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

    UE_LOG(LogTemp, Warning, TEXT("SetupInputComponent() Called"));

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