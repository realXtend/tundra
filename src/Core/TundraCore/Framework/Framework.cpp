// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Framework.h"
#include "Profiler.h"
#include "IRenderer.h"
#include "CoreException.h"
#include "Application.h"
#include "VersionInfo.h"
#include "ConfigAPI.h"
#include "PluginAPI.h"
#include "LoggingFunctions.h"
#include "IModule.h"
#include "FrameAPI.h"
#include "ConsoleAPI.h"

#include "InputAPI.h"
#include "AssetAPI.h"
#include "AudioAPI.h"
#include "SceneAPI.h"
#include "UiAPI.h"

#ifndef _WINDOWS
#include <sys/ioctl.h>
#endif
#ifdef ANDROID
#include "StaticPluginRegistry.h"
#include <termios.h>
#endif
#include <iostream>
#include <sstream>

#include <QDir>
#include <QDomDocument>

#include "MemoryLeakCheck.h"

#ifdef ANDROID
JavaVM* Framework::javaVM = 0;
JNIEnv* Framework::jniEnv = 0;
#endif

namespace
{
/// Temporary utility structure for storing supported command line parameters and their descriptions.
/** @cond PRIVATE */
struct CommandLineParameterMap
{
    /// Returns the command line structure in printable format.
    std::string ToString() const
    {
        std::stringstream ss;
        QMap<QString, QString>::const_iterator it = commands.begin();
        while(it != commands.end())
        {
            int charIdx = 0;
            const int treshold = 15;
#ifdef _WINDOWS
            int maxLineWidth = 80; // Initially assume default 80 on Windows.
            CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
            HANDLE hstdout;
            hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
            if (GetConsoleScreenBufferInfo(hstdout, &csbiInfo))
                maxLineWidth = csbiInfo.dwSize.X;
#else
            struct winsize w;
            ioctl(0, TIOCGWINSZ, &w);
            const int maxLineWidth = (int)w.ws_col;
#endif
            int cmdLength = it.key().length();
            ss << it.key().toStdString();
            if (cmdLength >= treshold)
            {
                ss << std::endl;
                for(charIdx = 0; charIdx < treshold ; ++charIdx)
                    ss << " ";
            }
            else
                for(charIdx = cmdLength; charIdx < treshold ; ++charIdx)
                    ss << " ";

            for(int i = 0; i < it.value().length(); ++i)
            {
                ss << it.value()[i].toAscii();
                ++charIdx;
                if (charIdx >= maxLineWidth)
                {
                    charIdx = 0;
                    for(charIdx; charIdx < treshold ; ++charIdx)
                        ss << " ";
                }
            }

            ss << std::endl;
            ++it;
        }
        return ss.str();
    }

    QMap<QString, QString> commands;
};
/** @endcond */

} //~unnamed namespace

Framework *Framework::instance = 0;

Framework::Framework(int argc_, char** argv_) :
    exitSignal(false),
    argc(argc_),
    argv(argv_),
    headless(false),
    application(0),
    frame(0),
    console(0),
    scene(0),
    input(0),
    asset(0),
    audio(0),
    plugin(0),
    config(0),
    ui(0),
#ifdef PROFILING
    profiler(0),
#endif
    profilerQObj(0),
    renderer(0),
    apiVersionInfo(0),
    applicationVersionInfo(0)
{
    // Remember this Framework instance in a static pointer. Note that this does not help visibility for external DLL code linking to Framework.
    instance = this;

#ifdef ANDROID
    LoadCommandLineFromFile();
#else
    // Remember all startup command line options.
    // Skip argv[0], since it is the program name.
    for(int i = 1; i < argc; ++i)
        startupOptions << argv[i];
#endif

    //  Load additional command line options from each config XML file.
    QStringList cmdLineParams = CommandLineParameters("--config");
    if (cmdLineParams.size() == 0) // By default, the plugins.xml file is loaded if no other config items are specified.
        LoadStartupOptionsFromXML("plugins.xml");
    foreach(const QString &config, cmdLineParams)
        LoadStartupOptionsFromXML(config);

    // Make sure we spawn a console window in each case we might need one.
    if (HasCommandLineParameter("--version") || HasCommandLineParameter("--help") || HasCommandLineParameter("--sharedconsole") || HasCommandLineParameter("--console") || HasCommandLineParameter("--headless"))
        Application::ShowConsoleWindow(HasCommandLineParameter("--sharedconsole"));

    ///\todo Delete the CommandLineParameterMap mechanism altogether.
    /// Instead, provide the command line parameter help from a help file, where all the various command line parameters can be assembled.
    CommandLineParameterMap cmdLineDescs;
#ifdef WIN32
    cmdLineDescs.commands["--console"] = "Shows a text-based console along with the main UI window.";
    cmdLineDescs.commands["--sharedConsole"] = "Same as '--console' but attaches the Tundra console to the parent process, without creating new command prompt for the console.";
    cmdLineDescs.commands["--perfHud"] = "Use Ogre with NVIDIA PerfHUD enabled, if applicable.";
    cmdLineDescs.commands["--d3d9"] = "Use Ogre with \"Direct3D9 Rendering Subsystem\", overrides the option that was set in config.";
    cmdLineDescs.commands["--direct3d9"] = "Same as --d3d9.";
#endif
    cmdLineDescs.commands["--help"] = "Produces help message."; // Framework
    cmdLineDescs.commands["--version"] = "Produces version information."; // Framework
    cmdLineDescs.commands["--headless"] = "Runs Tundra in headless mode without any windows or rendering."; // Framework
    cmdLineDescs.commands["--disableRunOnLoad"] = "Prevents script applications (EC_Script's with applicationName defined) starting automatically."; //JavascriptModule
    cmdLineDescs.commands["--server"] = "Starts Tundra as server."; // TundraLogicModule
    cmdLineDescs.commands["--port"] = "Specifies the Tundra server port."; // TundraLogicModule
    cmdLineDescs.commands["--protocol"] = "Specifies the Tundra server protocol. Options: '--protocol tcp' and '--protocol udp'. Defaults to udp if no protocol is spesified."; // KristalliProtocolModule
    cmdLineDescs.commands["--fpsLimit"] = "Specifies the FPS cap to use in rendering. Default: 60. Pass in 0 to disable."; // Framework
    cmdLineDescs.commands["--run"] = "Runs script on startup"; // JavaScriptModule
    cmdLineDescs.commands["--file"] = "Specifies a startup scene file. Multiple files supported. Accepts absolute and relative paths, local:// and http:// are accepted and fetched via the AssetAPI."; // TundraLogicModule & AssetModule
    cmdLineDescs.commands["--storage"] = "Adds the given directory as a local storage directory on startup."; // AssetModule
    cmdLineDescs.commands["--config"] = "Specifies a startup configration file to use. Multiple config files are supported, f.ex. '--config plugins.xml --config MyCustomAddons.xml'."; // Framework & PluginAPI
    cmdLineDescs.commands["--connect"] = "Connects to a Tundra server automatically. Syntax: '--connect serverIp;port;protocol;name;password'. Password is optional."; // TundraLogicModule & AssetModule
    cmdLineDescs.commands["--login"] = "Automatically login to server using provided data. Url syntax: {tundra|http|https}://host[:port]/?username=x[&password=y&avatarurl=z&protocol={udp|tcp}]. Minimum information needed to try a connection in the url are host and username."; // TundraLogicModule & AssetModule
    cmdLineDescs.commands["--netRate"] = "Specifies the number of network updates per second. Default: 30."; // TundraLogicModule
    cmdLineDescs.commands["--noAssetCache"] = "Disable asset cache."; // Framework
    cmdLineDescs.commands["--assetCacheDir"] = "Specify asset cache directory to use."; // Framework
    cmdLineDescs.commands["--clear-asset-cache"] = "At the start of Tundra, remove all data and metadata files from asset cache."; // AssetCache
    cmdLineDescs.commands["--logLevel"] = "Sets the current log level: 'error', 'warning', 'info', 'debug'."; // ConsoleAPI
    cmdLineDescs.commands["--logFile"] = "Sets logging file. Usage example: '--logfile TundraLogFile.txt'."; // ConsoleAPI
    cmdLineDescs.commands["--physicsRate"] = "Specifies the number of physics simulation steps per second. Default: 60."; // PhysicsModule
    cmdLineDescs.commands["--physicsMaxSteps"] = "Specifies the maximum number of physics simulation steps in one frame to limit CPU usage. If the limit would be exceeded, physics will appear to slow down. Default: 6."; // PhysicsModule
    cmdLineDescs.commands["--splash"] = "Shows splash screen during the startup."; // Framework
    cmdLineDescs.commands["--fullscreen"] = "Starts application in fullscreen mode."; // OgreRenderingModule
    cmdLineDescs.commands["--vsync"] = "Synchronizes buffer swaps to monitor vsync, eliminating tearing at the expense of a fixed frame rate."; // OgreRenderingModule
    cmdLineDescs.commands["--vsyncFrequency"] = "Sets display frequency rate for vsync, applicable only if fullscreen is set. Usage: '--vsyncFrequency <number>'."; // OgreRenderingModule
    cmdLineDescs.commands["--antialias"] = "Sets full screen antialiasing factor. Usage '--antialias <number>'."; // OgreRenderingModule
    cmdLineDescs.commands["--hide_benign_ogre_messages"] = "Sets some uninformative Ogre log messages to be ignored from the log output."; // OgreRenderingModule
    cmdLineDescs.commands["--no_async_asset_load"] = "Disables threaded loading of Ogre assets."; // OgreRenderingModule
    cmdLineDescs.commands["--autoDxtCompress"] = "Compress uncompressed texture assets to DXT1/DXT5 format on load to save memory."; // OgreRenderingModule
    cmdLineDescs.commands["--maxTextureSize"] = "Resize texture assets that are larger than this. Default: no resizing."; // OgreRenderingModule
    cmdLineDescs.commands["--variablePhysicsStep"] = "Use variable physics timestep to avoid taking multiple physics substeps during one frame."; // PhysicsModule
    cmdLineDescs.commands["--opengl"] = "Use Ogre with \"OpenGL Rendering Subsystem\" for rendering, overrides the option that was set in config.";
    cmdLineDescs.commands["--nullRenderer"] = "Disables all Ogre rendering operations."; // OgreRenderingModule
    cmdLineDescs.commands["--ogreCaptureTopWindow"] = "On some systems, the Ogre rendering output is overdrawn by the desktop compositing manager, "
        "but the actual cause of this is uncertain. As a workaround, try this switch to make Ogre output directly on the main window handle of the UI chain. "
        "However, this might introduce graphical issues."; // OgreRenderingModule
    cmdLineDescs.commands["--noUiCompositing"] = "Disables the UI compositing, use for debugging purposes only."; // Framework & OgreRenderingModule
    cmdLineDescs.commands["--noCentralWidget"] = "Disables the usage of QMainWindow's central widget."; // Framework
    cmdLineDescs.commands["--noMenuBar"] = "Disables showing of the application menu bar automatically."; // Framework
    cmdLineDescs.commands["--clientExtrapolationTime"] = "Rigidbody extrapolation time on client in milliseconds. Default 66."; // TundraProtocolModule
    cmdLineDescs.commands["--noClientPhysics"] = "Disables rigidbody handoff to client simulation after no movement packets received from server."; // TundraProtocolModule
    cmdLineDescs.commands["--dumpProfiler"] = "Dump profiling blocks to console every 5 seconds."; // DebugStatsModule
    
    apiVersionInfo = new VersionInfo(Application::Version());
    applicationVersionInfo = new VersionInfo(Application::Version());

    LogInfo("* API version         : " + apiVersionInfo->Version());
    LogInfo("* Application version : " + Application::FullIdentifier());

    if (HasCommandLineParameter("--help"))
    {
        LogInfo("Supported command line arguments (case-insensitive):");
        std::cout << cmdLineDescs.ToString();
    }

    if (HasCommandLineParameter("--version") || HasCommandLineParameter("--help"))
    {
#ifdef WIN32
        std::cout << std::endl;
        system("pause");
#endif
        Exit();
        return;
    }

    PrintStartupOptions();

    // In headless mode, no main UI/rendering window is initialized.
    if (HasCommandLineParameter("--headless"))
        headless = true;

#ifdef PROFILING
    profiler = new Profiler();
    PROFILE(FW_Startup);
#endif
    profilerQObj = new ProfilerQObj;

    // Create ConfigAPI, pass application data and prepare data folder.
    config = new ConfigAPI(this);
    QStringList configDirs = CommandLineParameters("--configdir");
    QString configDir = "$(USERDATA)/configuration"; // The default configuration goes to "C:\Users\username\AppData\Roaming\Tundra\configuration"
    if (configDirs.size() >= 1)
        configDir = configDirs.last();
    if (configDirs.size() > 1)
        LogWarning("Multiple --configdir parameters specified! Using \"" + configDir + "\" as the configuration directory.");
    config->PrepareDataFolder(configDir);

    // Create QApplication, set target FPS limit, if specified.
    application = new Application(this, argc, argv);
    QStringList fpsLimitParam = CommandLineParameters("--fpslimit");
    if (fpsLimitParam.size() > 1)
        LogWarning("Multiple --fpslimit parameters specified! Using " + fpsLimitParam.first() + " as the value.");
    if (fpsLimitParam.size() > 0)
    {
        bool ok;
        double targetFpsLimit = fpsLimitParam.first().toDouble(&ok);
        if (ok)
            application->SetTargetFpsLimit(targetFpsLimit);
        else
            LogWarning("Erroneous FPS limit given with --fpslimit: " + fpsLimitParam.first() + ". Ignoring.");
    }

    // Create core APIs
    frame = new FrameAPI(this);
    scene = new SceneAPI(this);
    plugin = new PluginAPI(this);
    asset = new AssetAPI(this, headless);
    // Prepare asset cache, if used.
    QString assetCacheDir = Application::UserDataDirectory() + QDir::separator() + "assetcache";
    if (CommandLineParameters("--assetcachedir").size() > 0)
        assetCacheDir = Application::ParseWildCardFilename(CommandLineParameters("--assetcachedir").last());
    if (CommandLineParameters("--assetcachedir").size() > 1)
        LogWarning("Multiple --assetcachedir parameters specified! Using \"" + CommandLineParameters("--assetcachedir").last() + "\" as the assetcache directory.");
    if (!HasCommandLineParameter("--noassetcache"))
        asset->OpenAssetCache(assetCacheDir);

    ui = new UiAPI(this);
    audio = new AudioAPI(this, asset); // AudioAPI depends on the AssetAPI, so must be loaded after it.
    input = new InputAPI(this);
    console = new ConsoleAPI(this);
    console->RegisterCommand("exit", "Shuts down gracefully.", this, SLOT(Exit()));
    console->RegisterCommand("inputContexts", "Prints all currently registered input contexts in InputAPI.", input, SLOT(DumpInputContexts()));
    console->RegisterCommand("dynamicObjects", "Prints all currently registered dynamic objets in Framework.", this, SLOT(PrintDynamicObjects()));

    RegisterDynamicObject("ui", ui);
    RegisterDynamicObject("frame", frame);
    RegisterDynamicObject("input", input);
    RegisterDynamicObject("console", console);
    RegisterDynamicObject("asset", asset);
    RegisterDynamicObject("audio", audio);
    RegisterDynamicObject("application", application);
    RegisterDynamicObject("config", config);
    RegisterDynamicObject("apiversion", apiVersionInfo);
    RegisterDynamicObject("applicationversion", applicationVersionInfo);
    RegisterDynamicObject("profiler", profilerQObj);
}

Framework::~Framework()
{
    SAFE_DELETE(input);
    SAFE_DELETE(asset);
    SAFE_DELETE(audio);
    SAFE_DELETE(plugin);
#ifdef PROFILING
    SAFE_DELETE(profiler);
#endif
    SAFE_DELETE(profilerQObj);

    SAFE_DELETE(console);
    SAFE_DELETE(scene);
    SAFE_DELETE(frame);
    SAFE_DELETE(ui);

    SAFE_DELETE(apiVersionInfo);
    SAFE_DELETE(applicationVersionInfo);

    // This delete must be the last one in Framework since application derives QApplication.
    // When we delete QApplication, we must have ensured that all QObjects have been deleted.
    /// \bug Framework is itself a QObject and we should delete application only after Framework has been deleted. A refactor is required.
    delete application;
}

void Framework::ProcessOneFrame()
{
    if (exitSignal == true)
        return; // We've accidentally ended up to update a frame, but we're actually quitting.

    PROFILE(Framework_ProcessOneFrame);

    static tick_t clockFreq;
    static tick_t lastClockTime;

    if (!lastClockTime)
        lastClockTime = GetCurrentClockTime();

    if (!clockFreq)
        clockFreq = GetCurrentClockFreq();

    tick_t currClockTime = GetCurrentClockTime();
    double frametime = ((double)currClockTime - (double)lastClockTime) / (double) clockFreq;
    lastClockTime = currClockTime;

    for(size_t i = 0; i < modules.size(); ++i)
    {
        try
        {
#ifdef PROFILING
            ProfilerSection ps(("Module_" + modules[i]->Name() + "_Update").toStdString());
#endif
            modules[i]->Update(frametime);
        }
        catch(const std::exception &e)
        {
            std::stringstream error;
            error << "ProcessOneFrame caught an exception while updating module " << modules[i]->Name().toStdString() << ": " << (e.what() ? e.what() : "(null)");
            std::cout << error.str() << std::endl;
            LogError(error.str());
        }
        catch(...)
        {
            std::string error("ProcessOneFrame caught an unknown exception while updating module " + modules[i]->Name().toStdString());
            std::cout << error << std::endl;
            LogError(error);
        }
    }

    asset->Update(frametime);
    input->Update(frametime);
    audio->Update(frametime);
    console->Update(frametime);
    frame->Update(frametime);

    if (renderer)
        renderer->Render(frametime);
}

void Framework::Go()
{
    // Check if we were never supposed to run
    if (exitSignal)
        return;
    
    srand(time(0));

#ifdef ANDROID
    // Run any statically registered plugin main functions first
    StaticPluginRegistryInstance()->RunPluginMainFunctions(this);
#endif

    foreach(const QString &config, plugin->ConfigurationFiles())
    {
        LogDebug("Loading plugins from config XML " + config);
        plugin->LoadPluginsFromXML(config);
    }

    for(size_t i = 0; i < modules.size(); ++i)
    {
        LogDebug("Initializing module " + modules[i]->Name());
        modules[i]->Initialize();
    }

    // Run our QApplication subclass.
    application->Go();

    // Qt main loop execution has ended, we are exiting.
    exitSignal = true;

    for(size_t i = 0; i < modules.size(); ++i)
    {
        LogDebug("Uninitializing module " + modules[i]->Name());
        modules[i]->Uninitialize();
    }

    // Deinitialize all core APIs.
    scene->Reset();
    asset->Reset();
    console->Reset();
    frame->Reset();
    input->SaveKeyBindingsToFile();
    input->Reset();
    audio->SaveSoundSettingsToConfig();
    audio->Reset();

    for(size_t i = 0; i < modules.size(); ++i)
    {
        LogDebug("Unloading module " + modules[i]->Name());
        modules[i]->Unload();
    }

    // Delete all modules.
    modules.clear();

    // Now that each module has been deleted, they've closed all their windows as well. Tear down the main UI.
    ui->Reset();

    // Actually unload all DLL plugins from memory.
    plugin->UnloadPlugins();
}

void Framework::Exit()
{
    exitSignal = true;
    if (application)
        application->AboutToExit();
}

void Framework::ForceExit()
{
    exitSignal = true;
    if (application)
        application->quit();
}

void Framework::CancelExit()
{
    exitSignal = false;

    // Our main loop is stopped when we are exiting,
    // we need to start it back up again if something canceled the exit.
    if (application)
        application->UpdateFrame();
}

Application *Framework::App() const
{
    return application;
}

#ifdef PROFILING
Profiler *Framework::GetProfiler() const
{
    return profiler;
}
#endif

FrameAPI *Framework::Frame() const
{
    return frame;
}

InputAPI *Framework::Input() const
{
    return input;
}

UiAPI *Framework::Ui() const
{
    return ui;
}

ConsoleAPI *Framework::Console() const
{
    return console;
}

AudioAPI *Framework::Audio() const
{
    return audio;
}

AssetAPI *Framework::Asset() const
{
    return asset;
}

SceneAPI *Framework::Scene() const
{
    return scene;
}

ConfigAPI *Framework::Config() const
{
    return config;
}

PluginAPI *Framework::Plugins() const
{
    return plugin;
}

IRenderer *Framework::Renderer() const
{
    return renderer;
}

VersionInfo *Framework::ApiVersion() const
{
    return apiVersionInfo;
}

VersionInfo *Framework::ApplicationVersion() const
{
    return applicationVersionInfo;
}

void Framework::RegisterRenderer(IRenderer *renderer_)
{
    renderer = renderer_;
}

void Framework::RegisterModule(IModule *module)
{
    module->SetFramework(this);
    modules.push_back(shared_ptr<IModule>(module));
    module->Load();
}

IModule *Framework::GetModuleByName(const QString &name) const
{
    for(size_t i = 0; i < modules.size(); ++i)
        if (modules[i]->Name() == name)
            return modules[i].get();
    return 0;
}

bool Framework::RegisterDynamicObject(QString name, QObject *object)
{
    if (name.length() == 0 || !object)
    {
        LogError("Framework::RegisterDynamicObject: empty name or null object passed.");
        return false;
    }
    if (property(name.toStdString().c_str()).isValid()) // We never override a property if it already exists.
    {
        LogError(QString("Framework::RegisterDynamicObject: Dynamic object with name \"%1\" already registered.").arg(name));
        return false;
    }

    setProperty(name.toStdString().c_str(), QVariant::fromValue<QObject*>(object));

    return true;
}

QString LookupRelativePath(QString path);

void Framework::LoadStartupOptionsFromXML(QString configurationFile)
{
    configurationFile = LookupRelativePath(configurationFile);

    QDomDocument doc("plugins");
    QFile file(configurationFile);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("Framework::LoadStartupOptionsFromXML: Failed to open file \"" + configurationFile + "\"!");
        return;
    }
    QString errorMsg;
    if (!doc.setContent(&file, false, &errorMsg))
    {
        LogError("Framework::LoadStartupOptionsFromXML: Failed to parse XML file \"" + configurationFile + "\": " + errorMsg + "!");
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    while(!n.isNull())
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (!e.isNull() && e.tagName() == "option" && e.hasAttribute("name"))
        {
#ifdef _DEBUG
            QString build = "debug";
#else
            QString build = "release";
#endif
            if (e.hasAttribute("build") && build.compare(e.attribute("build"), Qt::CaseInsensitive) != 0)
                continue; // The command line parameter was specified to be included only in the given build (debug/release), but we are not running that build.

            /// \bug Appended in this way, the parsing is not perfect (one configuration can continue from the other)
            startupOptions << e.attribute("name");

            if (e.hasAttribute("value"))
                startupOptions << e.attribute("value");
        }
        n = n.nextSibling();
    }
}

bool Framework::HasCommandLineParameter(const QString &value) const
{
    ///\todo Convert startupOptions to a key-value map.
    for(int i = 0; i < startupOptions.size(); ++i)
        if (!startupOptions[i].compare(value, Qt::CaseInsensitive))
            return true;
    return false;
}

QStringList Framework::CommandLineParameters(const QString &key) const
{
    ///\todo Remove all this logic. This is Win32-specific command line parsing, and should be done only once for Win32,
    /// and stored in an already processed format for faster retrieval.
    QStringList ret;
    for(int i = 0; i+1 < startupOptions.size(); ++i)
    {
        if (!startupOptions[i].compare(key, Qt::CaseInsensitive) && !startupOptions[i+1].startsWith("--"))
        {
            // Inspect for quoted parameters.
            QString quotedParam = startupOptions[i+1];
            if (quotedParam.startsWith("\""))
            {
                if (quotedParam.endsWith("\"")) // End quote is in the argv
                {
                    // Remove quotes and append to the return list.
                    quotedParam = quotedParam.right(quotedParam.length() -1);
                    quotedParam.chop(1);
                    ret.append(quotedParam);
                }
                else // End quote is not in the same argv
                {
                    for(int pi=i+2; pi+1 < startupOptions.size(); ++pi)
                    {
                        QString param = startupOptions[pi];

                        // If a new -- key is found before an end quote we have a error.
                        // Report and don't add anything to the return list as the param is malformed.
                        if (param.startsWith("--"))
                        {
                            LogError("Could not find an end quote for '" + key + "' parameter: " + quotedParam);
                            i = pi - 1; // Step one back so the main for loop will inspect this element next.
                            break;
                        }
                        // We found the end of the quoted param.
                        // Remove quotes and append to the return list.
                        else if (param.endsWith("\""))
                        {
                            i = pi; // Set the main for loops index so it will process the proper elements next.
                            quotedParam += " " + param;
                            if (quotedParam.startsWith("\""))
                                quotedParam = quotedParam.right(quotedParam.length() -1);
                            if (quotedParam.endsWith("\""))
                                quotedParam.chop(1);
                            ret.append(quotedParam);
                            break;
                        }
                        else // Append to param.
                            quotedParam += " " + param;
                    }
                }
            }
            else // No quote start, push as is
                ret.append(startupOptions[++i]);
        }
    }
    return ret;
}

void Framework::PrintStartupOptions()
{
    int i = 0;
    LogInfo("Startup options:");
    while(i < startupOptions.size())
    {
        if (!startupOptions[i].startsWith("--"))
            LogWarning("Warning: Orphaned startup option parameter value \"" + startupOptions[i] + "\" specified!");
        if (i+1 < startupOptions.size() && !startupOptions[i+1].startsWith("--"))
        {
            LogInfo("   '" + startupOptions[i] + "' '" + startupOptions[i+1] + "'");
            i += 2;
        }
        else
        {
            LogInfo("   '" + startupOptions[i] + "'");
            ++i;
        }
    }
}

void Framework::PrintDynamicObjects()
{
    LogInfo("Dynamic objects:");
    foreach(const QByteArray &obj, dynamicPropertyNames())
        LogInfo(QString(obj));
}

#ifdef ANDROID
StaticPluginRegistry* Framework::StaticPluginRegistryInstance()
{
    static StaticPluginRegistry* instance = new StaticPluginRegistry();
    return instance;
}

void Framework::LoadCommandLineFromFile()
{
    QFile file(Application::InstallationDirectory() + "commandline.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull())
    {
        int i;
        unsigned cmdStart = 0;
        unsigned cmdEnd = 0;
        bool cmd = false;
        bool quote = false;

        for(i = 0; i < line.length(); ++i)
        {
            if (line[i] == '\"')
                quote = !quote;
            if ((line[i] == ' ') && (!quote))
            {
                if (cmd)
                {
                    cmd = false;
                    cmdEnd = i;
                    startupOptions << line.mid(cmdStart, cmdEnd-cmdStart);
                }
            }
            else
            {
                if (!cmd)
                {
                   cmd = true;
                   cmdStart = i;
                }
            }
        }
        if (cmd)
            startupOptions << line.mid(cmdStart, i-cmdStart);

        line = in.readLine();
    }
}

#endif
