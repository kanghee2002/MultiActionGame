#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/HorizontalBox.h"
#include "InGameHUD.generated.h"

class ABaseCharacter;
class UProgressBar;
class UBackProgressBar;
class UHealthComponent;
class UStaminaComponent;

UCLASS()
class MULTIACTIONGAME_API UInGameHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// HealthComponent Binding
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeHealthComponent(UHealthComponent* HealthComp);

	// StaminaComponent Binding
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeStaminaComponent(UStaminaComponent* StaminaComp);

	// StaminaComponent Binding
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeHealCountText(ABaseCharacter* MyCharacter);

	// HealthComponent Binding
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeBossHealthComponent(UHealthComponent* HealthComp);

	// Boss UI Setting
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetBossHUD();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 이벤트 콜백 
	UFUNCTION(BlueprintCallable)
	void UpdateHealthBarUI(float NewHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable)
	void UpdateStaminaBarUI(float NewStamina);

	UFUNCTION(BlueprintCallable)
	void UpdateHealCountUI(int NewHealCount);

	UFUNCTION(BlueprintCallable)
	void UpdateBossHealthBarUI(float NewHealth, float MaxHealth);

	// 부드러운 체력 증가
	void UpdateHealthSmoothing(float Delta);

	// 위젯 그래프에서 BindWidget 해야 함 
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* HealthProgressBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UBackProgressBar* HealthBackProgressBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* StaminaProgressBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UBackProgressBar* StaminaBackProgressBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UOverlay* HealOverlay;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* HealCountText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UHorizontalBox* BossInfo;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* BossHealthProgressBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UBackProgressBar* BossHealthBackProgressBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* BossNameText;

	UPROPERTY()
	TObjectPtr<UHealthComponent> BoundHealthComponent;

private:
	float MaxStamina;

	bool bIsHealthIncreased;
	float TargetHealthPercent;
};
