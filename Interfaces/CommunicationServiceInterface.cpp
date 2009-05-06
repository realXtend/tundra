#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"

#include "CommunicationServiceInterface.h"

// todo: Move this implementation to Communication module

namespace Communication
{
	typedef std::map<std::string, std::string>::const_iterator property_iterator;

	void ContactInfo::SetProperty(std::string key, std::string value)
	{
		properties_.insert( std::pair<std::string, std::string>(key, value) );
	}

	std::string ContactInfo::GetProperty(std::string key)
	{
		property_iterator i;
		i = properties_.find(key);
		if (i==properties_.end())
			return "";
		else
			return i->second;
	}

	/*
	returns all property keys
	*/
	std::vector<std::string> ContactInfo::GetProperties()
	{
		std::vector<std::string> keys;
		for (property_iterator i = properties_.begin(); i != properties_.end(); i++)
		{
			keys.push_back(i->first);
		}
		return keys;
	}

	void Credentials::SetProperty(std::string key, std::string value)
	{
		properties_.insert( std::pair<std::string, std::string>(key, value) );
	}

	std::string Credentials::GetProperty(std::string key)
	{
		typedef std::map<std::string, std::string>::const_iterator property_iterator;
		property_iterator i;
		i = properties_.find(key);
		if (i==properties_.end())
			return "";
		else
			return i->second;
	}

	/*
	returns all property keys
	*/
	std::vector<std::string> Credentials::GetProperties()
	{
		std::vector<std::string> keys;
		for (property_iterator i = properties_.begin(); i != properties_.end(); i++)
		{
			keys.push_back(i->first);
		}
		return keys;
	}


} // end of namespace: Communication
