// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ProtocoUtilities_LoginCredentials_h
#define incl_ProtocoUtilities_LoginCredentials_h

#include "NetworkEvents.h"

#include <QUrl>

class LoginCredentials
{
public:
    explicit LoginCredentials(ProtocolUtilities::AuthenticationType type) :  type_(type) {}
    ProtocolUtilities::AuthenticationType GetType() const { return type_; }
    QString GetIdentity() const
    {
        switch(type_)
        {
        case ProtocolUtilities::AT_OpenSim:
            return firstName_ + " " + lastName_;
        case ProtocolUtilities::AT_RealXtend:
        case ProtocolUtilities::AT_Taiga:
            return identity_;
        case ProtocolUtilities::AT_Unknown:
        default:
            return QString();
        }
    };

    QString GetFirstName() const { return firstName_; }
    QString GetLastName() const { return lastName_; }
    QString GetPassword() const { return password_; }
    QString GetStartLocation() const { return startLocation_; }
    QUrl GetAuthenticationUrl() { return authenticationUrl_; }

    void SetType(ProtocolUtilities::AuthenticationType type) { type_ = type; }
    void SetIdentity(const QString &identity) { identity_ = identity; }
    void SetFirstName(const QString &firstName) { firstName_= firstName; }
    void SetLastName(const QString &lastName) { lastName_= lastName; }
    void SetPassword(const QString &password) { password_= password; }
    void SetStartLocation(const QString &startLocation) { startLocation_= startLocation; }
    void SetAuthenticationUrl(const QUrl &authenticationUrl) { authenticationUrl_ = authenticationUrl; }

private:
    ProtocolUtilities::AuthenticationType type_;
    QString firstName_;
    QString lastName_;
    QString password_;
    QString startLocation_;
    QString identity_;
    QUrl authenticationUrl_;
};

#endif
