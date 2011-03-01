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

#include "SceneManager.h"
#include "WorldLogicInterface.h"

#include "UiServiceInterface.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "CoreStringUtils.h"
#include "TtsModule.h"
#include "SettingsWidget.h"

#include "EC_TtsVoice.h"
#include "EC_TtsChat.h"

#include "MemoryLeakCheck.h"

namespace Tts
{
	const std::string TtsModule::module_name_ = std::string("TtsModule");

	TtsModule::TtsModule() :
	    QObject(),
        IModule(module_name_),
        settings_widget_(0),
        publish_own_voice_(false)
	{

	}

	TtsModule::~TtsModule()
	{
        SAFE_DELETE(settings_widget_);
	}

	void TtsModule::Load()
	{
		DECLARE_MODULE_EC(EC_TtsVoice);
		DECLARE_MODULE_EC(EC_TtsChat);
	}

	void TtsModule::UnLoad()
	{
	}

	void TtsModule::Initialize()
	{
		tts_service_ = TtsServicePtr(new TtsService(framework_));
		framework_->GetServiceManager()->RegisterService(Service::ST_Tts, tts_service_);
        connect(tts_service_.get(), SIGNAL(SettingsUpdated()), this, SLOT(ReadTtsSettings()));
        ReadTtsSettings();
	}

    void TtsModule::ReadTtsSettings()
    {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/Tts");
        bool old_value = publish_own_voice_;
        publish_own_voice_ = settings.value("Tts/publish_own_voice", true).toBool();
        if (old_value == true && publish_own_voice_ == false)
            UnpublishOwnAvatarVoice();
    }

	void TtsModule::PostInitialize()
	{
        SetupSettingsWidget();
    }
	void TtsModule::Uninitialize()
	{
		framework_->GetServiceManager()->UnregisterService(tts_service_);
		tts_service_.reset();
	}

    void TtsModule::UnpublishOwnAvatarVoice()
    {
        /// @todo IMPLEMENT
    }

    void TtsModule::SetupSettingsWidget()
    {
        UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
        if (!ui)
            return;

        settings_widget_ = new SettingsWidget(framework_);
        ui->AddSettingsWidget(settings_widget_, "TTS");
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
