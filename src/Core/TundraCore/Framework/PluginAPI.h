// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"

#include <QString>
#include <QObject>

#ifdef WIN32
#include "Win.h"
typedef HMODULE PluginHandle;
#elif defined(_POSIX_C_SOURCE) || defined(Q_WS_MAC) || defined(ANDROID)
#include <dlfcn.h>
typedef void * PluginHandle;
#endif

class Framework;

/// Implements plugin loading functionality.
class TUNDRACORE_API PluginAPI : public QObject
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
