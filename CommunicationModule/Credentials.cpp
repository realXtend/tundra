#include "Credentials.h"

namespace TpQt4Communication
{
	Credentials::Credentials()
	{
	}

	void Credentials::SetProtocol(std::string protocol)
	{
		protocol_ = protocol;
	}

	void Credentials::SetUserID(std::string user_id)
	{
		user_id_ = user_id;
	}

	void Credentials::SetPassword(std::string password)
	{
		password_ = password;
	}

	void Credentials::SetServer(std::string server)
	{
		server_ = server;
	}

	void Credentials::LoadFromFile(std::string path)
	{
		// Not implemented
	}

	void Credentials::SetServerPort(int port)
	{
		port_ = port;
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

	int Credentials::GetServerPort() const
	{
		return port_;
	}

} // end of namespace: TpQt4Communication


namespace Communication
{
	Credentials::Credentials(const QString& protocol, const QString& user_id, const QString &server, int port ): protocol_(protocol), user_id_(user_id), port_(port)
	{

	}

	void Credentials::SetProtocol(const QString &protocol)
	{
		protocol_ = protocol;
	}
	
	QString Credentials::GetProtocol() const
	{
		return protocol_;
	}

	void Credentials::SetUserID(const QString &user_id)
	{
		user_id_ = user_id;
	}

	QString Credentials::GetUserID() const
	{
		return user_id_;
	}

	void Credentials::SetServer(const QString &server)
	{
		server_ = server;
	}

	QString Credentials::GetServer() const
	{
		return server_;
	}

	void Credentials::SetPort(int port)
	{
		port_ = port;
	}

	int Credentials::GetPort() const
	{
		return port_;
	}

} // end of namespace: Communication
