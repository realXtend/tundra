// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ProtocoUtilities_LoginCredentials_h
#define incl_ProtocoUtilities_LoginCredentials_h

#include "Interfaces/LoginCredentialsInterface.h"

#include <QUrl>

namespace ProtocolUtilities
{

	class TaigaCredentials : public LoginCredentialsInterface
	{

	public:
		TaigaCredentials();
		virtual ~TaigaCredentials();

		virtual QString GetIdentity();
		virtual void SetIdentityUrl(QString identityUrl);

	private:
		QString identityUrl_;

	};

	class OpenSimCredentials : public ProtocolUtilities::LoginCredentialsInterface
	{

	public:
		OpenSimCredentials();
		virtual ~OpenSimCredentials();

		virtual QString GetIdentity();
		virtual QString GetFirstName();
		virtual QString GetLastName();
		virtual QString GetPassword();
		virtual void SetFirstName(QString firstName);
		virtual void SetLastName(QString firstName);
		virtual void SetPassword(QString password);

	private:
		QString firstName_;
		QString lastName_;
		QString password_;

	};

	class RealXtendCredentials : public ProtocolUtilities::LoginCredentialsInterface
	{

	public:
		RealXtendCredentials();
		virtual ~RealXtendCredentials();

		virtual QString GetIdentity();
		virtual QString GetPassword();
		virtual QUrl GetAuthenticationUrl();
		virtual void SetIdentity(QString identity);
		virtual void SetPassword(QString password);
		virtual void SetAuthenticationUrl(QUrl authenticationUrl);

	private:
		QString identity_;
		QString password_;
		QUrl authenticationUrl_;
	};

}

#endif // incl_ProtocoUtilities_LoginCredentials_h