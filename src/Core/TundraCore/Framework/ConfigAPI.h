// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"

#include <QObject>
#include <QVariant>
#include <QString>

class Framework;

/// Convenience structure for dealing constantly with same config file/sections.
/** @todo Make a simple struct and expose to QtScript by using the QScriptBindings tool. */
class TUNDRACORE_API ConfigData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString file WRITE SetFile READ File);
    Q_PROPERTY(QString section WRITE SetSection READ Section);
    Q_PROPERTY(QString key WRITE SetKey READ Key);
    Q_PROPERTY(QVariant value WRITE SetValue READ Value);
    Q_PROPERTY(QVariant defaultValue WRITE SetDefaultValue READ DefaultValue);

public:
    ConfigData() {}
    ConfigData(const QString &file, const QString &section, const QString &key = QString(), const QVariant &value = QVariant(), const QVariant &defaultValue = QVariant())
    {
        SetFile(file);
        SetSection(section);
        SetKey(key);
        SetValue(value);
        SetDefaultValue(defaultValue);
    }

    QString File() const { return file; }
    QString Section() const { return section; }
    QString Key() const { return key; }
    QVariant Value() const { return value; }
    QVariant DefaultValue() const { return defaultValue; }

    void SetFile(const QString &s) { file = s; }
    void SetSection(const QString &s) { section = s; }
    void SetKey(const QString &s) { key = s; }
    void SetValue(const QVariant &v) { value = v; }
    void SetDefaultValue(const QVariant &v) { defaultValue = v; }

    QString file;
    QString section;
    QString key;
    QVariant value;
    QVariant defaultValue;
};

/// Configuration API for accessing config files.
/** The Configuration API utilizes QVariants extensively for script-compatibility.
    In C++ code use the QVariant::to*() functions to convert the values to the correct type.
    The Config API supports ini sections but you may also write to the root of the ini document without a section.

    The API is available as 'config' dynamic property.

    JavaScript example on usage:
    @code
    var file = "myconfig";

    config.Set(file, "world", new QUrl("http://server.com")); // QUrl
    config.Set(file, "port", 8013); // int
    config.Set(file, "login data", "username", "John Doe"); // QString
    config.Set(file, "login data", "password", "pass123"); // QString

    var username = config.Get(file, "login data", "username");
    if (username != null)
        print("Hello there " + username);
    etc.
    @endcode

    @note All file, key and section parameters are case-insensitive. This means all of them are transformed to 
    lower case before any accessing files. "MyKey" will get and set you same value as "mykey". */
class TUNDRACORE_API ConfigAPI : public QObject
{
    Q_OBJECT

public:
    ///\todo Make these properties so that can be obtained to scripts too.
    static QString FILE_FRAMEWORK;
    static QString SECTION_FRAMEWORK;
    static QString SECTION_SERVER;
    static QString SECTION_CLIENT;
    static QString SECTION_RENDERING;
    static QString SECTION_UI;
    static QString SECTION_SOUND;

public slots:
    /// Returns if a key exists in the config.
    /** @param file Name of the file. For example: "foundation" or "foundation.ini" you can omit the .ini extension.
        @param section The section in the config where key is. For example: "login".
        @param key  Key to look for in the file under section. */
    bool HasKey(QString file, QString section, QString key) const;
    bool HasKey(const ConfigData &data) const; /**< @overload @param data Filled ConfigData object */
    bool HasKey(const ConfigData &data, QString key) const; /**< @overload */

    /// @todo Add DeleteKey

    /// Returns value for a key in a config file
    /** @param file Name of the file. For example: "foundation" or "foundation.ini" you can omit the .ini extension.
        @param section The section in the config where key is. For example: "login".
        @param key Key that value gets returned. For example: "username".
        @param defaultValue What you expect to get back if the file/section/key combination was not found.
        @return The value of key/section in file. */
    QVariant Get(QString file, QString section, QString key, const QVariant &defaultValue = QVariant()) const;
    QVariant Get(const ConfigData &data) const; /**< @overload @param data Filled ConfigData object. */
    /// @overload
    /** @param data ConfigData object that has file and section filled, also may have defaultValue and it will be used if input defaultValue is null. */
    QVariant Get(const ConfigData &data, QString key, const QVariant &defaultValue = QVariant()) const;

    /** Sets the value of key in a config file.
        @param file Name of the file. For example: "foundation" or "foundation.ini" you can omit the .ini extension.
        @param section The section in the config where key is. For example: "login".
        @param key Key that value gets set. For example: "username".
        @param value New value for the key.
        @note If setting value of type float, convert to double if you want the value to be human-readable in the file. */
    void Set(QString file, QString section, QString key, const QVariant &value); /**< @overload */
    void Set(const ConfigData &data); /**< @overload @param data Filled ConfigData object.*/
    void Set(const ConfigData &data, QString key, const QVariant &value); /**< @overload @param data ConfigData object that has file and section filled. */

    /// Returns the absolute path to the config folder where configs are stored. Guaranteed to have a trailing forward slash '/'.
    QString ConfigFolder() const { return configFolder_; }

    // DEPRECATED
    bool HasValue(QString file, QString section, QString key) const { return HasKey(file, section, key); } /**< @deprecated Use HasKey. @todo Add warning print @todo Remove */
    bool HasValue(const ConfigData &data) const { return HasKey(data); } /**< @deprecated Use HasKey. @todo Add warning print @todo Remove */
    bool HasValue(const ConfigData &data, QString key) const { return HasKey(data, key); } /**< @deprecated Use HasKey. @todo Add warning print @todo Remove */
    QString GetConfigFolder() const { return ConfigFolder(); } /**< @deprecated Use ConfigFolder. @todo Add warning print @todo Remove */

private:
    /// Get absolute file path for file. Guarantees that it ends with .ini.
    QString GetFilePath(const QString &file) const;

    /// Returns if file provided by the ConfigAPI caller is secure and we should write/read from it.
    /** The purpose of this function is to verify the file provided by calling code
        does not go out of the confined ConfigAPI folder. For security reasons we cannot let
        eg. scripts open configs where they like. The whole operation will be canceled if this validation fails. */
    bool IsFilePathSecure(const QString &file) const;

    /// Prepare string for config usage. Removes spaces from end and start, replaces mid string spaces with '_' and forces to lower case.
    void PrepareString(QString &str) const;

private:
    Q_DISABLE_COPY(ConfigAPI)
    friend class Framework;

    /// @note Framework takes ownership of the object.
    explicit ConfigAPI(Framework *framework);

    /// Opens up the Config API to the given data folder. This call will make sure that the required folders exist.
    /** @param configFolderName The name of the folder to store Tundra Config API data to. */
    void PrepareDataFolder(QString configFolderName);

    Framework *framework_;
    QString configFolder_; ///< Absolute path to the folder where to store the config files.
};
