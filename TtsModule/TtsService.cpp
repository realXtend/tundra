//$ HEADER_NEW_FILE $ 
#include "StableHeaders.h"

#include "TtsService.h"
#include "TtsModule.h"

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
	    //static const AvailableVoice Voices = {"-ES1","-ES2","-EN1","-EN2","-EN3","-EN4","-EN5","-EN6","-CAT1","-CAT2","-FI"};
        voices_["Finnish male"] = "-FI";
        voices_["English male 1"] = "-EN1";
        voices_["English male 2"] = "-EN3";
        voices_["English male 3"] = "-EN5";
        voices_["English female 1"] = "-EN2";
        voices_["English female 2"] = "-EN4";
        voices_["English female 3"] = "-EN6";
        voices_["Catalan male"] = "-CAT1";
        voices_["Catalan female"] = "-CAT2";
        voices_["Spanish female"] = "-ES1";
        voices_["Spanich male"] = "-ES2";
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

		std::stringstream commandoss;
		std::string commandos,msg;
		commandoss << "start /B festival.exe --libdir \"festival/lib\" "; 
		
        commandoss << voice_params.toStdString();

		commandoss << " -A -T \"";

		msg = message.toStdString();
		std::replace_if(msg.begin(),msg.end(),boost::is_any_of("\""),', ');
		commandoss << msg;
		commandoss << "\"";
		commandos = commandoss.str();

        /// @todo Use Qt
		system(commandos.c_str());	
	}

	void TtsService::Text2WAV(QString message, QString pathAndFileName, QString voice)
	{
		std::string msg;
		msg=message.toStdString();

		std::stringstream commandoss;
		std::string commandos;
		commandoss << "start /B festival.exe --libdir \"festival/lib\" "; 
        commandoss << voice.toStdString();
		commandoss << " -W ";
		commandoss << pathAndFileName.toStdString();
		commandoss << " -T \"";

		std::replace_if(msg.begin(),msg.end(),boost::is_any_of("{}\""),', ');
		commandoss << msg;
		commandoss << "\"";
		commandos = commandoss.str();

		system(commandos.c_str());	
	}

	void TtsService::Text2PHO(QString message, QString pathAndFileName, QString voice)
	{
		std::string msg;
		msg=message.toStdString();

		std::stringstream commandoss;
		std::string commandos;
		commandoss << "start /B festival.exe --libdir \"festival/lib\" "; 
        commandoss << voice.toStdString();
		commandoss << " -P ";
		commandoss << pathAndFileName.toStdString();
		commandoss << " -T \"";

		std::replace_if(msg.begin(),msg.end(),boost::is_any_of("\""),', ');
		commandoss << msg;
		commandoss << "\"";
		commandos = commandoss.str();

		system(commandos.c_str());	
	}
	
	void TtsService::File2Speech(QString pathAndFileName, QString voice)
	{
		std::stringstream commandoss;
		std::string commandos,file;
		commandoss << "start /B festival.exe --libdir \"festival/lib\" "; 

		
        commandoss << voice.toStdString();

		commandoss << " -A -F \"";

		file=pathAndFileName.toStdString();

		commandoss << file;
		commandoss << "\"";
		commandos = commandoss.str();

		system(commandos.c_str());	
	}
	void TtsService::File2WAV(QString pathAndFileNameIn, QString pathAndFileNameOut, QString voice)
	{
		std::string fileIn;
		fileIn=pathAndFileNameIn.toStdString();

		std::stringstream commandoss;
		std::string commandos;
		commandoss << "start /B festival.exe --libdir \"festival/lib\" "; 
        commandoss << voice.toStdString();
		commandoss << " -W ";
		commandoss << pathAndFileNameOut.toStdString();
		commandoss << " -F \"";

		commandoss << fileIn;
		commandoss << "\"";
		commandos = commandoss.str();

		system(commandos.c_str());	
	}

	void TtsService::File2PHO(QString pathAndFileNameIn, QString pathAndFileNameOut, QString voice)
	{
		std::string fileIn;
		fileIn=pathAndFileNameIn.toStdString();

		std::stringstream commandoss;
		std::string commandos;
		commandoss << "start /B festival.exe --libdir \"festival/lib\" "; 
        commandoss << voice.toStdString();
		commandoss << " -P ";
		commandoss << pathAndFileNameOut.toStdString();
		commandoss << " -F \"";

		commandoss << fileIn;
		commandoss << "\"";
		commandos = commandoss.str();

		system(commandos.c_str());	
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
