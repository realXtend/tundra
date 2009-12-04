// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenALAudio_EC_AttachedSound_h
#define incl_OpenALAudio_EC_AttachedSound_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace OpenALAudio
{
    class OPENAL_MODULE_API EC_AttachedSound : public Foundation::ComponentInterface
	{
		DECLARE_EC(EC_AttachedSound);
	public:
		virtual ~EC_AttachedSound();

	private:
		EC_AttachedSound(Foundation::ModuleInterface *module);
	};
}

#endif
