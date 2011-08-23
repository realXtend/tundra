// For conditions of distribution and use, see copyright notice in license.txt

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

#include "InputAPI.h"
#include "AssetAPI.h"
#include "AudioAPI.h"
#include "ConsoleAPI.h"
#include "SceneAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"

#include <iostream>
#include <QDir>

#include "MemoryLeakCheck.h"

Framework *Framework::instance = 0;

Framework::Framework(int argc, char** argv) :
    exit_signal_(false),
    argc_(argc),
    argv_(argv),
    headless_(false),
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
    renderer(0),
    apiVersionInfo(0),
    applicationVersionInfo(0)
{    // Remember this Framework instance in a static pointer. Note that this does not help visibility for external DLL code linking to Framework.
    instance = this;

    // Api/Application name and version. Can be accessed via ApiVersionInfo() and ApplicationVersionInfo().
    /// @note Modify these values when you are making a custom Tundra. Also the version needs to be changed here on releases.
    apiVersionInfo = new ApiVersionInfo(2, 0, 0, 0);
    applicationVersionInfo = new ApplicationVersionInfo(2, 0, 0, 0, "realXtend", "Tundra");

    ///\todo We cannot specify all commands here, since it is not extensible. Generate a method for modules to specify their own options (probably
    /// best is to have them parse their own options).
    cmdLineDescs["--help"] = "Produce help message"; // Framework
    cmdLineDescs["--headless"] = "Run in headless mode without any windows or rendering"; // Framework & OgreRenderingModule
    cmdLineDescs["--disablerunonload"] = "Do not start script applications (EC_Script's with applicationName defined) automatically";
    cmdLineDescs["--server"] = "Start Tundra server";
    cmdLineDescs["--port"] = "Start server in the specified port"; // TundraLogicModule
    cmdLineDescs["--protocol"] = "Start server with the specified protocol. Options: '--protocol tcp' and '--protocol udp'. Defaults to tcp if no protocol is spesified."; // KristalliProtocolModule
    cmdLineDescs["--fpslimit"] = "Specifies the fps cap to use in rendering. Default: 60. Pass in 0 to disable"; // OgreRenderingModule
    cmdLineDescs["--run"] = "Run script on startup"; // JavaScriptModule
    cmdLineDescs["--file"] = "Load scene on startup. Accepts absolute and relative paths, local:// and http:// are accepted and fetched via the AssetAPI."; // TundraLogicModule & AssetModule
    cmdLineDescs["--storage"] = "Adds the given directory as a local storage directory on startup"; // AssetModule
    cmdLineDescs["--config"] = "Specifies the startup configration file to use"; // Framework
    cmdLineDescs["--login"] = "Automatically login to server using provided data. Url syntax: {tundra|http|https}://host[:port]/?username=x[&password=y&avatarurl=z&protocol={udp|tcp}]. Minimum information needed to try a connection in the url are host and username";
    cmdLineDescs["--clear-asset-cache"] = "At the start of Tundra, remove all data and metadata files from asset cache.";

    if (HasCommandLineParameter("--help"))
    {
        std::cout << "Supported command line arguments: " << std::endl;
        ///\todo Maybe use some custom struct for command line arguments and implement ToString() function to there for pretty printing.
        QMap<QString, QString>::const_iterator it = cmdLineDescs.begin();
        while(it != cmdLineDescs.end())
        {
            int charIdx = 0;
            const int treshold = 15;
            const int maxLineWidth = 80; ///< @todo 80 on Windows, but how bout on other platforms?
            int cmdLength = it.key().length();
            std::cout << it.key().toStdString();
            if (cmdLength >= treshold)
            {
                std::cout << std::endl;
                for(charIdx = 0; charIdx < treshold ; ++charIdx)
                    std::cout << " ";
            }
            else
                for(charIdx = cmdLength; charIdx < treshold ; ++charIdx)
                    std::cout << " ";

            for(QString::const_iterator descIter = it.value().begin(); descIter != it.value().end(); ++descIter)
            {
                std::cout << (*descIter).toAscii();
                ++charIdx;
                if (charIdx >= maxLineWidth)
                {
                    charIdx = 0;
                    for(charIdx; charIdx < treshold ; ++charIdx)
                        std::cout << " ";
                }
            }

            std::cout << std::endl;
            ++it;
        }

        Exit();
    }
    else
    {
        if (HasCommandLineParameter("--headless"))
            headless_ = true;
#ifdef PROFILING
        profiler = new Profiler();
        PROFILE(FW_Startup);
#endif
        // Create ConfigAPI, pass application data and prepare data folder.
        config = new ConfigAPI(this);
        config->PrepareDataFolder("configuration");

        // Create QApplication
        application = new Application(this, argc_, argv_);       

        // Create core APIs
        frame = new FrameAPI(this);
        scene = new SceneAPI(this);
        asset = new AssetAPI(this, headless_);
        asset->OpenAssetCache(Application::UserDataDirectory() + QDir::separator() + "assetcache");
        ui = new UiAPI(this);
        audio = new AudioAPI(this, asset); // AudioAPI depends on the AssetAPI, so must be loaded after it.
        input = new InputAPI(this);
        plugin = new PluginAPI(this);
        console = new ConsoleAPI(this);
        console->RegisterCommand("exit", "Shuts down gracefully.", this, SLOT(Exit()));

        // Initialize SceneAPI.
        scene->Initialise();

        RegisterDynamicObject("ui", ui);
        RegisterDynamicObject("frame", frame);
        RegisterDynamicObject("input", input);
        RegisterDynamicObject("console", console);
        RegisterDynamicObject("asset", asset);
        RegisterDynamicObject("audio", audio);
        RegisterDynamicObject("application", application);
        RegisterDynamicObject("apiversion", apiVersionInfo);
        RegisterDynamicObject("applicationversion", applicationVersionInfo);
    }
}

Framework::~Framework()
{
    SAFE_DELETE(input);
    SAFE_DELETE(asset);
    SAFE_DELETE(audio);
    SAFE_DELETE(plugin);
#ifdef PROFILING
    /// \todo Deleting the profiler currently causes a crash, therefore disabled
    //SAFE_DELETE(profiler);
#endif
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
    if (exit_signal_ == true)
        return; // We've accidentally ended up to update a frame, but we're actually quitting.

    PROFILE(Framework_ProcessOneFrame);

    static tick_t clock_freq;
    static tick_t last_clocktime;

    if (!last_clocktime)
        last_clocktime = GetCurrentClockTime();

    if (!clock_freq)
        clock_freq = GetCurrentClockFreq();

    tick_t curr_clocktime = GetCurrentClockTime();
    double frametime = ((double)curr_clocktime - (double)last_clocktime) / (double) clock_freq;
    last_clocktime = curr_clocktime;

    for(size_t i = 0; i < modules.size(); ++i)
    {
        try
        {
#ifdef PROFILING
            ProfilerSection ps(("Module_" + modules[i]->Name() + "_Update").c_str());
#endif
            modules[i]->Update(frametime);
        }
        catch(const std::exception &e)
        {
            std::cout << "ProcessOneFrame caught an exception while updating module " << modules[i]->Name()
                << ": " << (e.what() ? e.what() : "(null)") << std::endl;
            LogError(std::string("ProcessOneFrame caught an exception while updating module " + modules[i]->Name()
                + ": " + (e.what() ? e.what() : "(null)")));
        }
        catch(...)
        {
            std::cout << "ProcessOneFrame caught an unknown exception while updating module " << modules[i]->Name() << std::endl;
            LogError(std::string("ProcessOneFrame caught an unknown exception while updating module " + modules[i]->Name()));
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
    if (exit_signal_)
        return;
    
    srand(time(0));

    plugin->LoadPluginsFromXML(plugin->ConfigurationFile());

    for(size_t i = 0; i < modules.size(); ++i)
    {
        LogDebug("Initializing module " + modules[i]->Name());
        modules[i]->Initialize();
    }

    // Run our QApplication subclass.
    application->Go();

    // Qt main loop execution has ended, we are exiting.
    exit_signal_ = true;

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
    audio->Reset();

    for(size_t i = 0; i < modules.size(); ++i)
    {
        LogDebug("Unloading module " + modules[i]->Name());
        modules[i]->Unload();
    }

    // Actually unload all DLLs from memory.
    modules.clear();
    plugin->UnloadPlugins();
}

void Framework::Exit()
{
    exit_signal_ = true;
    if (application)
        application->AboutToExit();
}

void Framework::ForceExit()
{
    exit_signal_ = true;
    if (application)
        application->quit();
}

void Framework::CancelExit()
{
    exit_signal_ = false;

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
/*
ConnectionAPI *Framework::Connection() const
{
    return connection;
}

ServerAPI *Framework::Server() const
{
    return server;
}
*/
PluginAPI *Framework::Plugins() const
{
    return plugin;
}

IRenderer *Framework::Renderer() const
{
    return renderer;
}

ApiVersionInfo *Framework::ApiVersion() const
{
    return apiVersionInfo;
}

ApplicationVersionInfo *Framework::ApplicationVersion() const
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
    modules.push_back(boost::shared_ptr<IModule>(module));
    module->Load();
}

IModule *Framework::GetModuleByName(const QString &name) const
{
    for(size_t i = 0; i < modules.size(); ++i)
        if (modules[i]->Name() == name.toStdString())
            return modules[i].get();

    return 0;
}

bool Framework::RegisterDynamicObject(QString name, QObject *object)
{
    if (name.length() == 0 || !object)
        return false;

    // We never override a property if it already exists.
    if (property(name.toStdString().c_str()).isValid())
        return false;

    setProperty(name.toStdString().c_str(), QVariant::fromValue<QObject*>(object));

    return true;
}

bool Framework::HasCommandLineParameter(const QString &value) const
{
    for(int i = 0; i < argc_; ++i)
        if (QString(argv_[i]) == value)
            return true;
    return false;
}

QStringList Framework::CommandLineParameters(const QString &key) const
{
    QStringList ret;
    for(int i = 0; i < argc_; ++i)
        if (QString(argv_[i]) == key && i+1 < argc_ && !QString(argv_[i+1]).startsWith("--"))
            ret.append(argv_[++i]);
    return ret;
}

