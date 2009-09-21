#ifndef incl_Comm_Credentials_h
#define incl_Comm_Credentials_h

#include "Foundation.h"
#include "EventDataInterface.h"



namespace TpQt4Communication
{

	/**
	 *  Credential for connecting to IM server
	 *
	 *  NOTE: Propably attribute map with arbitrary keys value pairs are the best choise here...
	 *
	 */
	class Credentials
	{
	public:
		Credentials();
		void SetProtocol(std::string protocol);
		void SetUserID(std::string user_id);
		void SetPassword(std::string pwd);
		void SetServer(std::string server);
		std::string GetProtocol() const;
		std::string GetUserID() const ;
		std::string GetUserPassword() const;
		std::string GetServer() const;
		void LoadFromFile(std::string path);
	private:
		std::string user_id_;
		std::string password_;
		std::string protocol_;
		std::string server_;
		std::map<std::string, std::string> attributes_;
	};
	typedef boost::weak_ptr<Credentials> CredentialsWeakPtr;
	

} // end of namespace: TpQt4Communication

#endif // incl_Comm_Credentials_h
