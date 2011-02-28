//$ HEADER_NEW_FILE $ 
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_TtsChat.h
 *  @brief  EC_TtsChat Tts voice component wich allows use Tts function on chat.
*/

#ifndef incl_TTS_EC_TtsChat_h
#define incl_TTS_EC_TtsChat_h

#include "IComponent.h"
#include "IAttribute.h"

#include "Declare_EC.h"
#include "TtsServiceInterface.h"

#include <QString>

class EC_TtsChat : public IComponent
{
	Q_OBJECT
	DECLARE_EC(EC_TtsChat);

private slots:
    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();

public:

    /// Destructor.
    ~EC_TtsChat();

	/// Return true if the component is serializable in XML
	virtual bool IsSerializable() const { return true; }

public slots:
    /// Send the message to tts service to play it
    /// @param msg Message to be shown.
	/// @param voice The name of entity that sends the message
    /// @note If there is not param voice, plays the default voice.
	void SpeakChatMessage(const QString msg, const QString name);

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_TtsChat(IModule *module);

    /// Tts pointer.
	Tts::TtsServiceInterface* ttsService_;
};

#endif
