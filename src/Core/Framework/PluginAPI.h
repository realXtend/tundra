// For conditions of distribution and use, see copyright notice in license.txt
#pragma once

#include <QString>
#include <QObject>

#ifdef WIN32
#include <Winsock2.h>
#include <Windows.h>
typedef HMODULE PluginHandle;
#elif defined(_POSIX_C_SOURCE) || defined(Q_WS_MAC)
#include <dlfcn.h>
typedef void * PluginHandle;
#endif

class Framework;

/// Implements plugin loading functionality.
class PluginAPI : public QObject
{
    Q_OBJECT;

private:
    struct Plugin
    {
        PluginHandle libraryHandle;
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

    void UnloadPlugins();

};
