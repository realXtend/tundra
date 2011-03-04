//$ HEADER_NEW_FILE $ 
/**
*  TTS Service
*    
*   This service can be used for synthesis purposes being possible inputs both raw text and file texts.
*	Output can be audio, WAV or PHO (phonetic information) and available methods are:
*
*  - Text2Speech
*  - Text2WAV
*  - Text2PHO
*  - File2Speech
*  - File2WAV
*  - File2PHO
*
*
*/

#ifndef incl_Interfaces_TtsServiceInterface_h
#define incl_Interfaces_TtsServiceInterface_h

#include "IService.h"

#include <QObject>
#include <Vector3D.h>
#include <QList>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QDateTime>


namespace Tts
{
	/// TTS Service Interface
	/// Available methods are Text or File to Speech, WAV or PHO.
	/// \todo Implement GetVoice GetVoice throw components
	class TtsServiceInterface : public QObject, public IService 
	{
		Q_OBJECT

    public:
        
	    virtual ~TtsServiceInterface() {}

	public slots:

        /** Synthetizes text to speech
		\param message Text to synthetize
        \param voice Voice database. @see GetAvailableVoices
        \param type 1 overlap speech(Chat), 2 exclusive speech(NPCs)
        \return tts process pid*/
		virtual qulonglong Text2Speech(QString message, QString voice, int type = 1) = 0;
		/** Synthetizes text to WAV
		\param message Text to synthetize.
		\param pathAndFileName The path and file name where the WAV is saved.
		\param voice Voice database. @see GetAvailableVoices*/
        virtual void Text2WAV(QString message, QString pathAndFileName, QString voice) = 0;
        /** Synthetizes text to PHO
		\param message Text to synthetize.
		\param pathAndFileName The path and file name where the phonetic information (PHO) is saved.
		\param voice Voice database. @see GetAvailableVoices*/
		virtual void Text2PHO(QString message, QString pathAndFileName, QString voice) = 0;

        /** Synthetizes a text file to speech
		\param pathAndFileName Input text file path, to synthetize.
		\param voice Voice database. @see GetAvailableVoices*/
		virtual void File2Speech(QString pathAndFileName, QString voice) = 0;
        /** Synthetizes a text file to WAV
		\param pathAndFileNameIn Input text file path, to synthetize.
        \param pathAndFileNameOut The path and file name where the WAV is saved.
		\param voice Voice database. @see GetAvailableVoices*/
        virtual void File2WAV(QString pathAndFileNameIn, QString pathAndFileNameOut, QString voice) = 0;
        /** Synthetizes a text file to PHO
		\param pathAndFileNameIn Input text file path, to synthetize.
        \param pathAndFileNameOut The path and file name where the PHO is saved.
		\param voice Voice database. @see GetAvailableVoices*/
		virtual void File2PHO(QString pathAndFileNameIn, QString pathAndFileNameOut, QString voice) = 0;

        /**
         \return Available voice options.
         \see Text2Speech*/
        virtual QStringList GetAvailableVoices() const = 0;

        virtual void TriggerSettingsUpdated() = 0;

    signals:
        /** Emited when settings are changed
            @see TriggerSettingsUpdated*/
        void SettingsUpdated();
		/** Emited when tts process is terminated
        \param processPid tts process pid*/            
        void ProcessFinished(qulonglong processPid);
	};

   	typedef boost::shared_ptr<TtsServiceInterface> TtsServicePtr;

}

#endif