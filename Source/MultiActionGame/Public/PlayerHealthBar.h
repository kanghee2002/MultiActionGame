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
	// HealthComponent 바인딩
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeWithHealthComponent(UHealthComponent* HealthComp);

protected:
	virtual void NativeConstruct() override;

	// 이벤트 콜백 
	UFUNCTION(BlueprintCallable)
	void UpdateHealthBarUI(float NewHealth, float Delta);

	// 위젯 그래프에서 BindWidget 해야 함 
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* HealthProgressBar;

	UPROPERTY()
	TObjectPtr<UHealthComponent> BoundHealthComponent;

private:
	// 최대 체력 값 
	float MaxHealth;
};
