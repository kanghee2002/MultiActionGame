#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InGameHUD.generated.h"

class UProgressBar;
class UHealthComponent;
class UStaminaComponent;

UCLASS()
class MULTIACTIONGAME_API UInGameHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// HealthComponent 바인딩
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeHealthComponent(UHealthComponent* HealthComp);

	// StaminaComponent 바인딩
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeStaminaComponent(UStaminaComponent* StaminaComp);

protected:
	virtual void NativeConstruct() override;

	// 이벤트 콜백 
	UFUNCTION(BlueprintCallable)
	void UpdateHealthBarUI(float NewHealth, float Delta);

	UFUNCTION(BlueprintCallable)
	void UpdateStaminaBarUI(float NewStamina);

	// 위젯 그래프에서 BindWidget 해야 함 
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* HealthProgressBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* StaminaProgressBar;

	UPROPERTY()
	TObjectPtr<UHealthComponent> BoundHealthComponent;

private:
	float MaxHealth;

	float MaxStamina;
};
