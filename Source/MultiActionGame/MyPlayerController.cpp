// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include <EnhancedInputSubsystems.h>
#include <EnhancedInputComponent.h>
#include <GameFramework/Character.h>
#include <Camera/CameraComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <GameFramework/CharacterMovementComponent.h>
#include "Public/InputActionGroup.h"

void AMyPlayerController::BeginPlay() {
    GetCharacter()->GetCharacterMovement()->MaxWalkSpeed = fMaxSpeedWalking;
}

void AMyPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	subsystem->ClearAllMappings();
	subsystem->AddMappingContext(MappingContext, 0);

    if (!InputComponent) {
        UE_LOG(LogTemp, Warning, TEXT("InputComponent is nullptr"));
        return;
    }

	UEnhancedInputComponent* component = Cast<UEnhancedInputComponent>(InputComponent);

    if (!component) {
        UE_LOG(LogTemp, Warning, TEXT("casting failed"));
        return;
    }

	component->BindAction(InputActionGroup->MoveAction, ETriggerEvent::Triggered, this, &AMyPlayerController::Move);
	component->BindAction(InputActionGroup->LookAction, ETriggerEvent::Triggered, this, &AMyPlayerController::Look);
    component->BindAction(InputActionGroup->SprintAction, ETriggerEvent::Started, this, &AMyPlayerController::StartSprint);
    component->BindAction(InputActionGroup->SprintAction, ETriggerEvent::Completed, this, &AMyPlayerController::StopSprint);
    component->BindAction(InputActionGroup->JumpAction, ETriggerEvent::Started, this, &AMyPlayerController::StartJump);
    component->BindAction(InputActionGroup->JumpAction, ETriggerEvent::Completed, this, &AMyPlayerController::StopJump);
}

void AMyPlayerController::Move(const FInputActionValue& value) {
    UE_LOG(LogTemp, Warning, TEXT("jumping: %d"), bIsJumping);
    if (bIsJumping) return;

    FVector2D movementVector = value.Get<FVector2D>();

    // 실제 카메라 회전 가져오기
    UCameraComponent* camera = GetCharacter()->FindComponentByClass<UCameraComponent>();
    FRotator cameraRotation = camera->GetComponentRotation();

    // 카메라 기준 방향 계산
    const FRotator yawRotation(0, cameraRotation.Yaw, 0);
    const FVector forwardDirection = FRotationMatrix(yawRotation).GetScaledAxis(EAxis::X);
    const FVector rightDirection = FRotationMatrix(yawRotation).GetScaledAxis(EAxis::Y);

    // 카메라 기준으로 이동 입력 적용
    GetCharacter()->AddMovementInput(forwardDirection, movementVector.X);
    GetCharacter()->AddMovementInput(rightDirection, movementVector.Y);

    // 이동 입력이 있을 때만 캐릭터 회전
    if (!movementVector.IsZero()) {
        // 실제 이동 방향 계산 (카메라 기준)
        FVector movementDirection = (forwardDirection * movementVector.X) + (rightDirection * movementVector.Y);
        movementDirection.Normalize();

        // 캐릭터가 이동 방향을 바라보도록 회전
        if (!movementDirection.IsZero()) {
            FRotator targetRotation = movementDirection.Rotation();
            FRotator currentRotation = GetCharacter()->GetActorRotation();

            FRotator newRotation = FMath::RInterpTo(currentRotation, targetRotation, GetWorld()->GetDeltaSeconds(), 8.0f);
            GetCharacter()->SetActorRotation(newRotation);
        }
    }
}

void AMyPlayerController::Look(const FInputActionValue& value) {
    FVector2D lookAxisVector = value.Get<FVector2D>();

    USpringArmComponent* springArm = GetCharacter()->FindComponentByClass<USpringArmComponent>();

    FRotator currentRotation = springArm->GetComponentRotation();
    FRotator newRotation = currentRotation;

    // 마우스 입력으로 회전 계산
    newRotation.Yaw += lookAxisVector.X;
    newRotation.Pitch += lookAxisVector.Y;

    // Pitch 제한 (위아래 각도 제한)
    newRotation.Pitch = FMath::Clamp(newRotation.Pitch, -70.0f, 70.0f);

    springArm->SetWorldRotation(newRotation);
}

void AMyPlayerController::StartSprint() {
    bIsSprinting = true;

    GetCharacter()->GetCharacterMovement()->MaxWalkSpeed = fMaxSpeedSprinting;
}

void AMyPlayerController::StopSprint() {
    bIsSprinting = false;

    GetCharacter()->GetCharacterMovement()->MaxWalkSpeed = fMaxSpeedWalking;
}

void AMyPlayerController::StartJump() {
    bIsJumping = true;

    GetCharacter()->Jump();
}

void AMyPlayerController::StopJump() {
    GetCharacter()->StopJumping();
}

void AMyPlayerController::OnCharacterLanded() {
    bIsJumping = false;
}
