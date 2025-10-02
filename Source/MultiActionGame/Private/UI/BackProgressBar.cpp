
#include "UI/BackProgressBar.h"

UBackProgressBar::UBackProgressBar()
{
	bIsDecreasing = false;
}

void UBackProgressBar::SetTargetPercent(float NewPercent)
{
	TargetPercent = NewPercent;

	if (bIsDecreasing)
	{
		return;
	}

	bIsDecreasing = true;
	
	GetWorld()->GetTimerManager().SetTimer(
		DelayTimerHandle,
		this,
		&UBackProgressBar::UpdatePercent,
		DecreaseDelay,
		false  // 한 번만 실행
	);
}

void UBackProgressBar::UpdatePercent()
{
	GetWorld()->GetTimerManager().SetTimer(
		UpdateTimerHandle,
		[this]()
		{
			float currentPercent = GetPercent();

			// 목표값에 도달했는지 확인
			if (currentPercent <= TargetPercent)
			{
				SetPercent(TargetPercent);

				// 타이머 정지
				GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
				bIsDecreasing = false;
				
				return;
			}

			// 값 감소
			float newPercent = currentPercent - (DecreaseSpeed * UpdateInterval);
			SetPercent(newPercent);
		},
		UpdateInterval,
		true  // 반복 실행
	);
}