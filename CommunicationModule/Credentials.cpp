#include "Credentials.h"

namespace TpQt4Communication
{
	Credentials::Credentials()
	{
	}

	void Credentials::SetProtocol(std::string protocol)
	{

	}

	void Credentials::SetUserID(std::string user_id)
	{

	}

	void Credentials::SetPassword(std::string pwd)
	{

	}

	void Credentials::SetServer(std::string server)
	{

	}

	void Credentials::LoadFromFile(std::string path)
	{
		// Not implemented
	}

	std::string Credentials::GetProtocol() const 
	{
		return protocol_;
	}

	std::string Credentials::GetServer() const
	{
		return server_;
	}

	
	std::string Credentials::GetUserID() const
	{
		return user_id_;
	}

	std::string Credentials::GetUserPassword() const
	{
		return password_;
	}



} // end of namespace: TpQt4Communication
