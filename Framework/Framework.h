// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include <QObject>
#include <QString>

#include <boost/smart_ptr.hpp>
#include <boost/program_options.hpp>

#include "FrameworkFwd.h"
//class ConnectionAPI;
//class ServerAPI;

/// The system root access object.
class Framework : public QObject
{
    Q_OBJECT

public:
    /// Constructs and initializes the framework.
    /** @param argc Command line argument count as provided by the operating system.
        @param argv Command line arguments as provided by the operating system.
    */
    Framework(int argc, char** argv);

    ~Framework();

    /// Parse program options from command line arguments
    /** For internal use. Should be called immediately after creating the framework,
        so all options will be taken in effect properly.
    */
    void ParseProgramOptions();

    /// Returns the command line options specified as command-line options when starting up.
    boost::program_options::variables_map &ProgramOptions() { return commandLineVariables; }

    /// Performs the module post-initialization steps.
    void PostInitialize();

    /// Entry point for the framework.
    void Go();

    /// Runs through a single frame of logic update and rendering.
    void ProcessOneFrame();

    /// Cancel a pending exit
    void CancelExit();

    /// Force immediate exit, with no possibility to cancel it
    void ForceExit();

    /// Returns true if framework is in the process of exiting (will exit at next possible opportunity)
    bool IsExiting() const { return exit_signal_; }

#ifdef PROFILING
    /// Returns the default profiler used by all normal profiling blocks. For profiling code, use PROFILE-macro.
    Profiler *GetProfiler();
#endif
    /*
    /// List all loaded modules
    ConsoleCommandResult ConsoleListModules(const StringVector &params);

    /// Output profiling data
    ConsoleCommandResult ConsoleProfile(const StringVector &params);

    /// limit frames
    ConsoleCommandResult ConsoleLimitFrames(const StringVector &params);
*/
    /// Loads all available modules. Do not call normally.
    void LoadModules();

    /// Unloads all available modules. Do not call normally.
    void UnloadModules();

    /// Get main QApplication
    Application *GetApplication() const;

    /// Returns module by class T.
    /** @param T class type of the module.
        @return The module, or null if the module doesn't exist. Always remember to check for null pointer.
        @note Do not store the returned raw module pointer anywhere or make a boost::weak_ptr/shared_ptr out of it. */
    template <class T>
    T *GetModule();

public slots:
    /// Returns the core API UI object.
    /** @note Never returns a null pointer. Use IsHeadless() to check out if we're running the headless mode or not. */
    UiAPI *Ui() const;

    /// Returns the core API Input object.
    InputAPI *Input() const;

    /// Returns the core API Frame object.
    FrameAPI *Frame() const;

    /// Returns the core API Console object.
    ConsoleAPI *Console() const;

    /// Returns the core API Audio object.
    AudioAPI *Audio() const;

    /// Returns core API Asset object.
    AssetAPI *Asset() const;

    /// Returns core API Scene object.
    SceneAPI *Scene() const;

    /// Returns core API Config object.
    ConfigAPI *Config() const;

    /// Returns core API Connection object.
//    ConnectionAPI *Connection() const;

    /// Returns core API Server object if we're acting as a network server.
//    ServerAPI *Server() const;

    PluginAPI *Plugins() const;

    /// Returns the system Renderer object.
    /// @note Please don't use this function. It exists for dependency inversion purposes only.
    /// Instead, call framework->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer(); to directly obtain the renderer,
    /// as that will make the dependency explicit. The IRenderer interface is not continuously updated to match the real Renderer implementation.
    IRenderer *GetRenderer() const;

    /// Returns the global Framework instance.
    /// @note DO NOT CALL THIS FUNCTION. Every point where this function is called
    ///       will cause a serious portability issue when we intend to run multiple instances inside a single process (inside a browser memory space).
    ///       This function is intended to serve only for carefully crafted re-entrant code (currently only logging and profiling).
    static Framework *GetInstance() { return instance; }

    /// Stores the Framework instance. Call this inside each plugin DLL main function that will have a copy of the static instance pointer.
    static void SetInstance(Framework *fw) { instance = fw; }

    /// Registers a new module into the Framework. Framework will take ownership of the module pointer, so it is safe to pass in a raw pointer.
    void RegisterModule(IModule *module);

    void RegisterRenderer(IRenderer *renderer);

    IModule *GetModuleByName(const QString &name);

    /// Returns if we're running the application in headless or not.
    bool IsHeadless() const { return headless_; }

    /// Stores the given QObject as a dynamic property into the Framework.
    /** This is done to implement easier script access for QObject-based interface objects.
        @param name The name to use for the property. Fails if name is an empty string.
        @param object The object to register as a property. The property will be a QVariant containing this QObject.
        @return If the registration succeeds, this returns true. Otherwise either 'object' pointer was null,
               or a property with that name was registered already.
        @note There is no unregister option. It can be implemented if someone finds it useful, but at this point
         we are going with a "unload-only-on-close" behavior.
    */
    bool RegisterDynamicObject(QString name, QObject *object);

    /// Signals the framework to exit
    void Exit();

private:
    Q_DISABLE_COPY(Framework)

    void UpdateModules();
    
    bool exit_signal_; ///< If true, exit application.
#ifdef PROFILING
    Profiler *profiler; ///< Profiler.
#endif
    boost::program_options::variables_map commandLineVariables; ///< program options
    boost::program_options::options_description commandLineDescriptions; ///< program option descriptions
    bool headless_; ///< Are we running in the headless mode.
    
    Application *application; ///< The main QApplication object.
    FrameAPI *frame; ///< The Frame API.
    ConsoleAPI *console; ///< The console API.
    UiAPI *ui; ///< The UI API.
    InputAPI *input; ///< The Input API.
    AssetAPI *asset; ///< The Asset API.
    AudioAPI *audio; ///< The Audio API.
    SceneAPI *scene; ///< The Scene API.
    ConfigAPI *config; ///< The Config API.
    PluginAPI *plugin;
    IRenderer *renderer;
//    ConnectionAPI *connection; ///< The Connection API.
//    ServerAPI *server; ///< The Server API, null if we're not operating as a server.

    /// Framework owns the memory of all the modules in the system. These are freed when Framework is exiting.
    std::vector<boost::shared_ptr<IModule> > modules;

    static Framework *instance;
    int argc_; ///< Command line argument count as supplied by the operating system.
    char **argv_; ///< Command line arguments as supplied by the operating system.
};

template <class T>
T *Framework::GetModule()
{ 
    for(size_t i = 0; i < modules.size(); ++i)
    {
        T *module = dynamic_cast<T*>(modules[i].get());
        if (module)
            return module;
    }

    return 0;
}

