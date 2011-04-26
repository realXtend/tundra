// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Framework_ConfigAPI_h
#define incl_Framework_ConfigAPI_h

#include <QObject>
#include <QVariant>
#include <QString>

class Framework;

/// Config info. A reusable config info for conviance so you can do less typin when dealing with constantly same config file/sections. QObject for script usage.
class ConfigData : public QObject
{
Q_OBJECT
public:
    Q_PROPERTY(QString file WRITE setFile READ getFile);
    Q_PROPERTY(QString section WRITE setSection READ getSection);
    Q_PROPERTY(QString key WRITE setKey READ getKey);
    Q_PROPERTY(QVariant value WRITE setValue READ getValue);
    Q_PROPERTY(QVariant defaultValue WRITE setDefaultValue READ getDefaultValue);

    ConfigData() {}
    ConfigData(const QString &file, const QString section, const QString &key = QString(), const QVariant &value = QVariant(), const QVariant &defaultValue = QVariant())
    {
        setFile(file);
        setSection(section);
        setKey(key);
        setValue(value); 
        setDefaultValue(defaultValue);
    }

    QString file;
    QString section;
    QString key;
    QVariant value;
    QVariant defaultValue;

public slots:
    QString getFile()                   { return file; }
    QString getSection()                { return section; }
    QString getKey()                    { return key; }
    QVariant getValue()                 { return value; }
    QVariant getDefaultValue()          { return defaultValue; }

    void setFile(const QString &s)      { file = s; }
    void setSection(const QString &s)   { section = s; }
    void setKey(const QString &s)       { key = s; }
    void setValue(const QVariant &v)    { value = v; }
    void setDefaultValue(const QVariant &v) { defaultValue = v; }
};

/** \brief Configuration API for getting and setting config values.
    \details Configuration API for getting and setting config values. Utilizing the ini file format and QSettings class.
    The API will return QVariant values and the user will have to know what type the value is and use the extensive QVariants::to*() functions
    to get the correct type. The Config API supports ini sections but you may also write to the root of the ini document without a section.
    This class is kept simple and easy to use and designed to be script language friendly. The API is registered to 'config' dynamic property.

    JavaScript example on usage:
    
    \code
    var file = "myconfig";

    config.Set(file, "world", new QUrl("http://server.com")); // QUrl
    config.Set(file, "port", 8013); // int
    config.Set(file, "login data", "username", "John Doe"); // QString
    config.Set(file, "login data", "password", "pass123"); // QString

    var username = config.Get(file, "login data", "username");
    if (username != null)
        print("Hello there", username);
    etc.
    \endcode

    \note All file, key and section parameters are case insensitive. This means all of them are transformed to 
    lower case before any accessing files. "MyKey" will get and set you same value as "mykey".
*/

class ConfigAPI : public QObject
{

Q_OBJECT

public:
    static QString FILE_FRAMEWORK;
    static QString SECTION_FRAMEWORK;
    static QString SECTION_SERVER;
    static QString SECTION_CLIENT;
    static QString SECTION_RENDERING;
    static QString SECTION_UI;
    
public slots:
    /// Returns if a key is available in the config.
    /// \param data ConfigData. Filled ConfigData object.
    /// \param key QString. Key to look for in the file under section.
    /// \return boolean if key exists in section of file.
    bool HasValue(const ConfigData &data);

    /// Returns if a key is available in the config.
    /// \param data ConfigData. Filled ConfigData object.
    /// \return boolean if key exists in section of file under section.
    bool HasValue(const ConfigData &data, QString key);

    /// Returns if a key is available in the config.
    /// \param file QString. Name of the file. For example: "foundation" or "foundation.ini" you can omit the .ini extension.
    /// \param section QString. The section in the config where key is. For example: "login".
    /// \param key QString. Key to look for in the file under section.
    /// \return boolean if key exists in section of file.
    bool HasValue(QString file, QString section, QString key);

    /// Gets a value of key from a config file
    /// \param data ConfigData. Filled ConfigData object.
    /// \return QVariant The value of key/section in file.
    QVariant Get(const ConfigData &data);

    /// Gets a value of key from a config file
    /// \param data ConfigData. ConfigData object that has file and section filled, also may have defaultValue and it will be used if input defaultValue is null.
    /// \param key QString. Key that value gets returned. For example: "username".
    /// \param defaultValue QVariant. What you expect to get back if the file/section/key combination was not found.
    /// \return QVariant The value of key/section in file.
    QVariant Get(const ConfigData &data, QString key, const QVariant &defaultValue = QVariant());

    /// Gets a value of key from a config file
    /// \param file QString. Name of the file. For example: "foundation" or "foundation.ini" you can omit the .ini extension.
    /// \param section QString. The section in the config where key is. For example: "login".
    /// \param key QString. Key that value gets returned. For example: "username".
    /// \param defaultValue QVariant. What you expect to get back if the file/section/key combination was not found.
    /// \return QVariant The value of key/section in file.
    QVariant Get(QString file, QString section, QString key, const QVariant &defaultValue = QVariant());   

    /// Sets the value of key in a config file.
    /// \param data ConfigData. Filled ConfigData object.
    /// \return QVariant The value of key/section in file.
    void Set(const ConfigData &data);

    /// Sets the value of key in a config file.
    /// \param data ConfigData. ConfigData object that has file and section filled.
    /// \param key QString. Key that value gets set. For example: "username".
    /// \param value QVariant. New Value of key in file.
    void Set(const ConfigData &data, QString key, const QVariant &value);

    /// Sets the value of key in a config file.
    /// \param file QString. Name of the file. For example: "foundation" or "foundation.ini" you can omit the .ini extension.
    /// \param section QString. The section in the config where key is. For example: "login".
    /// \param key QString. Key that value gets set. For example: "username".
    /// \param value QVariant. New Value of key in file.
    void Set(QString file, QString section, QString key, const QVariant &value);

    /// Gets the absolute path to the config folder where configs are stored. Guaranteed to have a trailing forward slash '/'.
    /// \return QString. Absolute path to config storage folder.
    QString GetConfigFolder() const { return configFolder_; }

    /// Get the application organization.
    /// \return QString. Application organization.
    QString GetApplicationOrganization() const { return applicationOrganization_; }

    /// Get the application name.
    /// \return QString. Application name.
    QString GetApplicationName() const { return applicationName_; }

    /// Get the application version.
    /// \return QString. Application version.
    QString GetApplicationVersion() const { return applicationVersion_; }

    /// Get the application identifier
    /// \return QString of ApplicationOrganization + " " + ApplicationName + " " ApplicationVersion
    QString GetApplicationIdentifier() const { return applicationOrganization_ + " " + applicationName_ + " " + applicationVersion_; }

private slots:
    /// Get absolute file path for file. Guarantees that it ends with .ini.
    QString GetFilePath(const QString &file);

    /// Prepare string for config usage. Removes spaces from end and start, replaces mid string spaces with '_' and forces to lower case.
    void PrepareString(QString &str);

private:
    Q_DISABLE_COPY(ConfigAPI)
    friend class Framework;

    /// Constructs the Config API.
    /// \param framework Framework. Takes ownership of the object.
    ConfigAPI(Framework *framework);

    /// Setter the application data. Get called by friend class framework.
    /// \param applicationOrganizaion QString. Application organization.
    /// \param applicationName QString. Application name.
    /// \param applicationVersion QString. Application version.
    void SetApplication(const QString &applicationOrganization, const QString &applicationName, const QString &applicationVersion);

    /// Prepares the data directory where config api will be working. This call will make sure that the folder path exists.
    /// \param configFolderName QString. The sub folder name on where to store configs.
    void PrepareDataFolder(const QString &configFolderName);
    
    /// Framework ptr.
    Framework *framework_;

    /// Absolute path to the folder where to store the config files.
    QString configFolder_;

    /// Application organization.
    QString applicationOrganization_;

    /// Application name.
    QString applicationName_;

    /// Application version.
    QString applicationVersion_;
};

#endif
