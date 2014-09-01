// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "PluginAPI.h"
#include "LoggingFunctions.h"
#include "Framework.h"
#include "Application.h"
#include "SystemInfo.h"

#include <QtXml>
#include <QDir>
#include <QFile>

#ifdef WIN32
#include "Win.h"
#else
#include <dlfcn.h>
#endif

/// Signature for Tundra plugins
typedef void (*TundraPluginMainSignature)(Framework *owner);

PluginAPI::PluginAPI(Framework *owner_)
:owner(owner_)
{
}

void PluginAPI::LoadPlugin(const QString &filename)
{
#ifdef ANDROID
    // On Android plugins are currently static
    return;
#endif

#ifdef WIN32
  #ifdef _DEBUG
    const QString pluginSuffix = "d.dll";
  #else
    const QString pluginSuffix = ".dll";
  #endif
#elif defined(_POSIX_C_SOURCE) || defined(ANDROID)
    const QString pluginSuffix = ".so";
#elif defined(__APPLE__)
    const QString pluginSuffix = ".dylib";
#endif

    // Check if the plugin source file even exists.
    QString path = QDir::toNativeSeparators(Application::InstallationDirectory() + "plugins/" + filename.trimmed() + pluginSuffix);
    if (!QFile::exists(path))
    {
        LogWarning(QString("Cannot load plugin \"%1\" as the file does not exist.").arg(path));
        return;
    }
    LogInfo("Loading plugin " + filename);
    owner->App()->SetSplashMessage("Loading plugin " + filename);

#ifdef WIN32
    HMODULE module = LoadLibrary(QStringToWString(path).c_str());
    if (module == NULL)
    {
        DWORD errorCode = GetLastError();
        LogError(QString("Failed to load plugin from \"%1\": %2 (Missing dependencies?)").arg(path).arg(GetErrorString(errorCode).c_str()));
        return;
    }
    TundraPluginMainSignature mainEntryPoint = (TundraPluginMainSignature)GetProcAddress(module, "TundraPluginMain");
    if (mainEntryPoint == NULL)
    {
        DWORD errorCode = GetLastError();
        LogError(QString("Failed to find plugin startup function 'TundraPluginMain' from plugin file \"%1\": %2").arg(path).arg(GetErrorString(errorCode).c_str()));
        return;
    }
#else
    const char *dlerrstr;
    dlerror();
    void *module = dlopen(path.toStdString().c_str(), RTLD_GLOBAL|RTLD_LAZY);
    if ((dlerrstr=dlerror()) != 0)
    {
        LogError("Failed to load plugin from file \"" + path + "\": Error " + dlerrstr + "!");
        return;
    }

    dlerror();
    TundraPluginMainSignature mainEntryPoint = (TundraPluginMainSignature)dlsym(module, "TundraPluginMain");
    if ((dlerrstr=dlerror()) != 0)
    {
        LogError("Failed to find plugin startup function 'TundraPluginMain' from plugin file \"" + path + "\": Error " + dlerrstr + "!");
        return;
    }
#endif
    Plugin p = { module, filename, path };
    plugins.push_back(p);
    mainEntryPoint(owner);
}

void PluginAPI::UnloadPlugins()
{
    for(std::list<Plugin>::reverse_iterator iter = plugins.rbegin(); iter != plugins.rend(); ++iter)
    {
#ifdef WIN32
        FreeLibrary((HMODULE)iter->handle);
#else
    /// \bug caused memory errors in destructors in the dlclose call chain
    //        dlclose(iter->handle);
#endif
    }
    plugins.clear();
}

void PluginAPI::ListPlugins() const
{
    LogInfo("Loaded plugins:");
    foreach(const Plugin &plugin, plugins)
        LogInfo(plugin.name);
}

QString LookupRelativePath(QString path)
{
    // If a relative path was specified, lookup from cwd first, then from application installation directory.
    if (QDir::isRelativePath(path))
    {
        QString cwdPath = Application::CurrentWorkingDirectory() + path;
        if (QFile::exists(cwdPath))
            return cwdPath;
        else
            return Application::InstallationDirectory() + path;
    }
    else
        return path;
}

QStringList PluginAPI::ConfigurationFiles() const
{
    QStringList configs;
    QStringList cmdLineParams = owner->CommandLineParameters("--config");
    if (cmdLineParams.size() > 0)
        foreach(const QString &config, cmdLineParams)
            configs.append(LookupRelativePath(config));
    return configs;
}

void PluginAPI::LoadPluginsFromXML(QString pluginConfigurationFile)
{
    bool showDeprecationWarning = true;
    pluginConfigurationFile = LookupRelativePath(pluginConfigurationFile);

    QDomDocument doc("plugins");
    QFile file(pluginConfigurationFile);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("PluginAPI::LoadPluginsFromXML: Failed to open file \"" + pluginConfigurationFile + "\"!");
        return;
    }
    QString errorMsg;
    if (!doc.setContent(&file, false, &errorMsg))
    {
        LogError("PluginAPI::LoadPluginsFromXML: Failed to parse XML file \"" + pluginConfigurationFile + "\":" + errorMsg);
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    while(!n.isNull())
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (!e.isNull() && e.tagName() == "plugin" && e.hasAttribute("path"))
        {
            QString pluginPath = e.attribute("path");
            LoadPlugin(pluginPath);
            if (showDeprecationWarning)
            {
                LogWarning("PluginAPI::LoadPluginsFromXML: In file " + pluginConfigurationFile + ", using XML tag <plugin path=\"PluginNameHere\"/> will be deprecated. Consider replacing it with --plugin command line argument instead");
                showDeprecationWarning = false;
            }
        }
        n = n.nextSibling();
    }
}

void PluginAPI::LoadPluginsFromCommandLine()
{
    if (!owner->HasCommandLineParameter("--plugin"))
        return;

    QStringList plugins = owner->CommandLineParameters("--plugin");
    foreach(QString plugin, plugins)
    {
        plugin = plugin.trimmed();
        if (!plugin.contains(";"))
            LoadPlugin(plugin);
        else
        {
            foreach(const QString &pluginIter, plugin.simplified().replace(" ", "").split(";", QString::SkipEmptyParts))
                LoadPlugin(pluginIter);
        }
    }
}
