// For conditions of distribution and use, see copyright notice in license.txt
#pragma once

#include <QString>
#include <QObject>

#if defined(_WINDOWS)
#if defined(_WINSOCKAPI_)
#undef _WINSOCKAPI_ 
#endif
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
    Q_OBJECT

public:
    explicit PluginAPI(Framework *owner);

    /// Returns list of plugin configuration files that were used to load the plugins at startup.
    QStringList ConfigurationFiles() const;

    /// Loads and executes the given shared library plugin.
    void LoadPlugin(const QString &filename);

    /// Parses the specified .xml file and loads and executes all plugins specified in that file.
    void LoadPluginsFromXML(QString pluginListFilename);

    void UnloadPlugins();

private:
    struct Plugin
    {
        PluginHandle libraryHandle;
    };
    std::list<Plugin> plugins;

    Framework *owner;
};
