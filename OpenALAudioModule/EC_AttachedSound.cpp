// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OpenALAudioModule.h"
#include "SoundSystem.h"
#include "EC_AttachedSound.h"

namespace OpenALAudio
{
    EC_AttachedSound::EC_AttachedSound(Foundation::ModuleInterface *module) : Foundation::ComponentInterface(module->GetFramework())
	{
	}

    EC_AttachedSound::~EC_AttachedSound()
	{
	}
}