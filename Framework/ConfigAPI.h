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
    /// Gets a value of key from a config file
    /// \param file QString. Name of the file. For example: "foundation" or "foundation.ini" you can omit the .ini extension.
    /// \param key QString. Key that value gets returned. For example: "username".
    /// \return QVariant The value of key in file.
    QVariant Get(const QString &file, const QString &key);

    /// Gets a value of key from a config file
    /// \param file QString. Name of the file. For example: "foundation" or "foundation.ini" you can omit the .ini extension.
    /// \param section QString. The section in the config where key is. For example: "login".
    /// \param key QString. Key that value gets returned. For example: "username".
    /// \return QVariant The value of key in file.
    QVariant Get(const QString &file, const QString &section, const QString &key);

    /// Sets the value of key in a config file.
    /// \param file QString. Name of the file. For example: "foundation" or "foundation.ini" you can omit the .ini extension.
    /// \param key QString. Key that value gets returned. For example: "username".
    /// \param value QVariant. New Value of key in file.
    void Set(const QString &file, const QString &key, const QVariant &value);

    /// Sets the value of key in a config file.
    /// \param file QString. Name of the file. For example: "foundation" or "foundation.ini" you can omit the .ini extension.
    /// \param section QString. The section in the config where key is. For example: "login".
    /// \param key QString. Key that value gets returned. For example: "username".
    /// \param value QVariant. New Value of key in file.
    void Set(const QString &file, const QString &section, const QString &key, const QVariant &value);

    /// Gets the absolute path to the config folder where configs are stored. Guaranteed to have a trailing forward slash '/'.
    /// \return QString. Absolute path to config storage folder.
    QString GetConfigFolder() const { return configFolder_; }

private slots:
    /// Get absolute file path for file. Guarantees that it ends with .ini.
    QString GetFilePath(const QString &file);

private:
    Q_DISABLE_COPY(ConfigAPI)
    friend class Foundation::Framework;

    /// Constructs the Config API.
    /// \param framework Framework. Takes ownership of the object.
    /// \param configFolder QString. Tells the config api where to store config files.
    ConfigAPI(Foundation::Framework *framework, const QString &configFolder);

    /// Framework ptr.
    Foundation::Framework *framework_;

    /// Absolute path to the folder where to store the config files.
    QString configFolder_;
};

#endif
