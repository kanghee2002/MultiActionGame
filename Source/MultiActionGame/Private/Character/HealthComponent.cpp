
#include "Character/HealthComponent.h"
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	DefaultMaxHealth = 100;
	CurrentHealth = DefaultMaxHealth;
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();


	AActor* Owner = GetOwner();
	if (Owner) {
		// OnTakeAnyDamage 호출은 서버에서 이루어짐
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
	// 이 함수가 서버에서만 실행되도록 보장
	if (!GetOwner()->HasAuthority()) return; 

	if (Damage <= 0) return;

	if (DamagedActor == DamageCauser) return;

	if (!ShouldApplyDamage(DamageCauser, InstigatedBy)) return; // 팀킬 필터링

	AActor* MyActor = GetOwner();
	FString ActorName;
	if (MyActor)
	{
		ActorName = MyActor->GetName();
	}

	UE_LOG(LogTemp, Warning, TEXT("TakeDamage: Damage=%.2f, Causer=%s, Object=%s, Obj Ptr=%p"), Damage,
		DamageCauser ? *DamageCauser->GetName() : TEXT("None"), *ActorName, MyActor);

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

void UHealthComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast(CurrentHealth, 0.f);

	// 로컬 클라이언트에서 로그를 찍어 동기화 확인
	UE_LOG(LogTemp, Log, TEXT("Client Health Synced: %f"), CurrentHealth);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, CurrentHealth);
}