// Fill out your copyright notice in the Description page of Project Settings.

#include "LandedAnimNotify.h"
#include "BaseAnimInstance.h"

void ULandedAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) {
	if (!MeshComp) return;

	UBaseAnimInstance* inst = Cast<UBaseAnimInstance>(MeshComp->GetAnimInstance());

	if (inst) {
		inst->Landed();
	}
}
