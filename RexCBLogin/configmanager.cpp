#include "../Core/DebugOperatorNew.h"
#include "configmanager.h"
#include "../Core/MemoryLeakCheck.h"

ConfigManager::ConfigManager()
{
	worldList = new QStringList();
	openIDList = new QStringList();
}

ConfigManager::~ConfigManager()
{
	delete worldList;
	delete openIDList;
}

void ConfigManager::setConfigFilePath(QString path)
{
	configFilePath = QString(path);
}

bool ConfigManager::readConfig()
{
	QFile file(configFilePath);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }

	reader.setDevice(&file);
	reader.readNext();

	if ( !reader.isStandaloneDocument() ) 
	{
		reader.readNext();
		if( reader.isStartElement() )
		{
			// Pass a few tokens to get first
			reader.readNext();
			reader.readNext();
			while ( !reader.atEnd() )
			{
				if( reader.name() == "world" )
				{
					worldList->append( reader.readElementText() );
				} 
				else if (reader.name() == "openid")
				{
					openIDList->append( reader.readElementText() );
				}
				reader.readNext();
			}
		}
	}
	file.close();
	return true;
}

bool ConfigManager::writeCongif(QString world, QString openID)
{
	if ( !worldList->contains(world) || !openIDList->contains(openID) )
	{
		QFile file(configFilePath);
		if (!file.open(QFile::WriteOnly | QFile::Text)) {
			return false;
		}

		QXmlStreamWriter writer(&file);
		writer.setAutoFormatting(true);
		
		// <identities>
		writer.writeStartElement("identities");

		// <world>
		for (int i=0; i<worldList->size(); ++i)
		{
			writer.writeStartElement("world");
			writer.writeCharacters(worldList->at(i));
			writer.writeEndElement();
		}
		if ( worldList->contains(world) == false ) 
		{
			writer.writeStartElement("world");
			writer.writeCharacters(world);
			writer.writeEndElement();
			worldList->append(world);
		}
		// </world>

		// <openid>
		for (int i=0; i<openIDList->size(); ++i)
		{
			writer.writeStartElement("openid");
			writer.writeCharacters(openIDList->at(i));
			writer.writeEndElement();
		}
		if ( openIDList->contains(openID) == false )
		{
			writer.writeStartElement("openid");
			writer.writeCharacters(openID);
			writer.writeEndElement();
			openIDList->append(openID);
		}
		// </openid>

		writer.writeEndElement();
		// </identities>

		file.close();
	}
	return true;
}

QStringList* ConfigManager::getWorldList()
{
	return worldList;
}

QStringList* ConfigManager::getOpenIDList()
{
	return openIDList;
}
