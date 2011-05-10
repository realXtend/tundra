// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Framework.h"
#include "ModuleManager.h"
#include "ServiceManager.h"
#include "RenderServiceInterface.h"
#include "CoreException.h"

#include "Application.h"

#include "InputAPI.h"
#include "FrameAPI.h"
#include "AssetAPI.h"
#include "AudioAPI.h"
#include "ConsoleAPI.h"
#include "DebugAPI.h"
#include "SceneAPI.h"
#include "ConfigAPI.h"
#include "PluginAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"

#include "MemoryLeakCheck.h"

Framework::Framework(int argc, char** argv) :
    exit_signal_(false),
    argc_(argc),
    argv_(argv),
    initialized_(false),
    headless_(false),
    application(0),
    frame(new FrameAPI(this)),
    console(0),
    debug(new DebugAPI(this)),
    scene(new SceneAPI(this)),
    input(0),
    asset(0),
    audio(0),
    plugin(0),
    ui(0),
    connection(0),
    server(0)
{
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
        ProfilerSection::SetProfiler(&profiler_);
#endif
        PROFILE(FW_Startup);

        // Create QApplication
        application = new Application(this, argc_, argv_);
        initialized_ = true;

        // Create ConfigAPI and pass application data.
        config = new ConfigAPI(this);
        config->SetApplication(applicationOrganization, applicationName, applicationVersion);

        // Prepare ConfigAPIs working directory
        config->PrepareDataFolder("configuration");

        // Create managers
        module_manager_ = ModuleManagerPtr(new ModuleManager(this));
        service_manager_ = ServiceManagerPtr(new ServiceManager());

        // Create AssetAPI.
        asset = new AssetAPI(headless_);
        asset->OpenAssetCache(Application::UserDataDirectory() + "assetcache");

        // Create AssetAPI.
        ui = new UiAPI(this);                

        // Create AudioAPI, depends on the AssetAPI, so must be loaded after it.
        audio = new AudioAPI(this, asset);

        // Create Input and PluginAPI.
        input = new InputAPI(this);
        plugin = new PluginAPI(this);
        console = new ConsoleAPI(this);

        // Initialize SceneAPI.
        scene->Initialise();

        RegisterDynamicObject("ui", ui);
        RegisterDynamicObject("frame", frame);
        RegisterDynamicObject("input", input);
        RegisterDynamicObject("console", console);
        RegisterDynamicObject("asset", asset);
        RegisterDynamicObject("audio", audio);
        RegisterDynamicObject("debug", debug);
        RegisterDynamicObject("application", application);
    }
}

Framework::~Framework()
{
    service_manager_.reset();
    module_manager_.reset();

    // Delete the QObjects that don't have a parent.
    delete input;
    delete asset;
    delete audio;
    delete plugin;

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

void Framework::PostInitialize()
{
    PROFILE(FW_PostInitialize);

    srand(time(0));

    plugin->LoadPluginsFromXML(plugin->ConfigurationFile());

    LoadModules();

    // PostInitialize SceneAPI.
    scene->PostInitialize();

    // commands must be registered after modules are loaded and initialized
    RegisterConsoleCommands();
}

void Framework::ProcessOneFrame()
{
    static tick_t clock_freq;
    static tick_t last_clocktime;

    if (!last_clocktime)
        last_clocktime = GetCurrentClockTime();

    if (!clock_freq)
        clock_freq = GetCurrentClockFreq();

    if (exit_signal_ == true)
        return; // We've accidentally ended up to update a frame, but we're actually quitting.

    {
        PROFILE(FW_MainLoop);

        tick_t curr_clocktime = GetCurrentClockTime();
        double frametime = ((double)curr_clocktime - (double)last_clocktime) / (double) clock_freq;
        last_clocktime = curr_clocktime;

        {
            PROFILE(FW_UpdateModules);
            module_manager_->UpdateModules(frametime);
        }

        // Process the asset API updates.
        {
            PROFILE(Asset_Update);
            asset->Update(frametime);
        }

        // Process all keyboard input.
        {
            PROFILE(Input_Update);
            input->Update(frametime);
        }

        // Process all audio playback.
        {
            PROFILE(Audio_Update);
            audio->Update(frametime);
        }

        // Process frame update now. Scripts handling the frame tick will be run at this point, and will have up-to-date 
        // information after for example network updates, that have been performed by the modules.
        {
            PROFILE(Frame_Update);
            frame->Update(frametime);
        }

        console->Update(frametime);

        if (!IsHeadless()) // Skip render if in headless mode.
        {
            // if we have a renderer service, render now
            boost::weak_ptr<RenderServiceInterface> renderer = service_manager_->GetService<RenderServiceInterface>();
            if (renderer.expired() == false)
            {
                PROFILE(FW_Render);
                renderer.lock()->Render();
            }
        }
    }

    RESETPROFILER
}

void Framework::Go()
{
    {
        PROFILE(FW_PostInitialize);
        PostInitialize();
    }
    
    // Run our QApplication subclass.
    application->Go();

    // Qt main loop execution has ended, we are existing.
    exit_signal_ = true;

    // Reset SceneAPI.
    scene->Reset();

    // Unload modules
    UnloadModules();
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

void Framework::LoadModules()
{
    module_manager_->InitializeModules();
}

void Framework::UnloadModules()
{
    module_manager_->UninitializeModules();
    ///\todo Horrible uninit call here now due to console refactoring
    console->Uninitialize();
    module_manager_->UnloadModules();
}

Application *Framework::GetApplication() const
{
    return application;
}
/*
ConsoleCommandResult Framework::ConsoleListModules(const StringVector &params)
{
    if (console)
    {
        console->Print("Loaded modules:");
        const ModuleManager::ModuleVector &modules = module_manager_->GetModuleList();
        for(size_t i = 0 ; i < modules.size() ; ++i)
            console->Print(modules[i].module_->Name().c_str());
    }

    return ConsoleResultSuccess();
}
*/
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

/// Outputs a hierarchical list of all PROFILE() blocks onto the given console.
/// @param node The root node where to start the printing.
/// @param showUnused If true, even blocks that haven't been called will be included. If false, only
///        the blocks that were actually recently called are included. 
void PrintTimingsToConsole(ConsoleAPI *console, ProfilerNodeTree *node, bool showUnused)
{
    const ProfilerNode *timings_node = dynamic_cast<const ProfilerNode*>(node);

    // Controls whether we will recursively call self to also print all child nodes.
    bool recurseToChildren = true;

    static int level = -2;

    if (timings_node)
    {
        level += 2;
        assert (level >= 0);

        double average = timings_node->num_called_total_ == 0 ? 0.0 : timings_node->total_ / timings_node->num_called_total_;

        if (timings_node->num_called_ == 0 && !showUnused)
            recurseToChildren = false;
        else
        {
            char str[512];
            
            sprintf(str, "%s: called total: %lu, elapsed total: %s, called: %lu, elapsed: %s, avg: %s",
                timings_node->Name().c_str(), timings_node->num_called_total_,
                FormatTime(timings_node->total_).c_str(), timings_node->num_called_,
                FormatTime(timings_node->elapsed_).c_str(), FormatTime(average).c_str());

/*
            timings += timings_node->Name();
            timings += ": called total " + ToString(timings_node->num_called_total_);
            timings += ", elapsed total " + ToString(timings_node->total_);
            timings += ", called " + ToString(timings_node->num_called_);
            timings += ", elapsed " + ToString(timings_node->elapsed_);
            timings += ", average " + ToString(average);
*/

            std::string timings;
            for(int i = 0; i < level; ++i)
                timings.append("&nbsp;");
            timings += str;

            console->Print(timings.c_str());
        }
    }

    if (recurseToChildren)
    {
        const ProfilerNodeTree::NodeList &children = node->GetChildren();
        for(ProfilerNodeTree::NodeList::const_iterator it = children.begin(); it != children.end() ; ++it)
            PrintTimingsToConsole(console, (*it).get(), showUnused);
    }

    if (timings_node)
        level -= 2;
}
/*
ConsoleCommandResult Framework::ConsoleProfile(const StringVector &params)
{
#ifdef PROFILING
    if (console)
    {
        Profiler &profiler = GetProfiler();
//            ProfilerNodeTree *node = profiler.Lock().get();
        ProfilerNodeTree *node = profiler.GetRoot();
        if (params.size() > 0 && params.front() == "all")
            PrintTimingsToConsole(console, node, true);
        else
            PrintTimingsToConsole(console, node, false);
        console->Print(" ");
//            profiler.Release();
    }
#endif
    return ConsoleResultSuccess();
}
*/
void Framework::RegisterConsoleCommands()
{
/*
    console->RegisterCommand(CreateConsoleCommand("ListModules",
        "Lists all loaded modules.", 
        ConsoleBind(this, &Framework::ConsoleListModules)));

#ifdef PROFILING
    console->RegisterCommand(CreateConsoleCommand("Profile", 
        "Outputs profiling data. Usage: Profile() for full, or Profile(name) for specific profiling block",
        ConsoleBind(this, &Framework::ConsoleProfile)));
#endif*/
}

#ifdef PROFILING
Profiler &Framework::GetProfiler()
{
    return profiler_;
}
#endif

ModuleManagerPtr Framework::GetModuleManager() const
{
    return module_manager_;
}

ServiceManagerPtr Framework::GetServiceManager() const
{
    return service_manager_;
}

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

DebugAPI *Framework::Debug() const
{
    return debug;
}

SceneAPI *Framework::Scene() const
{
    return scene;
}

ConfigAPI *Framework::Config() const
{
    return config;
}

ConnectionAPI *Framework::Connection() const
{
    return connection;
}

ServerAPI *Framework::Server() const
{
    return server;
}

PluginAPI *Framework::Plugins() const
{
    return plugin;
}

QObject *Framework::GetModuleQObj(const QString &name)
{
    ModuleWeakPtr module = GetModuleManager()->GetModule(name.toStdString());
    return dynamic_cast<QObject*>(module.lock().get());
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
