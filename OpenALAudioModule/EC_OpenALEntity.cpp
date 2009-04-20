// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OpenALAudioModule.h"
#include "OpenALAudio.h"
#include "EC_OpenALEntity.h"

namespace OpenALAudio
{
    EC_OpenALEntity::EC_OpenALEntity(Foundation::ModuleInterface *module) : Foundation::ComponentInterface(module->GetFramework())
	{
	}

    EC_OpenALEntity::~EC_OpenALEntity()
	{
	}
}