
/// <summary>
/// 멀티플레이 생각하면, 현재 코드는 클라이언트에서만 처리하는거라 이렇게 하면 안 될 수도..
/// 
/// 서버에서 처리하도록 Server RPC 써야한다는 말이 있네요.
/// 
/// 일단은 이렇게 해 둘게요.
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

	if (!ShouldApplyDamage(DamageCauser, InstigatedBy)) return; // 팀킬 필터링

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
	// TOOD: 같은 팀이면 리턴 

	if (DamageCauser == GetOwner()) return false; // 스스로에게 데미지 가해지지 않게 리턴
	return true;
}

