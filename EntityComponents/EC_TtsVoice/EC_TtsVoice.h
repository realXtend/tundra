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

/// maybe EC_DynamicComponent would be enough?
class EC_TtsVoice : public IComponent
{
	Q_OBJECT
	DECLARE_EC(EC_TtsVoice);

public slots:
    void OnClick();

private slots:
    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();

public:
    /// Destructor.
    ~EC_TtsVoice();

    /// VOIP protocol used eg. 'mumble'
    Q_PROPERTY(QString voicename READ getvoicename WRITE setvoicename);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, voicename);

    /// Sets voice for the entity.
    /// @param voice Voice.
	void SetMyVoice(const QString voice);

	/// Gets the voice of entity.
	QString GetMyVoice() const;

	/// Return true if the component is serializable in XML
	virtual bool IsSerializable() const { return true; }

    /// Send the message to tts service to play it
    /// @param msg Message to be shown.
	/// @param voice The Voice to play
    /// @note If there is not param voice, plays the own voice.
	void SpeakMessage(const QString msg, QString voice);
	void SpeakMessage(const QString msg);
	void SpeakMessage();

	/// Attributes
	Attribute<QString> voice_;
	Attribute<QString> message_;

private:

    /// Constuctor.
    /// @param module Owner module.
    explicit EC_TtsVoice(IModule *module);

    /// Tts pointer.
    Tts::TtsServiceInterface* ttsService_;
};

#endif
