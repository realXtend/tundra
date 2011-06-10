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

#include "EventManager.h"
#include "ModuleManager.h"
#include "CoreStringUtils.h"
#include "TtsModule.h"
#include "SettingsWidget.h"

#include "EC_TtsVoice.h"

#include "UiAPI.h"
#include "UiMainWindow.h"

#include "MemoryLeakCheck.h"

namespace Tts
{
	const std::string TtsModule::module_name_ = std::string("TtsModule");

	TtsModule::TtsModule() :
	    QObject(),
        IModule(module_name_),
        settings_widget_(0),
        tts_service_(0),
        publish_own_voice_(false)
	{

	}

	TtsModule::~TtsModule()
	{
	}

	void TtsModule::Load()
	{
		DECLARE_MODULE_EC(EC_TtsVoice);
	}

	void TtsModule::UnLoad()
	{
	}

	void TtsModule::Initialize()
	{
		tts_service_ = new TtsService(framework_);
        connect(tts_service_, SIGNAL(SettingsUpdated()), this, SLOT(ReadTtsSettings()));
		framework_->RegisterDynamicObject("tts", tts_service_);
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
        if (!settings_widget_)
            SetupSettingsWidget();
    }
	void TtsModule::Uninitialize()
	{
        SAFE_DELETE(settings_widget_);
        SAFE_DELETE(tts_service_)
	}

    void TtsModule::UnpublishOwnAvatarVoice()
    {
        /// @todo IMPLEMENT
    }

    void TtsModule::SetupSettingsWidget()
    {
        if(tts_service_)
        {
            settings_widget_ = new SettingsWidget(tts_service_);
            settings_widget_->setWindowTitle(QString("TTS Settings"));
        }
    }

    QWidget* TtsModule::GetSettingsWidget()
    {
        if (!settings_widget_)
            SetupSettingsWidget();
        return settings_widget_;
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
