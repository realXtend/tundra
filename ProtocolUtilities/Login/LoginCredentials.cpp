// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "Login/LoginCredentials.h"

namespace ProtocolUtilities
{

	TaigaCredentials::TaigaCredentials()
		: identityUrl_(0)
	{

	}

	TaigaCredentials::~TaigaCredentials()
	{

	}

	QString TaigaCredentials::GetIdentity()
	{
		return identityUrl_;
	}

	void TaigaCredentials::SetIdentityUrl(QString identityUrl)
	{
		identityUrl_ = identityUrl;
	}

	OpenSimCredentials::OpenSimCredentials()
		: firstName_(0), lastName_(0), password_(0)
	{

	}

	OpenSimCredentials::~OpenSimCredentials()
	{

	}

	QString OpenSimCredentials::GetIdentity()
	{
		return QString(firstName_ + " " + lastName_);
	}

	void OpenSimCredentials::SetFirstName(QString firstName)
	{
		firstName_ = firstName;
	}

	void OpenSimCredentials::SetLastName(QString lastName)
	{
		lastName_ = lastName;
	}

	void OpenSimCredentials::SetPassword(QString password)
	{
		password_ = password;
	}

	QString OpenSimCredentials::GetFirstName()
	{
		return firstName_;
	}

	QString OpenSimCredentials::GetLastName()
	{
		return lastName_;
	}

	QString OpenSimCredentials::GetPassword()
	{
		return password_;
	}

	RealXtendCredentials::RealXtendCredentials()
		: identity_(0), password_(0)
	{

	}

	RealXtendCredentials::~RealXtendCredentials()
	{
		
	}

	QString RealXtendCredentials::GetIdentity()
	{
		return identity_;
	}

	QString RealXtendCredentials::GetPassword()
	{
		return password_;
	}

	QUrl RealXtendCredentials::GetAuthenticationUrl()
	{
		return authenticationUrl_;
	}

	void RealXtendCredentials::SetIdentity(QString identity)
	{
		identity_ = identity;
	}

	void RealXtendCredentials::SetPassword(QString password)
	{
		password_ = password;
	}

	void RealXtendCredentials::SetAuthenticationUrl(QUrl authenticationUrl)
	{
		authenticationUrl_ = authenticationUrl;
	}
}