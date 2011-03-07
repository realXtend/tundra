//$ HEADER_NEW_FILE $ 
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_TtsVoice.cpp
 *  @brief  EC_TtsVoice Tts voice component wich allows use Tts function on entity.
*/

#include "StableHeaders.h"

#include "EC_TtsChat.h"
#include "EC_TtsVoice.h"
#include "IModule.h"
#include "Entity.h"
#include "TundraLogicModule.h"
#include "SceneManager.h"
#include "Client.h"

EC_TtsChat::EC_TtsChat(IModule *module) :
    IComponent(module->GetFramework()),
	ttsService_(0)
{
	// Get TTS service
	ttsService_ = framework_->GetService<Tts::TtsServiceInterface>();
	connect(this, SIGNAL(ParentEntitySet()), this, SLOT(RegisterActions()));
	//connect(ttsService_,SIGNAL(SettingsUpdated()),SLOT(UpdateChatSettings()));
}

EC_TtsChat::~EC_TtsChat()
{
}

void EC_TtsChat::SpeakChatMessage(const QString msg, const QString name)
{
	if(!ttsService_)
		ttsService_ = framework_->GetService<Tts::TtsServiceInterface>();

	/// @todo signal when settings are changed so we do not have to read each time we have a new message to play
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/Tts");
	bool tts_own_messages_ = settings.value("Tts/play_own_chat_messages", false).toBool();
	bool tts_other_messages_ = settings.value("Tts/play_other_chat_messages", false).toBool();
	QString default_avatar_tts_voice_ = settings.value("Tts/other_default_voice", "").toString();

	boost::shared_ptr<TundraLogic::Client> client=framework_->GetModule<TundraLogic::TundraLogicModule>()->GetClient();
	QString nameAvatar= "Avatar" + QString::number(client->GetConnectionID());
	Scene::Entity* entity=this->GetParentEntity()->GetScene()->GetEntityByNameRaw(name);

	if(name==nameAvatar){
		if(tts_own_messages_){
			EC_TtsVoice* voice = entity->GetComponent<EC_TtsVoice>().get();
			if(voice)
				ttsService_->Text2Speech(msg, voice->GetMyVoice());
			else
				ttsService_->Text2Speech(msg, default_avatar_tts_voice_);
		}
	}else{
		if (tts_other_messages_){
			EC_TtsVoice* voice = entity->GetComponent<EC_TtsVoice>().get();
			if(voice){
				ttsService_->Text2Speech(msg, voice->GetMyVoice());
			}else{
				ttsService_->Text2Speech(msg, default_avatar_tts_voice_);
			}
		}
	}
}

void EC_TtsChat::RegisterActions()
{
    Scene::Entity *entity = GetParentEntity();
    assert(entity);
	if (entity)
        entity->ConnectAction("SpeakChat", this, SLOT(SpeakChatMessage(const QString &,const QString &)));
}