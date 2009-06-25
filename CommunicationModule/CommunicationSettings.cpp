#include "StableHeaders.h"
#include "Foundation.h"


#include "CommunicationSettings.h"




namespace Communication
{

    typedef std::map<std::string, std::string>::const_iterator property_iterator;

    CommunicationSettings::CommunicationSettings(CommunicationManager* cmanager): Communication::CommunicationSettingsInterface(), cmanager_(cmanager)
    {
    }

    CommunicationSettings::~CommunicationSettings(void)
    {
    }

	void CommunicationSettings::SetProperty(const std::string& key, const std::string& value)
	{
        //using [] here instead of insert, because [] assign value to map even if key already exists, so that old values can be updated
		//properties_.insert( std::pair<std::string, std::string>(key, value) );
        properties_[key]=value;
	}


    std::string CommunicationSettings::GetProperty(const std::string &key)
    {
		typedef std::map<std::string, std::string>::const_iterator property_iterator;
		property_iterator i;
		i = properties_.find(key);
		if (i==properties_.end())
			return "";
		else
			return i->second;
    }

	std::vector<std::string> CommunicationSettings::GetProperties()
    {
		std::vector<std::string> keys;
		for (property_iterator i = properties_.begin(); i != properties_.end(); i++)
		{
			keys.push_back(i->first);
		}
		return keys;
    }

    void CommunicationSettings::Load()
    {
        Foundation::ScriptObject* sobj = cmanager_->CallPythonCommunicationObjectAndGetReturnValue("CGetSettings");
        char* retVal = sobj->ConvertToChar();
		if (retVal == NULL)
			return; 
        std::string settingsString(retVal);
        std::vector<std::string> settingsParamsPairsVector = this->SplitString(settingsString, std::string(":"), 0);
        
        for(std::vector<std::string>::iterator iter = settingsParamsPairsVector.begin(); iter<settingsParamsPairsVector.end()-1; iter+=2){
		    std::string key   = *(iter);
            std::string value = *(iter+1);
            properties_[key]=value;
        }
        // free object with python
        sobj->ReleaseResources();
    }

    void CommunicationSettings::Save()
    {
        std::string propertiesSaveString = "";
	    for (property_iterator i = properties_.begin(); i != properties_.end(); i++)
	    {
            std::string key = i->first;
            std::string value = i->second;
		    propertiesSaveString.append(key);
            propertiesSaveString.append(":");
            propertiesSaveString.append(value);
            propertiesSaveString.append(":");
	    }
        // remove the last ":"
        propertiesSaveString = propertiesSaveString.substr(0, propertiesSaveString.size()-1);
        Foundation::ScriptObject* sobj = cmanager_->CallPythonCommunicationObjectAndGetReturnValue("CSaveSettings",propertiesSaveString);
        sobj->ReleaseResources();
    }

    CommunicationSettings* CommunicationSettings::GetSettings(CommunicationManager* cmanager)
    {
        return new CommunicationSettings(cmanager);
    }

	/**
	 *  Split string, splits given string with given separator and uses splitAmount parameter for amount of splits done
     *  is splitAmount is zero then every occurrence of separator is used to make splits
     *  //\bug move this method along with CommunicationManager GetSplitString to some utility class static method
	 */
    std::vector<std::string> CommunicationSettings::SplitString(const std::string &inString, const std::string &separator, const int &splitAmount)
    {
        std::vector<std::string> returnVector;
        std::string::size_type start = 0;
        std::string::size_type end = 0;
        int round = 0;
        if(splitAmount==0)
        {
            while ((end=inString.find (separator, start)) != std::string::npos)
            {
                returnVector.push_back (inString.substr (start, end-start));
                start = end+separator.size();
            }        
        }
        else
        {
            while ((end=inString.find (separator, start)) != std::string::npos && round < splitAmount)
            {
                returnVector.push_back (inString.substr (start, end-start));
                start = end+separator.size();
                round++;
            }
        }
        returnVector.push_back (inString.substr (start));
        return returnVector;
    } 

}
