// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAnimInstance.h"
#include <GameFramework/Character.h>
#include <GameFramework/CharacterMovementComponent.h>

void UBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds) {
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedCharacter)
		CachedCharacter = Cast<ACharacter>(TryGetPawnOwner());

	if (!CachedCharacter) return;

	bIsInAir = CachedCharacter->GetCharacterMovement()->IsFalling();
	Speed = CachedCharacter->GetVelocity().Length();
}
