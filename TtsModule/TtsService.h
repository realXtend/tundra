//$ HEADER_NEW_FILE $ 
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Tts_TtsService_h
#define incl_Tts_TtsService_h

#include "Framework.h"

#include <QProcess>
#include <QMap>
#include <QPair>
#include <queue>
#include <deque>
#include <time.h>

namespace Tts
{
	using std::queue;
	using std::deque;
	typedef QPair<QString, clock_t> ttsPair;
	
	class TtsService : public QObject
	{
		Q_OBJECT

    public:

		// Constructor
		TtsService(Foundation::Framework* framework_);

        //! Destructor
        virtual ~TtsService();
        
	public slots:
        /** Synthetizes text to speech
		\param message Text to synthetize
        \param voice Voice database. @see GetAvailableVoices
        \param type 1 overlap speech(Chat), 2 exclusive speech(NPCs)
        \return tts process pid*/
		qulonglong Text2Speech(QString message, QString voice, int type = 1);

		/** Synthetizes text to WAV
		\param message Text to synthetize.
		\param pathAndFileName The path and file name where the WAV is saved.
		\param voice Voice database. @see GetAvailableVoices*/
        void Text2WAV(QString message, QString pathAndFileName, QString voice);
        
        /** Synthetizes text to PHO
		\param message Text to synthetize.
		\param pathAndFileName The path and file name where the phonetic information (PHO) is saved.
		\param voice Voice database. @see GetAvailableVoices*/
		void Text2PHO(QString message, QString pathAndFileName, QString voice);

        /** Synthetizes a text file to speech
		\param pathAndFileName Input text file path, to synthetize.
		\param voice Voice database. @see GetAvailableVoices*/
		void File2Speech(QString pathAndFileName, QString voice);
        
        /** Synthetizes a text file to WAV
		\param pathAndFileNameIn Input text file path, to synthetize.
        \param pathAndFileNameOut The path and file name where the WAV is saved.
		\param voice Voice database. @see GetAvailableVoices*/
        void File2WAV(QString pathAndFileNameIn, QString pathAndFileNameOut, QString voice);

        /** Synthetizes a text file to PHO
		\param pathAndFileNameIn Input text file path, to synthetize.
        \param pathAndFileNameOut The path and file name where the PHO is saved.
		\param voice Voice database. @see GetAvailableVoices*/
		void File2PHO(QString pathAndFileNameIn, QString pathAndFileNameOut, QString voice);

        /**
         \return Available voice options.
         \see Text2Speech*/
        QStringList GetAvailableVoices() const;

        void TriggerSettingsUpdated();

		void TtsAvailable();

		void HandleProcessFinished();

    signals:
        /** Emited when settings are changed
            @see TriggerSettingsUpdated*/
        void SettingsUpdated();

		/** Emited when tts process is terminated
        \param processPid tts process pid*/            
        void ProcessFinished(qulonglong processPid);

	private:
		
		void InitializeVoices();		
		QString RemoveUnwantedCharacters(QString text);

		//! Framework we belong to
		Foundation::Framework* framework_;
		QMap<QString, QString> voices_;

		//Voice voice_;

		//queue	of voices
		bool tts_available_;
		queue<ttsPair> queue_tts_;
		deque<ttsPair> deque_tts_;
	};
}
#endif