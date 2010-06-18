// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ConfigManager.h"
#include "InputEvents.h"

#include <QSettings>
#include <QFile>

namespace Input
{
    ConfigManager::ConfigManager(QObject *parent) :
        QObject(parent),
        default_config_("./data/default_bindings.ini"),
        custom_config_("bindings/current")
    {

    }

    // Public

    Foundation::KeyBindings *ConfigManager::GetUsedKeyBindings()
    {
        if (CustomConfigDefined())
            return ParseConfig("Bindings.Custom");
        else
            return ParseConfig("Bindings.Default");
    }

    Foundation::KeyBindings *ConfigManager::ParseConfig(QString group)
    {
        QSettings *settings;
        if (group == "Bindings.Custom")
            settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, custom_config_);
        else if (group == "Bindings.Default")
        {
            settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "bindings/default_bindings");
            if (settings->childGroups().count() == 0)
            {
                // Copy default_bindings.ini once to users scopes data folder
                // QSettings assumes read/write even if we only reading so we cant use the install folder
                // Windows UAC without admin mode will cause the file not to open from install folder, bindings pointer is null and
                // user will crash when starting to use the keyboard
                QFile default_bindings("./data/default_bindings.ini");
                QFile userscope_defaults(settings->fileName());

                if (default_bindings.open(QIODevice::ReadOnly) && userscope_defaults.open(QIODevice::ReadWrite))
                    userscope_defaults.write(default_bindings.readAll());
                default_bindings.close();
                userscope_defaults.close();

                settings->sync();
            }
        }

        if (!settings->isWritable())
            return 0;

        Foundation::KeyBindings *bindings = new Foundation::KeyBindings();

        foreach (QString identifier, bindings->GetConfigKeys())
        {
            QVariant current = settings->value(group + "/" + identifier);
            if (current.isNull())
                continue;

            QStringList keys_seq_list = current.toStringList();
            foreach (QString seq, keys_seq_list)
            {
                bindings->BindKey(QKeySequence(seq), bindings->EventPairForName(identifier));
            }
        }

        SAFE_DELETE(settings);
        return bindings;
    }

    void ConfigManager::WriteCustomConfig(Foundation::KeyBindings *bindings)
    {
        QSettings custom_settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, custom_config_);
        if (!custom_settings.isWritable())
            return;

        custom_settings.clear();
        custom_settings.beginGroup("Bindings.Custom");

        foreach(QString identifier, bindings->GetConfigKeys())
        {
            Foundation::EventPair find_pair(bindings->EventPairForName(identifier));
            if (find_pair.enter_id == 0)
                continue;

            std::list<Foundation::Binding> bindings_list = bindings->GetBindings(find_pair);
            if (bindings_list.size() > 1)
            {
                QList<QVariant> sequence_list;
                std::list<Foundation::Binding>::const_iterator iter = bindings_list.begin();
                while (iter != bindings_list.end())
                {
                    sequence_list.append((*iter).sequence);
                    iter++;
                }
                custom_settings.setValue(identifier, sequence_list);
            }
            else
            {
                if (bindings_list.size() != 0)
                    custom_settings.setValue(identifier, bindings_list.front().sequence);
            }
        }

        custom_settings.endGroup();
        custom_settings.sync();
    }

    void ConfigManager::ClearUserConfig()
    {
        QSettings custom_settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, custom_config_);
        if (custom_settings.isWritable())
        {
            custom_settings.clear();
            QFile custom_settings_file(custom_settings.fileName());
            custom_settings_file.remove();
        }
    }

    // Private

    bool ConfigManager::CustomConfigDefined()
    {
        QSettings custom_settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, custom_config_);
        if (custom_settings.childGroups().count() == 0)
            return false;
        return true;
    }

}