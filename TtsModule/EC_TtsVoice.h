//$ HEADER_NEW_FILE $ 
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_TtsVoice.h
 *  @brief  EC_TtsVoice Tts voice component wich allows use Tts function on entity.
*/

#ifndef incl_TTS_EC_TtsVoice_h
#define incl_TTS_EC_TtsVoice_h

#include "IComponent.h"
#include "IAttribute.h"


#include "Declare_EC.h"
#include "TtsServiceInterface.h"


#include <QString>

class EC_TtsVoice : public IComponent
{
	Q_OBJECT
	DECLARE_EC(EC_TtsVoice);

public:

    /// Name of the voice used for this EC.
	Q_PROPERTY(QString voice READ getvoice WRITE setvoice);
	DEFINE_QPROPERTY_ATTRIBUTE(QString, voice);

    /// Destructor.
    ~EC_TtsVoice();

	/// Return true if the component is serializable in XML
	virtual bool IsSerializable() const { return true; }

public slots:
    /// Send the message to tts service to play it
    /// @param msg Message to be shown.
	/// @param voice The Voice to play
    /// @note If there is not param voice, plays the default voice.
	void ListenMessage(const QString msg, const QString voice);
    /// Send the message to tts service to play it
    /// @param msg Message to be shown.
	void SpeakMessage(const QString msg);
    /// Add a tooltip to the help button in EC Editor
	/// @param v The voice of component
	void Help(const QString v);
	/// Return my voice
	QString GetMyVoice();

private slots:
	// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();

	/// Update the avatar voice when it is changed in the Settings panel
	void UpdateVoice();

private:

    /// Constuctor.
    /// @param module Owner module.
    explicit EC_TtsVoice(IModule *module);

    /// Tts pointer.
	Tts::TtsServiceInterface* ttsService_;
};

#endif
