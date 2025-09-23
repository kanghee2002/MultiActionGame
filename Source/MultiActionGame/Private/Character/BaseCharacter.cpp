// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacter.h"
#include "InputActionGroup.h"
#include "MainPlayerController.h"
#include "InGameHUD.h"

#include "BaseAnimInstance.h"

#include <Camera/CameraComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <GameFramework/CharacterMovementComponent.h>
#include <EnhancedInputComponent.h>

ABaseCharacter::ABaseCharacter() 
{
	PrimaryActorTick.bCanEverTick = true;

    static ConstructorHelpers::FObjectFinder<UInputActionGroup> ActionGroupAsset(TEXT("/Game/Input/InputActionGroup.InputActionGroup"));

    if (ActionGroupAsset.Succeeded()) InputActionGroup = ActionGroupAsset.Object;
    else UE_LOG(LogAssetData, Fatal, TEXT("InputActionGroup cannot be found!"));

	// 카메라 설정
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);

	// 카메라 위치
    SpringArm->TargetArmLength = 500.0f;
    SpringArm->SocketOffset = FVector(0.0f, 0.0f, 100.0f);

	// 마우스로 회전, 부드러운 움직임
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bEnableCameraLag = true;
	
	// 캐릭터 회전해도 카메라 영향 X
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritRoll = false;

	// 충돌 X -> 카메라 당겨짐 X
	SpringArm->bDoCollisionTest = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    Camera->bUsePawnControlRotation = false;

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

	// 멀티 설정
	bReplicates = true;
	bAlwaysRelevant = true;

	HealthCompRef = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	StaminaCompRef = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaCompRef"));

	OnTakeAnyDamage.AddDynamic(this, &ABaseCharacter::OnDamageReceived);

	// 변수 설정
	BasicAttackDamage = 10.0f;
	AttackStaminaCost = 15.0f;
	RollStaminaCost = 20.0f;
	CurrentHealCount = 5;
	SprintSpeed = 800.0f;
	WalkSpeed = 400.0f;
}

void ABaseCharacter::BeginPlay() 
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CurrentSpeed = WalkSpeed;
		GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;

		bCanPlayerControl = true;
	}

	HealthCompRef->OnDeath.AddDynamic(this, &ABaseCharacter::OnDeath);


	UE_LOG(LogTemp, Warning, TEXT("BeginPlay Character: %s"), *this->GetName());
}

void ABaseCharacter::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);

	SynchronizeRotation(DeltaTime);
}

void ABaseCharacter::SynchronizeRotation(float DeltaTime)
{
	FRotator currentRotation = GetActorRotation();

	float angleDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(currentRotation.Yaw, ReplicatedRotation.Yaw));

	float rotationSpeed = 8.0f;

	if (angleDifference > 1.0f)
	{
		FRotator newRotation;
		if (IsLocallyControlled())
		{
			// 로컬일 경우 지정된 방향으로 회전
			newRotation = FMath::RInterpTo(currentRotation, TargetRotation, DeltaTime, rotationSpeed);
		}
		else
		{
			// 로컬이 아닐 경우 복제된 방향으로 회전
			newRotation = FMath::RInterpTo(currentRotation, ReplicatedRotation, DeltaTime, rotationSpeed);
		}
		SetActorRotation(newRotation);
	}
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) 
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* component = Cast<UEnhancedInputComponent>(PlayerInputComponent);

    if (!component) {
        UE_LOG(LogTemp, Warning, TEXT("casting failed"));
        return;
    }

    component->BindAction(InputActionGroup->MoveAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Move);
    component->BindAction(InputActionGroup->LookAction, ETriggerEvent::Triggered, this, &ABaseCharacter::Look);
    component->BindAction(InputActionGroup->SprintAction, ETriggerEvent::Triggered, this, &ABaseCharacter::StartSprint);
    component->BindAction(InputActionGroup->SprintAction, ETriggerEvent::Completed, this, &ABaseCharacter::StopSprint);
    component->BindAction(InputActionGroup->JumpAction, ETriggerEvent::Started, this, &ABaseCharacter::Roll);
	component->BindAction(InputActionGroup->LightAttackAction, ETriggerEvent::Started, this, &ABaseCharacter::LightAttack);
	component->BindAction(InputActionGroup->HeavyAttackAction, ETriggerEvent::Started, this, &ABaseCharacter::HeavyAttack);
	component->BindAction(InputActionGroup->SelfHealAction, ETriggerEvent::Started, this, &ABaseCharacter::SelfHeal);
}

// Movement
void ABaseCharacter::Move(const FInputActionValue& value) 
{
	if (!bCanPlayerControl)
	{
		return;
	}

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

            // 로컬은 바라보는 방향 지정, 서버에 바라보는 방향 알림
            if (IsLocallyControlled())
            {
				TargetRotation = targetRotation;
                Server_SetRotation(targetRotation);
            }
        }
    }
}
    
void ABaseCharacter::Look(const FInputActionValue& value) 
{
    FVector2D lookAxisVector = value.Get<FVector2D>();

    FRotator currentRotation = SpringArm->GetComponentRotation();
    FRotator newRotation = currentRotation;

    // 마우스 입력으로 회전 계산
    newRotation.Yaw += lookAxisVector.X;
    newRotation.Pitch -= lookAxisVector.Y;

    // Pitch 제한 (위아래 각도 제한)
    newRotation.Pitch = FMath::Clamp(newRotation.Pitch, -70.0f, 70.0f);

    SpringArm->SetWorldRotation(newRotation);
}

void ABaseCharacter::OnRep_CurrentHealCount()
{
	OnHealCountChanged.Broadcast(CurrentHealCount);
}

void ABaseCharacter::OnRep_CurrentMaxSpeed()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentMaxSpeed: %f"), CurrentSpeed);
	GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;
}

void ABaseCharacter::StartSprint() 
{
    if (!HasAuthority())
    {
        Server_StartSprint();
    }
	else
	{
		Server_StartSprint_Implementation();
	}
}

void ABaseCharacter::StopSprint() 
{
	if (!HasAuthority())
	{
		Server_StopSprint();
	}
	else
	{
		Server_StopSprint_Implementation();
	}
}

void ABaseCharacter::LightAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("Light Attack"));

	if (!HasAuthority())
	{
		// 클라라면 서버에 요청
		Server_Attack();
	}
	else
	{
		// 서버라면 바로 실행
		Server_Attack_Implementation();
	}
}

void ABaseCharacter::HeavyAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("Heavy Attack"));
}

void ABaseCharacter::Roll()
{
	if (!HasAuthority())
	{
		Server_Roll();
	}
	else
	{
		Server_Roll_Implementation();
	}
}

// Server Rotation
void ABaseCharacter::Server_SetRotation_Implementation(FRotator NewRotation)
{
	ReplicatedRotation = NewRotation;
}

// Server Sprint
void ABaseCharacter::Server_StartSprint_Implementation()
{
	if (!StaminaCompRef || !StaminaCompRef->CanSprint())
	{
		CurrentSpeed = WalkSpeed;
		GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;
	}
	else
	{
		CurrentSpeed = SprintSpeed;
		GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;

		StaminaCompRef->StartSprint();
	}
}

void ABaseCharacter::Server_StopSprint_Implementation()
{
	CurrentSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;

	if (StaminaCompRef)
	{
		StaminaCompRef->StopSprint();
	}
}

// Server Attack
void ABaseCharacter::Server_Attack_Implementation()
{
	if (!BP_CanAttack())
	{
		return;
	}

	if (!StaminaCompRef || !StaminaCompRef->TryUseStamina(AttackStaminaCost))
	{
		return;
	}

	StopRecoveryStamina();

	BP_ExecuteAttack();

	Multicast_PlayAttackAnimation();
}

void ABaseCharacter::Multicast_PlayAttackAnimation_Implementation()
{
	BP_PlayAttackAnimation();
}

// Server Hit
void ABaseCharacter::OnDamageReceived_Implementation(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (HealthCompRef->CurrentHealth <= 0.0f)
	{
		return;
	}

	bCanPlayerControl = false;

	StopRecoveryStamina();

	Multicast_PlayHitAnimation();
}

void ABaseCharacter::Multicast_PlayHitAnimation_Implementation()
{
	BP_PlayHitAnimation();
}

// Server Roll
void ABaseCharacter::Server_Roll_Implementation()
{
	if (!BP_CanRoll())
	{
		return;
	}

	if (!StaminaCompRef || !StaminaCompRef->TryUseStamina(AttackStaminaCost))
	{
		return;
	}

	StopRecoveryStamina();

	bIsInvincible = true;
	bCanPlayerControl = false;

	FVector directionVector;
	if (IsLocallyControlled())
	{
		directionVector = TargetRotation.Vector();
	}
	else
	{
		directionVector = ReplicatedRotation.Vector();
	}

	LaunchCharacter(directionVector * 900.0f + FVector(0.0f, 0.0f, 200.0f), true, true);

	Multicast_PlayRollAnimation();
}

void ABaseCharacter::Multicast_PlayRollAnimation_Implementation()
{
	BP_PlayRollAnimation();
}

void ABaseCharacter::SelfHeal()
{
	if (!HasAuthority())
	{
		Server_SelfHeal();
	}
	else
	{
		Server_SelfHeal_Implementation();
	}
}

void ABaseCharacter::Server_SelfHeal_Implementation()
{
	if (!BP_CanSelfHeal())
	{
		return;
	}

	if (CurrentHealCount <= 0)
	{
		return;
	}

	bCanPlayerControl = false;
	
	CurrentHealCount--;
	OnHealCountChanged.Broadcast(CurrentHealCount);

	HealthCompRef->Heal(50.0f);

	Multicast_PlaySelfHealAnimation();
}

void ABaseCharacter::Multicast_PlaySelfHealAnimation_Implementation()
{
	BP_PlaySelfHealAnimation();
}

void ABaseCharacter::OnDeath_Implementation()
{
	bCanPlayerControl = false;

	BP_PlayDeathAnimation();
}

// Replication
void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, bIsInvincible);
    DOREPLIFETIME(ABaseCharacter, bCanDoComboAttack);
	DOREPLIFETIME(ABaseCharacter, bCanPlayerControl);
    DOREPLIFETIME(ABaseCharacter, ReplicatedRotation);
    DOREPLIFETIME(ABaseCharacter, CurrentSpeed);
}