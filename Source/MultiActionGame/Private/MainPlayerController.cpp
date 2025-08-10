// Fill out your copyright notice in the Description page of Project Settings.
#include "MainPlayerController.h"

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

    // 아래 코드: PlayerHealth 관련 로직 (UI 인스턴스 생성 & Viewport에 Add)
    if (APawn* P = GetPawn())
    {
        HealthCompRef = P->FindComponentByClass<UHealthComponent>();
    }

    if (HealthBarClass && HealthCompRef)
    {
        HealthBarWidget = CreateWidget<UPlayerHealthBar>(GetWorld(), HealthBarClass);
        if (HealthBarWidget)
        {
            HealthBarWidget->AddToViewport();
            HealthBarWidget->InitializeWithHealthComponent(HealthCompRef);
        }
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