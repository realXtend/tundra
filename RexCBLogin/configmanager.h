#ifndef incl_ConfigManager_h
#define incl_ConfigManager_h

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QStringList>
#include <QFile>

class ConfigManager
{

public:
	ConfigManager();
	~ConfigManager();
	void setConfigFilePath(QString path);
	bool readConfig();
	bool writeCongif(QString world, QString openID);
	QStringList* getWorldList();
	QStringList* getOpenIDList();

private:
	QString configFilePath;
	QXmlStreamReader reader;
	QStringList *worldList;
	QStringList *openIDList;

};

#endif // incl_ConfigManager_h