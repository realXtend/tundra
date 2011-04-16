#include "StableHeaders.h"
#include "PluginAPI.h"
#include "LoggingFunctions.h"
#include "Framework.h"
#include "Application.h"

#include <QtXml>
#include <iostream>
#include <vector>

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


PluginAPI::PluginAPI(Foundation::Framework *owner_)
:owner(owner_)
{
}

typedef void (*TundraPluginMainSignature)(Foundation::Framework *owner);

void PluginAPI::LoadPlugin(const QString &filename)
{
#ifdef _DEBUG
    const QString pluginSuffix = "d.dll";
#else
    const QString pluginSuffix = ".dll";
#endif

    QString path = Application::InstallationDirectory() + "plugins/" + filename.trimmed() + pluginSuffix;

    path = path.replace("/", "\\");
    LogInfo("Loading up plugin \"" + filename + "\" in path \"" + path + "\".");
    ///\todo Cross-platform -> void* & dlopen.
    ///\todo Unicode support!
    HMODULE module = LoadLibraryA(path.toStdString().c_str());
    if (module == NULL)
    {
        DWORD errorCode = GetLastError(); ///\todo ToString.
        LogError("Failed to load plugin from file \"" + path + "\": Error " + GetErrorString(errorCode).c_str() + "!");
        return;
    }
    TundraPluginMainSignature mainEntryPoint = (TundraPluginMainSignature)GetProcAddress(module, "TundraPluginMain");
    if (mainEntryPoint == NULL)
    {
        DWORD errorCode = GetLastError(); ///\todo ToString.
        LogError("Failed to find plugin startup function 'TundraPluginMain' from plugin file \"" + path + "\": Error " + GetErrorString(errorCode).c_str() + "!");
        return;
    }

    LogInfo("Starting up plugin \"" + path + "\".");
    mainEntryPoint(owner);
}

void PluginAPI::LoadPluginsFromXML(const QString &pluginListFilename)
{
    QDomDocument doc("plugins");
    QFile file(pluginListFilename);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("PluginAPI::LoadPluginsFromXML: Failed to open file \"" + pluginListFilename + "\"!");
        return;
    }
    if (!doc.setContent(&file))
    {
        LogError("PluginAPI::LoadPluginsFromXML: Failed to parse XML file \"" + pluginListFilename + "\"!");
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
