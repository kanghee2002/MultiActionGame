// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CheckBox.h"
#include "RadioCheckBox.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelected, int32, InValue);

UCLASS()
class MULTIACTIONGAME_API URadioCheckBox : public UCheckBox
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	int32 Index;

	FOnSelected OnSelected;

	void Initialize(TArray<URadioCheckBox*> CheckBoxGroup);

private:
	TArray<URadioCheckBox*> RadioCheckBoxes;

	UFUNCTION()
	void TriggerAction(bool bIsChecked);

	UFUNCTION()
	void UncheckOthers(bool bIsChecked);
};
