//$ HEADER_NEW_FILE $ 
#include "StableHeaders.h"

#include "TtsService.h"
#include "TtsModule.h"
#include <QSettings>
#include "MemoryLeakCheck.h"
#include <time.h>


#define INI_FILE_PATH "data/tts.ini"
const int MAX_WAIT_TTS = 10; //10s

namespace Tts
{
	TtsService::TtsService(Foundation::Framework* framework) : 
        framework_(framework),
		deque_tts_()
			//voice_(Voices.ES1)
    {
        InitializeVoices();		
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/Tts");
		tts_available_ = true;
		//$ END_MOD $
    }
		
    TtsService::~TtsService()
    {

    }
    
    void TtsService::InitializeVoices()
    {
        QSettings voice_settings(INI_FILE_PATH, QSettings::IniFormat);
        voice_settings.beginGroup("Voices");
        QStringList voices = voice_settings.allKeys();
        foreach(QString v, voices)
        {
            QString params = voice_settings.value(v).toString();
            voices_[v] = params;

            QString message = QString("Load TTS option: %1 = %2").arg(v).arg(params);
            TtsModule::LogDebug(message.toStdString());
        }
        voice_settings.endGroup();
        return;
    }

	qulonglong TtsService::Text2Speech(QString message, QString voice, int type)
	{
		QString voice_params=0;
		if (!voices_.contains(voice))
		{
			QString message = QString("Unsupported voice %1, using the default voice").arg(voice);
			TtsModule::LogError(message.toStdString());
			QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/Tts");
			QString default_tts_voice = settings.value("Tts/other_default_voice", "").toString();
			voice_params = voices_[default_tts_voice];
		}else
			voice_params = voices_[voice];

		message = RemoveUnwantedCharacters(message);

		QString command = QString("festival.exe --libdir \"festival/lib\" %1 -A -T \"%2\"").arg(voice_params).arg(message);
				
		//Avoid tts overlap
		//check priority
		QProcess* p = new QProcess;
		if (type == 1)
		{
			//Put in front without time
			//Chat = overlapped
			p->start(command);
		}
		if (type == 2)
		{
			//Put in front without time
			if (tts_available_) 
			{
				p->start(command);
				tts_available_=false;
				connect(p, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(TtsAvailable()));
			}
		}
		p->setProperty("PID", (qulonglong)p->pid());
		connect(p, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(HandleProcessFinished()));

		return (qulonglong)p->pid();
	}

	void TtsService::TtsAvailable() {
		tts_available_=true;
	}

	void TtsService::HandleProcessFinished()
	{
		QProcess *process = qobject_cast<QProcess*>(sender());
		if (process == 0)
			return;
		emit ProcessFinished(process->property("PID").toULongLong());
	}

	void TtsService::Text2WAV(QString message, QString pathAndFileName, QString voice)
	{
        if (!voices_.contains(voice))
        {
            QString message = QString("Unsupported voice %1").arg(voice);
            TtsModule::LogError(message.toStdString());       
            return;
        }
        QString voice_params = voices_[voice];
        message = RemoveUnwantedCharacters(message);

        QString command = QString("festival.exe --libdir \"festival/lib\" %1 -W %2 -T \"%3\"").arg(voice_params).arg(pathAndFileName).arg(message); 
        QProcess* p = new QProcess(this);
        p->start(command);
	}

	void TtsService::Text2PHO(QString message, QString pathAndFileName, QString voice)
	{
        if (!voices_.contains(voice))
        {
            QString message = QString("Unsupported voice %1").arg(voice);
            TtsModule::LogError(message.toStdString());       
            return;
        }
        QString voice_params = voices_[voice];
        message = RemoveUnwantedCharacters(message);

        QString command = QString("festival.exe --libdir \"festival/lib\" %1 -P %2 -T \"%3\"").arg(voice_params).arg(pathAndFileName).arg(message); 
        QProcess* p = new QProcess(this);
        p->start(command);
	}
	
	void TtsService::File2Speech(QString pathAndFileName, QString voice)
	{
        if (!voices_.contains(voice))
        {
            QString message = QString("Unsupported voice %1").arg(voice);
            TtsModule::LogError(message.toStdString());       
            return;
        }
        QString voice_params = voices_[voice];

        QString command = QString("festival.exe --libdir \"festival/lib\" %1 -A -F \"%2\"").arg(voice_params).arg(pathAndFileName); 
        QProcess* p = new QProcess(this);
        p->start(command);
	}

	void TtsService::File2WAV(QString pathAndFileNameIn, QString pathAndFileNameOut, QString voice)
	{
        if (!voices_.contains(voice))
        {
            QString message = QString("Unsupported voice %1").arg(voice);
            TtsModule::LogError(message.toStdString());       
            return;
        }
        QString voice_params = voices_[voice];

        QString command = QString("festival.exe --libdir \"festival/lib\" %1 -W %2 -F\"%3\"").arg(voice_params).arg(pathAndFileNameOut).arg(pathAndFileNameIn); 
        QProcess* p = new QProcess(this);
        p->start(command);
	}

	void TtsService::File2PHO(QString pathAndFileNameIn, QString pathAndFileNameOut, QString voice)
	{
        if (!voices_.contains(voice))
        {
            QString message = QString("Unsupported voice %1").arg(voice);
            TtsModule::LogError(message.toStdString());       
            return;
        }
        QString voice_params = voices_[voice];

        QString command = QString("festival.exe --libdir \"festival/lib\" %1 -P %2 -F\"%3\"").arg(voice_params).arg(pathAndFileNameOut).arg(pathAndFileNameIn); 
        QProcess* p = new QProcess(this);
        p->start(command);
	}

    QString TtsService::RemoveUnwantedCharacters(QString text)
    {
        text.replace("\"", " ");
        text.replace("{", ", ");
        text.replace("}", ", ");
        return text;
    }

    QStringList TtsService::GetAvailableVoices() const
    {
        return voices_.keys();
    }

    void TtsService::TriggerSettingsUpdated()
    {
        emit Tts::TtsServiceInterface::SettingsUpdated();
    }

	/*const Voice TtsService::GetVoice()
	{
		return voice_;
	}

	void TtsService::SetVoice(Voice voice)
	{
		voice_=voice;
	}*/
}
