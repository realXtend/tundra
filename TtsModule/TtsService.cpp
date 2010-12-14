//$ HEADER_NEW_FILE $ 
#include "StableHeaders.h"

#include "TtsService.h"
#include "TtsModule.h"
#include <QSettings>
#include <QProcess>
#include "MemoryLeakCheck.h"

#define INI_FILE_PATH "data/tts.ini"

namespace Tts
{
	TtsService::TtsService(Foundation::Framework* framework) : 
        framework_(framework)
			//voice_(Voices.ES1)
    {
        InitializeVoices();
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

	void TtsService::Text2Speech(QString message, QString voice)
	{
        if (!voices_.contains(voice))
        {
            QString message = QString("Unsupported voice %1").arg(voice);
            TtsModule::LogError(message.toStdString());       
            return;
        }
        QString voice_params = voices_[voice];
        message = RemoveUnwantedCharacters(message);

        QString command = QString("festival.exe --libdir \"festival/lib\" %1 -A -T \"%2\"").arg(voice_params).arg(message); 
        QProcess* p = new QProcess(this);
        p->start(command);
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
