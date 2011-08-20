//$ HEADER_NEW_FILE $ 
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   NotificationTts.cpp
 *  @brief  Synthetizes notifications using TTS Service
 *			
 */

#include "StableHeaders.h"

#include "NotificationTts.h"

#include "TtsServiceInterface.h"
#include "UiServiceInterface.h" 
#include <QSettings>

#include <QColor>


namespace NotifiTts
{

	const std::string NotificationTts::module_name_ = std::string("NotificationTts");

	NotificationTts::NotificationTts() :
		QObject(),
        IModule(module_name_),
        enabled_(false),
        notification_voice_("")
	{
	}

	NotificationTts::~NotificationTts()
	{
	}

    void NotificationTts::UpdateTtsSettings()
    {
        /// @todo signal when settings are changed
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/Tts");
        enabled_ = settings.value("Tts/play_notifications_messages", false).toBool();
        notification_voice_ = settings.value("Tts/notification_voice", "").toString();
    }

	void NotificationTts::Load()
	{
	}

	void NotificationTts::UnLoad()
	{
	}

	void NotificationTts::Initialize()
	{
		if (framework_ &&  framework_->GetServiceManager())
        {
			UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
            if (ui)
				connect(ui, SIGNAL(Notification(const QString&)), SLOT(Notification2Speech(const QString&)));
		}

	}
	void NotificationTts::PostInitialize()
	{
	}

	void NotificationTts::Uninitialize()
	{
	}
	
	void NotificationTts::Notification2Speech(const QString &message)
	{
        UpdateTtsSettings();

        if (!enabled_)
            return;

		tts_service_ = framework_->GetService<Tts::TtsServiceInterface>();
		if (!tts_service_)
			return;
		tts_service_->Text2Speech(message, notification_voice_);
	}
	
} // end of namespace

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}


using namespace NotifiTts;

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(NotificationTts)
POCO_END_MANIFEST
