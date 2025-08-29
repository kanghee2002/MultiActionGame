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

	// 기존에 바인딩된 컴포넌트가 있고, 새로운 컴포넌트와 다른 경우 해제
	if (BoundHealthComponent && BoundHealthComponent != HealthComp)
	{
		BoundHealthComponent->OnHealthChanged.RemoveAll(this);
	}

	// 새 컴포넌트를 멤버 변수에 저장
	BoundHealthComponent = HealthComp;
	MaxHealth = HealthComp->DefaultMaxHealth;

	// 이미 바인딩되어 있지 않은 경우에만 바인딩
	HealthComp->OnHealthChanged.AddUniqueDynamic(this, &UPlayerHealthBar::UpdateHealthBarUI);

	UE_LOG(LogTemp, Warning, TEXT("Bound to HealthComponent: %p (Owner: %s) (Owner: %p)"),
		HealthComp, HealthComp->GetOwner() ? *HealthComp->GetOwner()->GetName() : TEXT("None"), HealthComp->GetOwner());

	UpdateHealthBarUI(HealthComp->DefaultMaxHealth, 0.f);
}

void UPlayerHealthBar::UpdateHealthBarUI(float NewHealth, float Delta)
{
	if (HealthProgressBar && MaxHealth > 0)
	{
		float Percent = NewHealth / MaxHealth;
		HealthProgressBar->SetPercent(Percent);
	}

	UE_LOG(LogTemp, Warning, TEXT("Update HealthBar: %f. / Delta=%f "), NewHealth, Delta);
}
