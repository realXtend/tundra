#ifndef incl_CommunicationSettings_h
#define incl_CommunicationSettings_h

#include "CommunicationManager.h"

/**
 *  CommunicationSettings class
 *
 *  Implement CommunicationSettingsInterface
 */
namespace Communication
{
    class CommunicationSettings : public CommunicationSettingsInterface
    {
    public:
        CommunicationSettings(CommunicationManager* cmanager);
        virtual ~CommunicationSettings(void);

        
        virtual void SetProperty(const std::string& key, const std::string& value);
        
		virtual std::string GetProperty(const std::string &key);
		virtual std::vector<std::string> GetProperties();
        virtual void Load();
        virtual void Save();
        static CommunicationSettings* GetSettings(CommunicationManager* cmanager);
        static std::vector<std::string> SplitString(const std::string &inString, const std::string &separator, const int &splitAmount);
	protected:
		std::map<std::string, std::string> properties_;
        CommunicationManager* cmanager_;
    };
}
#endif