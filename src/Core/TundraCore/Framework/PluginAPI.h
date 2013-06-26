// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"

#include <QString>
#include <QObject>

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

    /// Loads plugins specified on command line with --plugin
    void LoadPluginsFromCommandLine();

    void UnloadPlugins();

public slots:
    /// Prints the list of loaded plugins to the console.
    void ListPlugins() const;

private:
    struct Plugin
    {
        void *handle;
        QString name;
        QString filename;
    };
    std::list<Plugin> plugins;

    Framework *owner;
};
