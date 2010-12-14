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
#include "EC_DynamicComponent.h"

#include "UiServiceInterface.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "CoreStringUtils.h"
#include "TtsModule.h"
#include "EC_TtsVoice.h"
#include "SettingsWidget.h"

#include "MemoryLeakCheck.h"

namespace Tts
{
	const std::string TtsModule::module_name_ = std::string("TtsModule");

	TtsModule::TtsModule() :
	    QObject(),
        IModule(module_name_),
        settings_widget_(0),
        own_avatar_voice_(""),
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
        own_avatar_voice_ = settings.value("Tts/own_voice", "").toString();
    }

	void TtsModule::PostInitialize()
	{
        SetupSettingsWidget();
        connect(framework_, SIGNAL(DefaultWorldSceneChanged(const Scene::ScenePtr &)), this, SLOT(ConnectSceneSignals()));
    }

    void TtsModule::ConnectSceneSignals()
    {
        Scene::SceneManager* scene = framework_->DefaultScene();
        if (!scene)
            return;

        connect(scene, SIGNAL( ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type) ), this, SLOT(CheckNewComponent(Scene::Entity*, IComponent*, AttributeChange::Type)));
    }

    void TtsModule::CheckNewComponent(Scene::Entity* ent, IComponent* comp, AttributeChange::Type change_type)
    {
        if (!publish_own_voice_)
            return;

        Foundation::WorldLogicInterface* world_logic = framework_->GetService<Foundation::WorldLogicInterface>();
        if (!world_logic)
            return;
        
        Scene::EntityPtr user = world_logic->GetUserAvatarEntity();
        if (!user)
            return;

        IComponent* component = user->GetOrCreateComponent("EC_DynamicComponent","EC_TtsVoice", AttributeChange::Replicate, true).get();
        EC_DynamicComponent* tts_voice =  dynamic_cast<EC_DynamicComponent*>(component);
        if (!tts_voice)
            return;

        if (!tts_voice->ContainsAttribute("avatar voice"))
            tts_voice->AddQVariantAttribute("avatar voice", AttributeChange::Replicate);
        if (tts_voice->GetAttribute("avatar voice").toString() != own_avatar_voice_)
        {
            tts_voice->SetAttribute("avatar voice", own_avatar_voice_);
            QString message = QString("Added tts voice for own avatar: %1").arg(own_avatar_voice_);
            LogInfo(message.toStdString());
        }
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
