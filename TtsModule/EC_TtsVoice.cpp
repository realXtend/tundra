//$ HEADER_NEW_FILE $ 
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_TtsVoice.cpp
 *  @brief  EC_TtsVoice Tts voice component wich allows use Tts function on entity.
*/

#include "StableHeaders.h"

#include "EC_TtsVoice.h"
#include "IModule.h"
#include "Entity.h"
#include "input.h"
#include "TundraLogicModule.h"
#include "Client.h"
#include <QToolTip>

EC_TtsVoice::EC_TtsVoice(IModule *module) :
    IComponent(module->GetFramework()),
	ttsService_(0),
	voice(this, "voice",0)
{
	// Get TTS service
	ttsService_ = framework_->GetService<Tts::TtsServiceInterface>();
	connect(this, SIGNAL(ParentEntitySet()), this, SLOT(RegisterActions()));

	static AttributeMetadata metadata;
	AttributeMetadata::ButtonInfoList voiceButton;
	static bool metadataInitialized = false;
	if(!metadataInitialized)
	{
		voiceButton.push_back(AttributeMetadata::ButtonInfo(voice.GetName(), "?","Help"));
		metadata.buttons=voiceButton;
		metadataInitialized = true;
	}
	voice.SetMetadata(&metadata);

	connect(ttsService_,SIGNAL(SettingsUpdated()),SLOT(UpdateVoice()));
	//UpdateVoice();
}

EC_TtsVoice::~EC_TtsVoice()
{
}

void EC_TtsVoice::ListenMessage(const QString msg,const QString v)
{
	if(!ttsService_)
		ttsService_ = framework_->GetService<Tts::TtsServiceInterface>();

	ttsService_->Text2Speech(msg,v);
}

void EC_TtsVoice::SpeakMessage(const QString msg)
{
	if(!ttsService_)
		ttsService_ = framework_->GetService<Tts::TtsServiceInterface>();

	ttsService_->Text2Speech(msg,voice.Get());
}

QString EC_TtsVoice::GetMyVoice()
{
	if(!ttsService_)
		ttsService_ = framework_->GetService<Tts::TtsServiceInterface>();

	return voice.Get();
}

void EC_TtsVoice::RegisterActions()
{
    Scene::Entity *entity = GetParentEntity();
    assert(entity);
	if (entity){
        entity->ConnectAction("Speak", this, SLOT(SpeakMessage(const QString &)));
		entity->ConnectAction("Listen", this, SLOT(ListenMessage(const QString &,const QString &)));
	}
}

void EC_TtsVoice::Help(const QString v){
	QStringList voices = ttsService_->GetAvailableVoices();
	QString s = "Available voices:\n";
	for(int i=0;i<voices.length();i++)
		s=s+voices.at(i)+"\n";
	
	QToolTip::showText(framework_->GetInput()->MousePressedPos(Qt::MouseButton::LeftButton),s);
}

void EC_TtsVoice::UpdateVoice()
{
	boost::shared_ptr<TundraLogic::Client> client=framework_->GetModule<TundraLogic::TundraLogicModule>()->GetClient();
	QString name= "Avatar" + QString::number(client->GetConnectionID());
	if(GetParentEntity()->GetName()==name){
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/Tts");
		setvoice(settings.value("Tts/own_voice", "").toString());
	}
}