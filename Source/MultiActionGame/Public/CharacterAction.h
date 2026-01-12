// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterAction.generated.h"

UENUM(BlueprintType)
enum class ECharacterAction : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	LightAttack = 1 UMETA(DisplayName = "Light Attack"),
	HeavyAttack = 2 UMETA(DisplayName = "Heavy Attack"),
	Skill = 3		UMETA(DisplayName = "Skill"),
	Jump = 4		UMETA(DisplayName = "Jump"),
	SelfHeal = 5	UMETA(DisplayName = "Self Heal")
};
