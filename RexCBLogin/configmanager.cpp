#include "configmanager.h"

ConfigManager::ConfigManager()
{
	worldList = new QStringList();
	openIDList = new QStringList();
}

void ConfigManager::setConfigFilePath(QString path)
{
	configFilePath = new QString(path);
}

bool ConfigManager::readConfig()
{
	QFile file(*configFilePath);
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
	QFile file(*configFilePath);
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
	writer.writeStartElement("world");
	writer.writeCharacters(world);
	writer.writeEndElement();
	// </world>
	// <openid>
	for (int i=0; i<openIDList->size(); ++i)
	{
		writer.writeStartElement("openid");
		writer.writeCharacters(openIDList->at(i));
		writer.writeEndElement();
	}
	writer.writeStartElement("openid");
	writer.writeCharacters(openID);
	writer.writeEndElement();
	// </openid>
	writer.writeEndElement();
	// </identities>

	//writer.writeStartElement("world");
	//writer.writeCharacters(world);
	//writer.writeEndElement();

	file.close();
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
