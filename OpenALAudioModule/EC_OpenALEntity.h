// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenALModule_EC_OpenALEntity_h
#define incl_OpenALModule_EC_OpenALEntity_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace OpenALAudio
{
    class MODULE_API EC_OpenALEntity : public Foundation::ComponentInterface
	{
		DECLARE_EC(EC_OpenALEntity);
	public:
		virtual ~EC_OpenALEntity();

	private:
		EC_OpenALEntity(Foundation::ModuleInterface *module);
	};
}

#endif
