//$ HEADER_NEW_FILE $ 
/**
 *  TTS Module
 *
 *
 *
 *  @file   TtsModule.cpp
 *  @brief  TTS Module registers a TTS Service, integrating Festival Speech Synthesis
 *			System with realXtend.
 */

#include "StableHeaders.h"

#include "EventManager.h"
#include "ModuleManager.h"
#include "CoreStringUtils.h"
#include "MemoryLeakCheck.h"

#include <QColor>


#include "TtsModule.h"
#include "EC_TtsVoice.h"

namespace Tts
{
	const std::string TtsModule::module_name_ = std::string("TtsModule");

	TtsModule::TtsModule() :
	    QObject(),
        IModule(module_name_)
	{

	}

	TtsModule::~TtsModule()
	{
	}

	void TtsModule::Load()
	{
		DECLARE_MODULE_EC(EC_TtsVoice);

		if (QDir("tmp").exists())
			boost::filesystem::remove_all("tmp");

		QDir().mkdir("tmp");
	}

	void TtsModule::UnLoad()
	{
		if (QDir("tmp").exists())
			boost::filesystem::remove_all("tmp");
	}

	void TtsModule::Initialize()
	{
		tts_service_ = TtsServicePtr(new TtsService(framework_));
		framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Tts, tts_service_);
	}
	void TtsModule::PostInitialize()
	{
	}


	void TtsModule::Uninitialize()
	{
		framework_->GetServiceManager()->UnregisterService(tts_service_);
		tts_service_.reset();
	}
} // end of namespace: Tts


extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}


using namespace Tts;

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(TtsModule)
POCO_END_MANIFEST
