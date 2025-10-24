#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "SkillData.h"
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

	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeHealCountText(ABaseCharacter* MyCharacter);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeSkillCooldownProgressBar(ABaseCharacter* MyCharacter);

	// HealthComponent Binding
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeBossHealthComponent(UHealthComponent* HealthComp);

	// Boss UI Setting
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeHUD(ECharacterType CharacterType);

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
	void UpdateSkillCooldownUI(float NewCooldown, float Cooldown);

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
	USizeBox* HealInfo;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	USizeBox* HealSkillSpacer;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* HealCountText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* SkillImage;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* SkillCooldownProgressBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	USizeBox* BossInfo;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillData", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkillData> BossSkillData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillData", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkillData> KnightSkillData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillData", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkillData> ArcherSkillData;
};
