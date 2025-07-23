#include "SoundManager.h"

void USoundManager::playSound2d(UObject* worldContextObject, USoundBase* sound, float volume)
{
	if (sound && worldContextObject)
	{
		UGameplayStatics::PlaySound2D(worldContextObject, sound, volume);
	}
}


void USoundManager::playSoundAtLocation(UObject* worldContextObject, USoundBase* sound, FVector location, float volume)
{
    if (sound && worldContextObject)
    {
        UGameplayStatics::PlaySoundAtLocation(worldContextObject, sound, location, volume);
    }
}


UAudioComponent* USoundManager::playBgm(UObject* worldContextObject, USoundBase* sound, float volume, bool loop)
{
    if (sound && worldContextObject)
    {
        UAudioComponent* audioComp = UGameplayStatics::SpawnSound2D(worldContextObject, sound, volume, 1.0f, 0.0f, nullptr, loop);
        if (audioComp)
        {
            audioComp->Play();
        }
        return audioComp;
    }
    return nullptr;
}