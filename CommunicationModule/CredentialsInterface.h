// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_CredentialsInterface_h
#define incl_Comm_CredentialsInterface_h

class QString;

namespace Communication
{
    /**
     *  Gredentials for opening connection to IM server. Some fields are optional in 
     *  with some protocols
     *  usage: credentials = new Credentials("jabber", "my_jabber_id", "my_jabber_server", "5222"); 
     *
     *  \todo <string><string map might be better here so that we could offer protocol specific fields.
     *  \todo Also per protocol Credertials classes might be the right way
     *        eg. JabberCredentials, IrcCredentials, OpensimIMCredentials 
     */
    class CredentialsInterface
    {
    public:
//        virtual CredentialsInterface(const QString& protocol, const QString& user_id, const QString &server, const QString &port ) = 0;

        //! Define the used protocol eg. "Jabber", "Opensim_UDP_IM", "irc"
        virtual void SetProtocol(const QString &protocol) = 0;

        //! Provides the protocol
        virtual QString GetProtocol() const = 0;

        virtual void SetUserID(const QString &user_id) = 0;
        virtual QString GetUserID() const = 0;

        virtual void SetPassword(const QString &password) = 0;
        virtual QString GetPassword() const = 0;

        virtual void SetServer(const QString &server) = 0;
        virtual QString GetServer() const = 0;

        //! Define port of IM server to connect
        virtual void SetPort(int port) = 0;

        //! Provides the port of IM server
        virtual int GetPort() const = 0;
    };
}

#endif

