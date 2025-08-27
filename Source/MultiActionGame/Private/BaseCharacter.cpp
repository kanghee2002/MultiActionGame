// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include <Camera/CameraComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <GameFramework/CharacterMovementComponent.h>
#include "InputActionGroup.h"
#include "MainPlayerController.h"
#include "PlayerHealthBar.h"
#include "BaseAnimInstance.h"
#include <EnhancedInputComponent.h>

// Sets default values
ABaseCharacter::ABaseCharacter() {
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    static ConstructorHelpers::FObjectFinder<UInputActionGroup> ActionGroupAsset(TEXT("/Game/Input/InputActionGroup.InputActionGroup"));

    if (ActionGroupAsset.Succeeded()) InputActionGroup = ActionGroupAsset.Object;
    else UE_LOG(LogAssetData, Fatal, TEXT("InputActionGroup cannot be found!"));

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 400.f;
    SpringArm->SocketOffset = FVector(0., 100., 80.);
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bEnableCameraLag = true;
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritRoll = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    Camera->bUsePawnControlRotation = false;

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

	bReplicates = true;
	bAlwaysRelevant = true;

	HealthCompRef = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay() {
	Super::BeginPlay();

    GetCharacterMovement()->MaxWalkSpeed = GetMaxWalkSpeed();

	UE_LOG(LogTemp, Warning, TEXT("BeginPlay Character: %s"), *this->GetName());
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

    // 로컬이 아닌 경우에만 보간
    if (!IsLocallyControlled())
    {
        FRotator currentRotation = GetActorRotation();

        float angleDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(currentRotation.Yaw, ReplicatedRotation.Yaw));
        
        if (angleDifference > 1.0f)
        {
            FRotator newRotation = FMath::RInterpTo(currentRotation, ReplicatedRotation, DeltaTime, 10.0f);
            SetActorRotation(newRotation);
        }
    }
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* component = Cast<UEnhancedInputComponent>(PlayerInputComponent);

    if (!component) {
        UE_LOG(LogTemp, Warning, TEXT("casting failed"));
        return;
    }

    component->BindAction(InputActionGroup->MoveAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Move);
    component->BindAction(InputActionGroup->LookAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Look);
    component->BindAction(InputActionGroup->SprintAction, ETriggerEvent::Started, this, &ABaseCharacter::StartSprint);
    component->BindAction(InputActionGroup->SprintAction, ETriggerEvent::Completed, this, &ABaseCharacter::StopSprint);
    component->BindAction(InputActionGroup->JumpAction, ETriggerEvent::Started, this, &ABaseCharacter::Jump);
    component->BindAction(InputActionGroup->JumpAction, ETriggerEvent::Completed, this, &ABaseCharacter::StopJumping);
}

void ABaseCharacter::Move(const FInputActionValue& value) {
    if (bIsJumping) return;

    FVector2D movementVector = value.Get<FVector2D>();

    // 실제 카메라 회전 가져오기
    FRotator cameraRotation = Camera->GetComponentRotation();

    // 카메라 기준 방향 계산
    const FRotator yawRotation(0, cameraRotation.Yaw, 0);
    const FVector forwardDirection = FRotationMatrix(yawRotation).GetScaledAxis(EAxis::X);
    const FVector rightDirection = FRotationMatrix(yawRotation).GetScaledAxis(EAxis::Y);

    // 카메라 기준으로 이동 입력 적용
    AddMovementInput(forwardDirection, movementVector.X);
    AddMovementInput(rightDirection, movementVector.Y);

    // 이동 입력이 있을 때만 캐릭터 회전
    if (!movementVector.IsZero()) {
        // 실제 이동 방향 계산 (카메라 기준)
        FVector movementDirection = (forwardDirection * movementVector.X) + (rightDirection * movementVector.Y);
        movementDirection.Normalize();

        // 캐릭터가 이동 방향을 바라보도록 회전
        if (!movementDirection.IsZero()) {
            FRotator targetRotation = movementDirection.Rotation();

            // 로컬일 때만 자체적으로 보간, 서버에 바라보는 방향 알림
            if (IsLocallyControlled())
            {
                FRotator currentRotation = GetActorRotation();
                FRotator newRotation = FMath::RInterpTo(currentRotation, targetRotation, GetWorld()->GetDeltaSeconds(), 8.0f);
                SetActorRotation(newRotation);
                ServerSetRotation(targetRotation);
            }
        }
    }
}
    
void ABaseCharacter::Look(const FInputActionValue& value) {
    FVector2D lookAxisVector = value.Get<FVector2D>();

    FRotator currentRotation = SpringArm->GetComponentRotation();
    FRotator newRotation = currentRotation;

    // 마우스 입력으로 회전 계산
    newRotation.Yaw += lookAxisVector.X;
    newRotation.Pitch += lookAxisVector.Y;

    // Pitch 제한 (위아래 각도 제한)
    newRotation.Pitch = FMath::Clamp(newRotation.Pitch, -70.0f, 70.0f);

    SpringArm->SetWorldRotation(newRotation);
}

void ABaseCharacter::StartSprint() {
    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = GetMaxSprintSpeed();

    if (!HasAuthority())
    {
        ServerStartSprint();
    }
}

void ABaseCharacter::StopSprint() {
    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = GetMaxWalkSpeed();

    if (!HasAuthority())
    {
        ServerStopSprint();
    }
}

void ABaseCharacter::ServerSetRotation_Implementation(FRotator NewRotation)
{
    ReplicatedRotation = NewRotation;
}

void ABaseCharacter::ServerStartSprint_Implementation()
{
    StartSprint();
}

void ABaseCharacter::ServerStopSprint_Implementation()
{
    StopSprint();
}

void ABaseCharacter::Jump() {
    Super::Jump();

    bIsJumping = true;

    UBaseAnimInstance* inst = Cast<UBaseAnimInstance>(GetMesh()->GetAnimInstance());
    if (inst) {
        inst->JumpStart();
    }
}

void ABaseCharacter::StopJumping() {
    Super::StopJumping();
}

void ABaseCharacter::Landed(const FHitResult& hit) {
    Super::Landed(hit);

    bIsJumping = false;
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	UE_LOG(LogTemp, Warning, TEXT("Setting up replication for Character"));
    DOREPLIFETIME(ABaseCharacter, bIsSprinting);
    DOREPLIFETIME(ABaseCharacter, bIsJumping);
    DOREPLIFETIME(ABaseCharacter, ReplicatedRotation);
}