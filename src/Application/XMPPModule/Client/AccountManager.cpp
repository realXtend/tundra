// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AccountManager.h"
#include "XMPPModule.h"

#include "ConfigAPI.h"
#include "Framework.h"
#include "LoggingFunctions.h"

#include <qxmpp/QXmppUtils.h>

#include <QFile>
#include <QTextStream>

#include "MemoryLeakCheck.h"

namespace XMPP
{

AccountManager::AccountManager(Framework *framework) :
    accounts_(new QDomDocument()),
    accounts_filename_("XmppAccounts.xml")
{
    accounts_path_ = framework->Config()->GetConfigFolder();
}

AccountManager::~AccountManager()
{
    SAFE_DELETE(accounts_);
}

QStringList AccountManager::GetServers()
{
    QStringList list;
    QDomElement element = accounts_->documentElement().firstChildElement("account");
    while(!element.isNull())
    {
        if(!element.firstChildElement("server").isNull())
            if(!list.contains(element.firstChildElement("server").text()))
                list << element.firstChildElement("server").text();
        element = element.nextSiblingElement("account");
    }
    return list;
}

QStringList AccountManager::GetUserJids(QString server)
{
    QStringList list;
    QDomElement element = accounts_->documentElement().firstChildElement("account");
    while(!element.isNull())
    {
        if(element.firstChildElement("server").text() == server)
            list << element.firstChildElement("jid").text();
        element = element.nextSiblingElement("account");
    }

    return list;
}

QString AccountManager::GetUserPassword(QString userJid, QString server)
{
    QDomElement element = accounts_->documentElement().firstChildElement("account");
    while(!element.isNull())
    {
        if(element.firstChildElement("jid").text() == userJid && element.firstChildElement("server").text() == server)
        {
            QByteArray passwdEncryptedBa = QByteArray::fromBase64(
                    element.firstChildElement("password").text().toUtf8());
            QString passwd = GenerateXor(passwdEncryptedBa, userJid.toUtf8());
            return passwd;
        }
        element = element.nextSiblingElement("account");
    }

    return "";
}

void AccountManager::AddLoginData(QString userJid, QString server, QString password)
{
    if(userJid.isEmpty() || server.isEmpty())
    {
        LogError("XMPPModule: Please provide atleast user Jabber ID and XMPP Server address to save login credentials.");
        return;
    }

    if(accounts_->documentElement().isNull())
    {
        accounts_->appendChild(accounts_->createElement("accounts"));
    }

    QDomElement element = accounts_->documentElement().firstChildElement("account");
    while(!element.isNull())
    {
        if(element.firstChildElement("jid").text() == userJid && element.firstChildElement("server").text() == server)
        {
            accounts_->documentElement().removeChild(element);
            break;
        }
        element = element.nextSiblingElement("account");
    }

    QDomElement newElement = accounts_->createElement("account");

    QDomElement newElementuserJid = accounts_->createElement("jid");
    newElementuserJid.appendChild(accounts_->createTextNode(userJid));
    newElement.appendChild(newElementuserJid);

    QDomElement newElementPasswd = accounts_->createElement("password");
    newElementPasswd.appendChild(accounts_->createTextNode(
            GenerateXor(password.toUtf8(), userJid.toUtf8()).toBase64()));
    newElement.appendChild(newElementPasswd);

    QDomElement newElementServer = accounts_->createElement("server");
    newElementServer.appendChild(accounts_->createTextNode(server));
    newElement.appendChild(newElementServer);

    accounts_->documentElement().appendChild(newElement);

    SaveToFile();
}

void AccountManager::LoadFromFile()
{
    QFile file(accounts_path_ + accounts_filename_);
    if(file.open(QIODevice::ReadOnly))
    {
        accounts_->setContent(&file, true);
    }
}

void AccountManager::SaveToFile()
{
    QFile file(accounts_path_ + accounts_filename_);
    if(file.open(QIODevice::ReadWrite))
    {
        QTextStream tstream(&file);
        accounts_->save(tstream, 2);
        file.close();
    }
}

QByteArray AccountManager::GenerateXor(const QByteArray &data, const QByteArray &key)
{
    Q_ASSERT(!key.isEmpty());

    QByteArray result;
    for(int i = 0 , j = 0; i < data.length(); ++i , ++j)
    {
        if(j == key.length())
            j = 0;
        result.append(data.at(i) ^ key.at(j));
    }
    return result;
}

} // end of namespace: XMPP
