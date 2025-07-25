// Fill out your copyright notice in the Description page of Project Settings.
#include "MainPlayerController.h"

#include "PlayerHealthBar.h"        // UPlayerHealthBar 클래스 헤더
#include "HealthComponent.h"        // UHealthComponent 클래스 헤더
#include "Blueprint/UserWidget.h"   // CreateWidget 함수 사용을 위한 헤더

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
