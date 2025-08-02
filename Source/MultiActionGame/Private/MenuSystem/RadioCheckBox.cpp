// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuSystem/RadioCheckBox.h"

void URadioCheckBox::Initialize(TArray<URadioCheckBox*> CheckBoxGroup)
{
	RadioCheckBoxes = CheckBoxGroup;

	this->OnCheckStateChanged.Clear();

	this->OnCheckStateChanged.AddDynamic(this, &URadioCheckBox::UncheckOthers);
	this->OnCheckStateChanged.AddDynamic(this, &URadioCheckBox::TriggerAction);
}

void URadioCheckBox::TriggerAction(bool bIsChecked)
{
	OnSelected.Broadcast(Index);
}


void URadioCheckBox::UncheckOthers(bool bIsChecked)
{
	for (URadioCheckBox* CheckBox : RadioCheckBoxes)
	{
		CheckBox->SetCheckedState(ECheckBoxState::Unchecked);
	}
	this->SetCheckedState(ECheckBoxState::Checked);
}