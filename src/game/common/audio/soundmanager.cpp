#include "soundmanager.h"

void SoundManager::Reset() {}

void SoundManager::Add_Audio_Event(AudioEventRTS *event) {}

bool SoundManager::Can_Play_Now(AudioEventRTS *event)
{
    return false;
}

bool SoundManager::Violates_Voice(AudioEventRTS *event)
{
    return false;
}

bool SoundManager::Is_Interrupting(AudioEventRTS *event)
{
    return (event->Get_Event_Info()->Get_Control() & CONTROL_IS_INTERRUPTING) != 0;
}
