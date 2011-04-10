// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ConfigAPI_h
#define incl_Foundation_ConfigAPI_h

#include <QObject>
#include <QVariant>
#include <QString>

namespace Foundation { class Framework; }

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

public slots:
    /// Returns if a key is available in the config.
    /// \param file QString. Name of the file. For example: "foundation" or "foundation.ini" you can omit the .ini extension.
    /// \param section QString. The section in the config where key is. For example: "login".
    /// \param key QString. Key to look for in the file.
    /// \return boolean if key exists in section of file.
    bool HasValue(QString file, QString section, QString key);

    /// Gets a value of key from a config file
    /// \param file QString. Name of the file. For example: "foundation" or "foundation.ini" you can omit the .ini extension.
    /// \param section QString. The section in the config where key is. For example: "login".
    /// \param key QString. Key that value gets returned. For example: "username".
    /// \param defaultValue QVariant. What you expect to get back if the file/section/key combination was not found.
    /// \return QVariant The value of key in file.
    QVariant Get(QString file, QString section, QString key, const QVariant &defaultValue = QVariant());

    /// Sets the value of key in a config file.
    /// \param file QString. Name of the file. For example: "foundation" or "foundation.ini" you can omit the .ini extension.
    /// \param section QString. The section in the config where key is. For example: "login".
    /// \param key QString. Key that value gets returned. For example: "username".
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
    friend class Foundation::Framework;

    /// Constructs the Config API.
    /// \param framework Framework. Takes ownership of the object.
    ConfigAPI(Foundation::Framework *framework);

    /// Setter the application data. Get called by friend class framework.
    /// \param applicationOrganizaion QString. Application organization.
    /// \param applicationName QString. Application name.
    /// \param applicationVersion QString. Application version.
    void SetApplication(const QString &applicationOrganization, const QString &applicationName, const QString &applicationVersion);

    /// Prepares the data directory where config api will be working. This call will make sure that the folder path exists.
    /// \note Frameworks Platform object needs to be ready and prepared before this gets called.
    /// \param configFolderName QString. The sub folder name on where to store configs.
    void PrepareDataFolder(const QString &configFolderName);
    
    /// Framework ptr.
    Foundation::Framework *framework_;

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
