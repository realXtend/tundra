// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Credentials.h"

#include "MemoryLeakCheck.h"

namespace Communication
{
    Credentials::Credentials() : protocol_(""), user_id_(""), password_(""), server_(""), port_(0)
    {

    }

    Credentials::Credentials(const QString& protocol, const QString& user_id, const QString &password, const QString &server, int port ): protocol_(protocol), user_id_(user_id), password_(password), server_(server), port_(port)
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

    void Credentials::SetPassword(const QString &password)
    {
        password_ = password;
    }

    QString Credentials::GetPassword() const
    {
        return password_;
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
