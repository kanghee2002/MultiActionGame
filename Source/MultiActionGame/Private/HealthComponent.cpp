
/// <summary>
/// ��Ƽ�÷��� �����ϸ�, ���� �ڵ�� Ŭ���̾�Ʈ������ ó���ϴ°Ŷ� �̷��� �ϸ� �� �� ����..
/// 
/// �������� ó���ϵ��� Server RPC ����Ѵٴ� ���� �ֳ׿�.
/// 
/// �ϴ��� �̷��� �� �ѰԿ�.
/// </summary>



#include "HealthComponent.h"
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	DefaultMaxHealth = 100;
	CurrentHealth = DefaultMaxHealth;
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();


	AActor* Owner = GetOwner();
	if (Owner) {
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::TakeDamage);
	}

}

void UHealthComponent::Heal(float Amount)
{
	if (Amount <= 0 || CurrentHealth <= 0) return;

	float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, DefaultMaxHealth);

	OnHealthChanged.Broadcast(CurrentHealth, CurrentHealth - OldHealth);
}

void UHealthComponent::ResetHealth()
{
	CurrentHealth = DefaultMaxHealth;

	OnHealthChanged.Broadcast(CurrentHealth, 0);
}

void UHealthComponent::TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0)
		return;

	if (!ShouldApplyDamage(DamageCauser, InstigatedBy)) return; // ��ų ���͸�

	float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, DefaultMaxHealth);

	OnHealthChanged.Broadcast(CurrentHealth, CurrentHealth - OldHealth);

	if (CurrentHealth <= 0)
	{
		OnDeath.Broadcast();
	}
}


bool UHealthComponent::ShouldApplyDamage(AActor* DamageCauser, AController* InstigatedBy) const
{
	// TOOD: ���� ���̸� ���� 

	if (DamageCauser == GetOwner()) return false; // �����ο��� ������ �������� �ʰ� ����
	return true;
}

