// For conditions of distribution and use, see copyright notice in license.txt
#pragma once

#include <QString>
#include <QObject>
#include <Windows.h> ///\todo Cross-platform -> <dlfcn.h>

class Framework;

class PluginAPI : public QObject
{
    Q_OBJECT;

private:
    struct Plugin
    {
        HMODULE libraryHandle; ///\todo Cross-platform -> void*.
    };
    std::list<Plugin> plugins;

    Framework *owner;
public:
    explicit PluginAPI(Framework *owner);

    /// Returns the plugin configuration file that was used to load the plugins at startup.
    QString ConfigurationFile() const;

    /// Loads and executes the given shared library plugin.
    void LoadPlugin(const QString &filename);

    /// Parses the specified .xml file and loads and executes all plugins specified in that file.
    void LoadPluginsFromXML(QString pluginListFilename);
};
