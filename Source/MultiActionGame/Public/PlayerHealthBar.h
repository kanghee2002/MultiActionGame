#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHealthBar.generated.h"

class UProgressBar;
class UHealthComponent;

UCLASS()
class MULTIACTIONGAME_API UPlayerHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	// HealthComponent ���ε�
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeWithHealthComponent(UHealthComponent* HealthComp);

protected:
	virtual void NativeConstruct() override;

	// �̺�Ʈ �ݹ� 
	UFUNCTION()
	void UpdateHealthBarUI(float NewHealth, float Delta);

	// ���� �׷������� BindWidget �ؾ� �� 
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* HealthProgressBar;

private:
	// �ִ� ü�� �� 
	float MaxHealth;
};
