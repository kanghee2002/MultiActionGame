// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacter.h"
#include "InputActionGroup.h"
#include "MainPlayerController.h"
#include "InGameHUD.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/HealthBarWidget.h"

#include "BaseAnimInstance.h"
#include "MainGameMode.h"
#include "Engine/EngineTypes.h"

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
	bUseReplicatedRotation = false;

	bIsInvincible = false;
	bIsFaint = false;

	BasicAttackDamage = 3.5f;
	LightAttackStaminaCost = 10.0f;
	HeavyAttackStaminaCost = 20.0f;
	RollStaminaCost = 30.0f;
	CurrentHealCount = 5;
	SprintSpeed = 700.0f;
	WalkSpeed = 400.0f;
}

void ABaseCharacter::BeginPlay() 
{
	Super::BeginPlay();

	// 초기화 (서버)
	if (HasAuthority())
	{
		CurrentSpeed = WalkSpeed;
		GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;

		bCanPlayerControl = true;
	}

	HealthCompRef->OnFullHealth.AddDynamic(this, &ABaseCharacter::OnFullHealth);

	HealthCompRef->OnDeath.AddDynamic(this, &ABaseCharacter::OnDeath);

	// 체력바 초기화
	if (UWidgetComponent* widgetComponent = FindComponentByClass<UWidgetComponent>())
	{
		if (UHealthBarWidget* healthBarComponent = Cast<UHealthBarWidget>(widgetComponent->GetUserWidgetObject()))
		{
			healthBarComponent->InitializeHealthComponent(HealthCompRef);
		}
	}

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
	FRotator targetRotatation;

	if (IsLocallyControlled())
	{
		if (bUseReplicatedRotation)
		{
			targetRotatation = ReplicatedRotation;
		}
		else
		{
			targetRotatation = LocalRotation;
		}
	}
	else
	{
		targetRotatation = ReplicatedRotation;
	}

	float angleDiff = FMath::Abs(FMath::FindDeltaAngleDegrees(currentRotation.Yaw, targetRotatation.Yaw));

	if (angleDiff > 1.0f)
	{
		FRotator newRotation = FMath::RInterpTo(currentRotation, targetRotatation, DeltaTime, RotationSpeed);
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
    component->BindAction(InputActionGroup->JumpAction, ETriggerEvent::Started, this, &ABaseCharacter::JumpAction);
	component->BindAction(InputActionGroup->LightAttackAction, ETriggerEvent::Started, this, &ABaseCharacter::LightAttack);
	component->BindAction(InputActionGroup->HeavyAttackAction, ETriggerEvent::Started, this, &ABaseCharacter::HeavyAttack);
	component->BindAction(InputActionGroup->SelfHealAction, ETriggerEvent::Started, this, &ABaseCharacter::SelfHeal);
	component->BindAction(InputActionGroup->SkillAction, ETriggerEvent::Started, this, &ABaseCharacter::UseSkill);
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
				bUseReplicatedRotation = false;
				LocalRotation = targetRotation;
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
	GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;
}

void ABaseCharacter::OnRep_CurrentSkillCooldown()
{
	OnSkillCooldownChanged.Broadcast(CurrentSkillCooldown, SkillCooldown);
}

void ABaseCharacter::OnRep_ReplicatedRotation()
{
	if (IsLocallyControlled())
	{
		bUseReplicatedRotation = true;
	}
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
	if (!HasAuthority())
	{
		Server_LightAttack();
	}
	else
	{
		Server_LightAttack_Implementation();
	}
}

void ABaseCharacter::HeavyAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("Heavy Attack"));

	if (!HasAuthority())
	{
		Server_HeavyAttack();
	}
	else
	{
		Server_HeavyAttack_Implementation();
	}
}

void ABaseCharacter::UseSkill()
{
	UE_LOG(LogTemp, Warning, TEXT("Use Skill"));

	if (!HasAuthority())
	{
		Server_UseSkill();
	}
	else
	{
		Server_UseSkill_Implementation();
	}
}

void ABaseCharacter::JumpAction()
{
	BP_JumpAction();
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

// Server Light Attack
void ABaseCharacter::Server_LightAttack_Implementation()
{
	if (!BP_CanAttack())
	{
		return;
	}

	if (!StaminaCompRef || !StaminaCompRef->TryUseStamina(LightAttackStaminaCost))
	{
		return;
	}

	StopRecoveryStamina();

	BP_ExecuteLightAttack();

	Multicast_PlayLightAttackAnimation();
}

void ABaseCharacter::Multicast_PlayLightAttackAnimation_Implementation()
{
	BP_PlayLightAttackAnimation();
}

// Server Heavy Attack
void ABaseCharacter::Server_HeavyAttack_Implementation()
{
	if (!BP_CanAttack())
	{
		return;
	}

	if (!StaminaCompRef || !StaminaCompRef->TryUseStamina(HeavyAttackStaminaCost))
	{
		return;
	}

	StopRecoveryStamina();

	BP_ExecuteHeavyAttack();

	Multicast_PlayHeavyAttackAnimation();
}

void ABaseCharacter::Multicast_PlayHeavyAttackAnimation_Implementation()
{
	BP_PlayHeavyAttackAnimation();
}

// Server Use Skill
void ABaseCharacter::Server_UseSkill_Implementation()
{
	if (!BP_CanAttack())
	{
		return;
	}

	if (CurrentSkillCooldown > 0.0f)
	{
		return;
	}

	if (!StaminaCompRef || !StaminaCompRef->TryUseStamina(SkillStaminaCost))
	{
		return;
	}

	CurrentSkillCooldown = SkillCooldown;
	OnSkillCooldownChanged.Broadcast(CurrentSkillCooldown, SkillCooldown);

	StopRecoveryStamina();

	BP_ExecuteSkill();

	Multicast_PlaySkillAnimation();
}

void ABaseCharacter::Multicast_PlaySkillAnimation_Implementation()
{
	BP_PlaySkillAnimation();
}

void ABaseCharacter::StartSkillCooldown()
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(SkillCooldownTimerHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(
			SkillCooldownTimerHandle,
			this,
			&ABaseCharacter::DecreaseSkillCooldown,
			CooldownUpdateInterval,
			true  // 반복 실행
		);
	}
}

void ABaseCharacter::DecreaseSkillCooldown()
{
	CurrentSkillCooldown -= CooldownUpdateInterval;

	if (CurrentSkillCooldown <= 0.0f)
	{
		CurrentSkillCooldown = 0.0f;

		GetWorld()->GetTimerManager().ClearTimer(SkillCooldownTimerHandle);
	}

	OnSkillCooldownChanged.Broadcast(CurrentSkillCooldown, SkillCooldown);
}

// Server Hit
void ABaseCharacter::OnDamageReceived_Implementation(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (HealthCompRef->CurrentHealth <= 0.0f || bIsFaint)
	{
		return;
	}

	if (bIsSuperArmored)
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

	if (!StaminaCompRef || !StaminaCompRef->TryUseStamina(RollStaminaCost))
	{
		return;
	}

	StopRecoveryStamina();

	bCanPlayerControl = false;

	bIsInvincible = true;

	FVector directionVector;
	if (IsLocallyControlled())
	{
		directionVector = LocalRotation.Vector();
	}
	else
	{
		directionVector = ReplicatedRotation.Vector();
	}

	LaunchCharacter(directionVector * 1000.0f + FVector(0.0f, 0.0f, 180.0f), true, true);

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

void ABaseCharacter::OnFullHealth()
{
	if (bIsFaint)
	{
		Revive();
	}
}

void ABaseCharacter::Revive()
{
	bIsFaint = false;
	bIsInvincible = true;

	if (BodyCapsule)
	{
		BodyCapsule->SetCollisionProfileName(TEXT("Hero"));
	}

	AMainGameMode* GameMode = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->ProcessPlayerRevive();
	}

	Multicast_PlayReviveAnimation();
}

void ABaseCharacter::Multicast_PlayReviveAnimation_Implementation()
{
	BP_PlayReviveAnimation();
}

void ABaseCharacter::OnDeath_Implementation()
{
	bCanPlayerControl = false;
	bIsFaint = true;

	if (BodyCapsule)
	{
		BodyCapsule->SetCollisionProfileName(TEXT("Boss"));
	}

	AMainGameMode* GameMode = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->ProcessPlayerDeath(CharacterType);
	}

	Multicast_PlayDeathAnimation();
}

void ABaseCharacter::Multicast_PlayDeathAnimation_Implementation()
{
	BP_PlayDeathAnimation();
}

// Replication
void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, bIsInvincible);
	DOREPLIFETIME(ABaseCharacter, bIsSuperArmored);
    DOREPLIFETIME(ABaseCharacter, bCanDoComboAttack);
	DOREPLIFETIME(ABaseCharacter, bCanPlayerControl);
    DOREPLIFETIME(ABaseCharacter, ReplicatedRotation);
    DOREPLIFETIME(ABaseCharacter, CurrentHealCount);
    DOREPLIFETIME(ABaseCharacter, CurrentSpeed);
    DOREPLIFETIME(ABaseCharacter, CurrentSkillCooldown);
}