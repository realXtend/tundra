// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Framework.h"
#include "Profiler.h"
#include "IRenderer.h"
#include "CoreException.h"

#include "Application.h"

#include "InputAPI.h"
#include "FrameAPI.h"
#include "AssetAPI.h"
#include "AudioAPI.h"
#include "ConsoleAPI.h"
#include "SceneAPI.h"
#include "ConfigAPI.h"
#include "PluginAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "LoggingFunctions.h"
#include "IModule.h"
#include <iostream>

#include "MemoryLeakCheck.h"

Framework *Framework::instance = 0;

Framework::Framework(int argc, char** argv) :
    exit_signal_(false),
    argc_(argc),
    argv_(argv),
    headless_(false),
    application(0),
    frame(new FrameAPI(this)),
    console(0),
    scene(new SceneAPI(this)),
    input(0),
    asset(0),
    audio(0),
    plugin(0),
    ui(0),
//    connection(0),
//    server(0),
    renderer(0)
{
    // Remember this Framework instance in a static pointer. Note that this does not help visibility for external DLL code linking to Framework.
    instance = this;

    // Application name and version. Can be accessed via ConfigAPI.
    /// \note Modify these values when you are making a custom Tundra. Also the version needs to be changed here on releases.
    const QString applicationOrganization = "realXtend";
    const QString applicationName = "Tundra";
    const QString applicationVersion = "2.0";

    ParseProgramOptions();

    if (commandLineVariables.count("help")) 
    {
        std::cout << "Supported command line arguments: " << std::endl;
        std::cout << commandLineDescriptions << std::endl;
    }
    else
    {
        if (commandLineVariables.count("headless"))
            headless_ = true;
#ifdef PROFILING
        profiler = new Profiler();
        PROFILE(FW_Startup);
#endif

        // Create QApplication
        application = new Application(this, argc_, argv_);

        // Create ConfigAPI and pass application data.
        config = new ConfigAPI(this);
        config->SetApplication(applicationOrganization, applicationName, applicationVersion);

        // Prepare ConfigAPIs working directory
        config->PrepareDataFolder("configuration");

        // Create core APIs
        asset = new AssetAPI(this, headless_);
        asset->OpenAssetCache(Application::UserDataDirectory() + "assetcache");
        ui = new UiAPI(this);
        audio = new AudioAPI(this, asset); // AudioAPI epends on the AssetAPI, so must be loaded after it.
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
    }
}

Framework::~Framework()
{
    // Delete the QObjects that don't have a parent.
    delete input;
    delete asset;
    delete audio;
    delete plugin;
    delete ui;
#ifdef PROFILING
    delete profiler;
#endif

    // This delete must be the last one in Framework since application derives QApplication.
    // When we delete QApplication, we must have ensured that all QObjects have been deleted.
    ///\bug Framework is itself a QObject and we should delete application only after Framework has been deleted. A refactor is required.
    delete application;
}

void Framework::ParseProgramOptions()
{
    namespace po = boost::program_options;

    ///\todo We cannot specify all commands here, since it is not extensible. Generate a method for modules to specify their own options (probably
    /// best is to have them parse their own options).
    commandLineDescriptions.add_options()
        ("help", "Produce help message") // Framework
        ("headless", "Run in headless mode without any windows or rendering") // Framework & OgreRenderingModule
        ("server", "Start Tundra server")
        ("port", po::value<int>(0), "Start server in the specified port") // TundraLogicModule
        ("protocol", po::value<std::string>(), "Start server with the specified protocol. Options: '--protocol tcp' and '--protocol udp'. Defaults to tcp if no protocol is spesified.") // KristalliProtocolModule
        ("fpslimit", po::value<float>(0), "Specifies the fps cap to use in rendering. Default: 60. Pass in 0 to disable") // OgreRenderingModule
        ("run", po::value<std::string>(), "Run script on startup") // JavaScriptModule
        ("file", po::value<std::string>(), "Load scene on startup. Accepts absolute and relative paths, local:// and http:// are accepted and fetched via the AssetAPI.") // TundraLogicModule & AssetModule
        ("storage", po::value<std::string>(), "Adds the given directory as a local storage directory on startup") // AssetModule
        ("config", po::value<std::string>(), "Specifies the startup configration file to use") // Framework
        ("login", po::value<std::string>(), "Automatically login to server using provided data. Url syntax: {tundra|http|https}://host[:port]/?username=x[&password=y&avatarurl=z&protocol={udp|tcp}]. Minimum information needed to try a connection in the url are host and username");

    try
    {
        po::store(po::command_line_parser(argc_, argv_).options(commandLineDescriptions).allow_unregistered().run(), commandLineVariables);
    }
    catch(std::exception &e)
    {
        LogWarning(e.what());
    }
    po::notify(commandLineVariables);
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
            std::cout << "UpdateModules caught an exception while updating module " << modules[i]->Name()
                << ": " << (e.what() ? e.what() : "(null)") << std::endl;
            LogCritical(std::string("UpdateModules caught an exception while updating module " + modules[i]->Name()
                + ": " + (e.what() ? e.what() : "(null)")));
        }
        catch(...)
        {
            std::cout << "UpdateModules caught an unknown exception while updating module " << modules[i]->Name() << std::endl;
            LogCritical(std::string("UpdateModules caught an unknown exception while updating module " + modules[i]->Name()));
        }
    }

    asset->Update(frametime);
    input->Update(frametime);
    audio->Update(frametime);
    console->Update(frametime);
    frame->Update(frametime);

    if (renderer)
        renderer->Render();
}

void Framework::Go()
{
    srand(time(0));

    plugin->LoadPluginsFromXML(plugin->ConfigurationFile());

    for(size_t i = 0; i < modules.size(); ++i)
    {
        LogDebug("Preinitializing module " + modules[i]->Name());
        modules[i]->PreInitialize();
    }

    for(size_t i = 0; i < modules.size(); ++i)
    {
        LogDebug("Initializing module " + modules[i]->Name());
        modules[i]->Initialize();
    }

    for(size_t i = 0; i < modules.size(); ++i)
    {
        LogDebug("PostInitializing module " + modules[i]->Name());
        modules[i]->PostInitialize();
    }

    // Run our QApplication subclass.
    application->Go();

    // Qt main loop execution has ended, we are existing.
    exit_signal_ = true;

    // Reset SceneAPI.
    scene->Reset();

    for(size_t i = 0; i < modules.size(); ++i)
    {
        LogDebug("Uninitializing module " + modules[i]->Name());
        modules[i]->Uninitialize();
    }

    for(size_t i = 0; i < modules.size(); ++i)
    {
        LogDebug("Unloading module " + modules[i]->Name());
        modules[i]->Unload();
    }

    /// Let go of all the module shared pointers, which causes a deletion of each module.
    modules.clear();
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

Application *Framework::GetApplication() const
{
    return application;
}

static std::string FormatTime(double time)
{
    char str[128];
    if (time >= 60.0)
    {
        double seconds = fmod(time, 60.0);
        int minutes = (int)(time / 60.0);
        sprintf(str, "%dmin %2.2fs", minutes, (float)seconds);
    }
    else if (time >= 1.0)
        sprintf(str, "%2.2fs", (float)time);
    else
        sprintf(str, "%2.2fms", (float)time*1000.f);

    return std::string(str);
}

#ifdef PROFILING
Profiler *Framework::GetProfiler()
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

IRenderer *Framework::GetRenderer() const
{
    return renderer;
}

void Framework::RegisterRenderer(IRenderer *renderer_)
{
    renderer = renderer_;
}

void Framework::RegisterModule(IModule *module)
{
    module->SetFramework(this);
    modules.push_back(boost::shared_ptr<IModule>(module));
    LogInfo("Registered a new module " + module->Name());

    module->Load();
}

IModule *Framework::GetModuleByName(const QString &name)
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
