// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include <EnhancedInputSubsystems.h>

void AMainPlayerController::BeginPlay() {
    Super::BeginPlay();

    UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
    subsystem->ClearAllMappings();
    subsystem->AddMappingContext(MappingContext, 0);

    if (!InputComponent) {
        UE_LOG(LogTemp, Warning, TEXT("InputComponent is nullptr"));
        return;
    }
}
