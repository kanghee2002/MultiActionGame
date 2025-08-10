#include "PlayerHealthBar.h"
#include "Components/ProgressBar.h"
#include "HealthComponent.h"

void UPlayerHealthBar::NativeConstruct()
{
	Super::NativeConstruct();
	// 초기화 작업 가능
}

void UPlayerHealthBar::InitializeWithHealthComponent(UHealthComponent* HealthComp)
{
	if (!HealthComp) return;
	MaxHealth = HealthComp->DefaultMaxHealth;
	// 델리게이트 바인딩
	HealthComp->OnHealthChanged.AddDynamic(this, &UPlayerHealthBar::UpdateHealthBarUI);

	UpdateHealthBarUI(HealthComp->DefaultMaxHealth, 0.f);

}

void UPlayerHealthBar::UpdateHealthBarUI(float NewHealth, float Delta)
{
	if (HealthProgressBar && MaxHealth > 0)
	{
		float Percent = NewHealth / MaxHealth;
		HealthProgressBar->SetPercent(Percent);
	}

	UE_LOG(LogTemp, Log, TEXT("Health updated: %f. \nDelta=%f "), NewHealth, Delta);

}
