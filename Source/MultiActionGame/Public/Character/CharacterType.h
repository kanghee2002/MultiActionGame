// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterType.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	Boss = 0,
	Knight = 1,
	Archer = 2,
	Wizard = 3,
	MAX = 4		UMETA(Hidden),
};
