// Fill out your copyright notice in the Description page of Project Settings.
#include "MainPlayerController.h"

#include "PlayerHealthBar.h"        // UPlayerHealthBar Ŭ���� ���
#include "HealthComponent.h"        // UHealthComponent Ŭ���� ���
#include "Blueprint/UserWidget.h"   // CreateWidget �Լ� ����� ���� ���

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

    // �Ʒ� �ڵ�: PlayerHealth ���� ���� (UI �ν��Ͻ� ���� & Viewport�� Add)
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
