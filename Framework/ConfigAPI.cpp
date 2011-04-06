
#include "StableHeaders.h"
#include "Framework.h"
#include "ConfigAPI.h"

#include <QSettings>

ConfigAPI::ConfigAPI(Foundation::Framework *framework, const QString &configFolder) :
    QObject(framework),
    framework_(framework),
    configFolder_(configFolder)
{
    // Replace windows style backslashes with forward slashes
    configFolder_.replace("\\", "/");

    // Be sure the path ends with a forward slash
    if (!configFolder_.endsWith("/"))
        configFolder_.append("/");

    // Register to scripts
    framework_->RegisterDynamicObject("config", this);
}

QString ConfigAPI::GetFilePath(const QString &file)
{
    QString filePath = configFolder_ + file.trimmed().toLower();
    if (!filePath.endsWith(".ini"))
        filePath.append(".ini");
    return filePath;
}

QVariant ConfigAPI::Get(const QString &file, const QString &key)
{
    return Get(file, QString(), key);
}

QVariant ConfigAPI::Get(const QString &file, const QString &section, const QString &key)
{
    QSettings config(GetFilePath(file), QSettings::IniFormat);
    if (section.isEmpty())
        return config.value(key.trimmed().toLower());
    else
        return config.value(section.trimmed().toLower() + "/" + key.trimmed().toLower());
}

void ConfigAPI::Set(const QString &file, const QString &key, const QVariant &value)
{
    Set(file, QString(), key, value);
}

void ConfigAPI::Set(const QString &file, const QString &section, const QString &key, const QVariant &value)
{
    QSettings config(GetFilePath(file), QSettings::IniFormat);
    if (!config.isWritable())
        return;
    if (section.isEmpty())
        config.setValue(key.trimmed().toLower(), value);
    else
        config.setValue(section.trimmed().toLower() + "/" + key.trimmed().toLower(), value);
    config.sync();
}