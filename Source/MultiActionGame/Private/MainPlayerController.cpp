// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
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