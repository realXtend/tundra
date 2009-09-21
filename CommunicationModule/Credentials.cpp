#include "Credentials.h"

namespace TpQt4Communication
{
	Credentials::Credentials()
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

	

} // end of namespace: TpQt4Communication
