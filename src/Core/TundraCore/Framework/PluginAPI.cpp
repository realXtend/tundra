// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "PluginAPI.h"
#include "LoggingFunctions.h"
#include "Framework.h"
#include "Application.h"

#include <QtXml>

#include <vector>
#include <sstream>

std::string WStringToString(const std::wstring &str)
{
    std::vector<char> c((str.length()+1)*4);
    wcstombs(&c[0], str.c_str(), c.size()-1);
    return &c[0];
}

std::string GetErrorString(int error)
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

    LogInfo("Loading plugin '" + filename + "'.");
    owner->App()->SetSplashMessage("Loading plugin " + filename);
    QString path = Application::InstallationDirectory() + "plugins/" + filename.trimmed() + pluginSuffix;

    ///\todo Unicode support!
#ifdef WIN32
    path = path.replace("/", "\\");
    HMODULE module = LoadLibraryA(path.toStdString().c_str());
    if (module == NULL)
    {
        DWORD errorCode = GetLastError();
        LogError("Failed to load plugin from file \"" + path + "\": Error " + GetErrorString(errorCode).c_str() + "!");
        return;
    }
    TundraPluginMainSignature mainEntryPoint = (TundraPluginMainSignature)GetProcAddress(module, "TundraPluginMain");
    if (mainEntryPoint == NULL)
    {
        DWORD errorCode = GetLastError();
        LogError("Failed to find plugin startup function 'TundraPluginMain' from plugin file \"" + path + "\": Error " + GetErrorString(errorCode).c_str() + "!");
        return;
    }
#else
    const char *dlerrstr;
    dlerror();
    PluginHandle module = dlopen(path.toStdString().c_str(), RTLD_GLOBAL|RTLD_LAZY);
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
    Plugin p = { module };
    plugins.push_back(p);
    mainEntryPoint(owner);
}

void PluginAPI::UnloadPlugins()
{
    for(std::list<Plugin>::reverse_iterator iter = plugins.rbegin(); iter != plugins.rend(); ++iter)
#ifdef WIN32
        FreeLibrary(iter->libraryHandle);
#else
    /// \bug caused memory errors in destructors in the dlclose call chain
    //        dlclose(iter->libraryHandle);
#endif
    plugins.clear();
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

void PluginAPI::LoadPluginsFromXML(QString pluginConfigurationFile)
{
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
        }
        n = n.nextSibling();
    }
}
