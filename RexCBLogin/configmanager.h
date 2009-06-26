#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QStringList>
#include <QFile>

class ConfigManager
{

public:
	ConfigManager();
	void setConfigFilePath(QString path);
	bool readConfig();
	bool writeCongif(QString world, QString openID);
	QStringList* getWorldList();
	QStringList* getOpenIDList();

private:
	QString *configFilePath;
	QXmlStreamReader reader;
	QStringList *worldList;
	QStringList *openIDList;

};

#endif