#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "UObject/NoExportTypes.h"
#include "SoundManager.generated.h"

UCLASS()
class MULTIACTIONGAME_API USoundManager : public UObject
{
	GENERATED_BODY()

public:
	static void playSound2d(UObject* worldContextObject, USoundBase* sound, float volume = 1.0f);
	static void playSoundAtLocation(UObject* worldContextObject, USoundBase* sound, FVector location, float volume = 1.0f);
	static UAudioComponent* playBgm(UObject* worldContextObject, USoundBase* sound, float volume = 1.0f, bool loop = true);





};
