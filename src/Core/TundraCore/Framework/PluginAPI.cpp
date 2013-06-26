// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "PluginAPI.h"
#include "LoggingFunctions.h"
#include "Framework.h"
#include "Application.h"

#include <QtXml>
#include <QDir>
#include <QFile>

#include <vector>
#include <sstream>

#ifdef WIN32
#include "Win.h"
#elif defined(_POSIX_C_SOURCE) || defined(Q_WS_MAC) || defined(ANDROID)
#include <dlfcn.h>
#endif

/// @todo Move to CoreStringUtils?
static std::string WStringToString(const std::wstring &str)
{
    std::vector<char> c((str.length()+1)*4);
    wcstombs(&c[0], str.c_str(), c.size()-1);
    return &c[0];
}

/// @todo Move to SystemInfo?
static std::string GetErrorString(int error)
{
#ifdef WIN32
    void *lpMsgBuf = 0;

    HRESULT hresult = HRESULT_FROM_WIN32(error);
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        0, hresult, 0 /*Default language*/, (LPTSTR) &lpMsgBuf, 0, 0);

    // Copy message to C++ -style string, since the data need to be freed before return.
#ifdef UNICODE
    std::wstringstream ss;
#else
    std::stringstream ss;
#endif
    ss << (LPTSTR)lpMsgBuf << "(" << error << ")";
    LocalFree(lpMsgBuf);
#ifdef UNICODE
    return WStringToString(ss.str());
#else
    return ss.str();
#endif

#else
    std::stringstream ss;
    ss << strerror(error) << "(" << error << ")";
    return ss.str();
#endif
}

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

    ///\todo Unicode support!
#ifdef WIN32
    HMODULE module = LoadLibraryA(path.toStdString().c_str());
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
#ifdef WIN32
        FreeLibrary((HMODULE)iter->handle);
#else
    /// \bug caused memory errors in destructors in the dlclose call chain
    //        dlclose(iter->handle);
#endif
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
    QString defaultConfigFilename = "plugins.xml";
    QStringList cmdLineParams = owner->CommandLineParameters("--config");
    if (cmdLineParams.size() > 0)
        foreach(const QString &config, cmdLineParams)
            configs.append(LookupRelativePath(config));
    else
        configs.append(LookupRelativePath(defaultConfigFilename));

    return configs;
}

void PluginAPI::LoadPluginsFromCommandLine()
{
    if (owner->HasCommandLineParameter("--plugin"))
    {
        QStringList pluginPaths = owner->CommandLineParameters("--plugin");

        for (int i = 0; i < pluginPaths.size(); ++i)
        {
            QStringList pluginList = pluginPaths.at(i).simplified().replace(" ", "").split(";", QString::SkipEmptyParts);
            for (int j = 0; j < pluginList.size(); ++j)
                LoadPlugin(pluginList.at(j));
        }
    }
    else
        LogError("PluginAPI::LoadPluginsFromCommandLine: No plugins were specified.");
}
