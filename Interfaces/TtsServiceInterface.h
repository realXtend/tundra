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
#include <QDateTime>


namespace Tts
{

	typedef std::string Voice;
    /// Available voice list. \note Every new language must be added here and in the Festival library folder. Also Festival.exe must be updated.
	struct AvailableVoice
	{
		Voice ES1;
		Voice ES2;
		Voice EN1;
		Voice EN2;
		Voice EN3;
		Voice EN4;
		Voice EN5;
		Voice EN6;
		Voice CAT1;
		Voice CAT2;
		Voice FI;
	};

    ///Voice tag values
	static const AvailableVoice Voices = {"-ES1","-ES2","-EN1","-EN2","-EN3","-EN4","-EN5","-EN6","-CAT1","-CAT2","-FI"};

	/// TTS Service Interface
	/// Available methods are Text or File to Speech, WAV or PHO.
	/// \todo Implement GetVoice GetVoice throw components
	class TtsServiceInterface : public QObject, public IService 
	{
		Q_OBJECT

    public:
        
	    virtual ~TtsServiceInterface() {}
        /** Synthetizes text to speech
		\param message Text to synthetize.
		\param voice Voice database, must be one from AvailableVoices. */
		virtual void Text2Speech(QString message, Voice voice) = 0;
		/** Synthetizes text to WAV
		\param message Text to synthetize.
		\param pathAndFileName The path and file name where the WAV is saved.
		\param voice Voice database, must be one from AvailableVoices. */
        virtual void Text2WAV(QString message, QString pathAndFileName, Voice voice) = 0;
        /** Synthetizes text to PHO
		\param message Text to synthetize.
		\param pathAndFileName The path and file name where the phonetic information (PHO) is saved.
		\param voice Voice database, must be one from AvailableVoices. */
		virtual void Text2PHO(QString message, QString pathAndFileName, Voice voice) = 0;

        /** Synthetizes a text file to speech
		\param pathAndFileName Input text file path, to synthetize.
		\param voice Voice database, must be one from AvailableVoices. */
		virtual void File2Speech(QString pathAndFileName, Voice voice) = 0;
        /** Synthetizes a text file to WAV
		\param pathAndFileNameIn Input text file path, to synthetize.
        \param pathAndFileNameOut The path and file name where the WAV is saved.
		\param voice Voice database, must be one from AvailableVoices. */
        virtual void File2WAV(QString pathAndFileNameIn, QString pathAndFileNameOut, Voice voice) = 0;
        /** Synthetizes a text file to PHO
		\param pathAndFileNameIn Input text file path, to synthetize.
        \param pathAndFileNameOut The path and file name where the PHO is saved.
		\param voice Voice database, must be one from AvailableVoices. */
		virtual void File2PHO(QString pathAndFileNameIn, QString pathAndFileNameOut, Voice voice) = 0;



		//virtual const Voice GetVoice() = 0;
		//virtual void SetVoice(Voice voice) = 0;
	};

   	typedef boost::shared_ptr<TtsServiceInterface> TtsServicePtr;

}

#endif
 