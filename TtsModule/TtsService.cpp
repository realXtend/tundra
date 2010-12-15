//$ HEADER_NEW_FILE $ 
#include "StableHeaders.h"

#include "TtsService.h"


namespace Tts
{
	TtsService::TtsService(Foundation::Framework* framework) : 
        framework_(framework)
			//voice_(Voices.ES1)
    {

    }
		
    TtsService::~TtsService()
    {

    }
    

	void TtsService::Text2Speech(QString message, Voice voice)
	{

		std::stringstream commandoss;
		std::string commandos,msg;
		commandoss << "start /B festival.exe --libdir \"festival/lib\" "; 
		
		commandoss << voice;

		commandoss << " -A -T \"";

		msg=message.toStdString();
		std::replace_if(msg.begin(),msg.end(),boost::is_any_of("\""),', ');
		commandoss << msg;
		commandoss << "\"";
		commandos = commandoss.str();

		system(commandos.c_str());	
	}

	void TtsService::Text2WAV(QString message, QString pathAndFileName, Voice voice)
	{
		std::string msg;
		msg=message.toStdString();

		std::stringstream commandoss;
		std::string commandos;
		commandoss << "start /B festival.exe --libdir \"festival/lib\" "; 
		commandoss << voice;
		commandoss << " -W ";
		commandoss << pathAndFileName.toStdString();
		commandoss << " -T \"";

		std::replace_if(msg.begin(),msg.end(),boost::is_any_of("{}\""),', ');
		commandoss << msg;
		commandoss << "\"";
		commandos = commandoss.str();

		system(commandos.c_str());	
	}

	void TtsService::Text2PHO(QString message, QString pathAndFileName, Voice voice)
	{
		std::string msg;
		msg=message.toStdString();

		std::stringstream commandoss;
		std::string commandos;
		commandoss << "start /B festival.exe --libdir \"festival/lib\" "; 
		commandoss << voice;
		commandoss << " -P ";
		commandoss << pathAndFileName.toStdString();
		commandoss << " -T \"";

		std::replace_if(msg.begin(),msg.end(),boost::is_any_of("\""),', ');
		commandoss << msg;
		commandoss << "\"";
		commandos = commandoss.str();

		system(commandos.c_str());	
	}
	
	void TtsService::File2Speech(QString pathAndFileName, Voice voice)
	{
		std::stringstream commandoss;
		std::string commandos,file;
		commandoss << "start /B festival.exe --libdir \"festival/lib\" "; 

		
		commandoss << voice;

		commandoss << " -A -F \"";

		file=pathAndFileName.toStdString();

		commandoss << file;
		commandoss << "\"";
		commandos = commandoss.str();

		system(commandos.c_str());	
	}
	void TtsService::File2WAV(QString pathAndFileNameIn, QString pathAndFileNameOut, Voice voice)
	{
		std::string fileIn;
		fileIn=pathAndFileNameIn.toStdString();

		std::stringstream commandoss;
		std::string commandos;
		commandoss << "start /B festival.exe --libdir \"festival/lib\" "; 
		commandoss << voice;
		commandoss << " -W ";
		commandoss << pathAndFileNameOut.toStdString();
		commandoss << " -F \"";

		commandoss << fileIn;
		commandoss << "\"";
		commandos = commandoss.str();

		system(commandos.c_str());	
	}

	void TtsService::File2PHO(QString pathAndFileNameIn, QString pathAndFileNameOut, Voice voice)
	{
		std::string fileIn;
		fileIn=pathAndFileNameIn.toStdString();

		std::stringstream commandoss;
		std::string commandos;
		commandoss << "start /B festival.exe --libdir \"festival/lib\" "; 
		commandoss << voice;
		commandoss << " -P ";
		commandoss << pathAndFileNameOut.toStdString();
		commandoss << " -F \"";

		commandoss << fileIn;
		commandoss << "\"";
		commandos = commandoss.str();

		system(commandos.c_str());	
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
