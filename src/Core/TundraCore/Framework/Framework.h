// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "FrameworkFwd.h"
#include "CoreTypes.h"

#include <QObject>
#include <QStringList>

#ifdef ANDROID
#include <jni.h>
#endif

/// The system root access object.
class TUNDRACORE_API Framework : public QObject
{
    Q_OBJECT

public:
    /// Constructs and initializes the framework.
    /** @param argc Command line argument count as provided by the operating system.
        @param argv Command line arguments as provided by the operating system. */
    Framework(int argc, char** argv);

    ~Framework();

    /// Entry point for the framework.
    void Go();

    /// Runs through a single frame of logic update and rendering.
    void ProcessOneFrame();

    /// Returns module by class T.
    /** @param T class type of the module.
        @return The module, or null if the module doesn't exist. Always remember to check for null pointer.
        @note Do not store the returned raw module pointer anywhere or make a weak_ptr/shared_ptr out of it. */
    template <class T>
    T *Module() const;

    /// Registers a new module into the Framework.
    /** Framework will take ownership of the module pointer, so it is safe to pass in a raw pointer. */
    void RegisterModule(IModule *module);

    /// Stores the given QObject as a dynamic property into the Framework.
    /** This is done to implement easier script access for QObject-based interface objects.
        @param name The name to use for the property. Fails if name is an empty string.
        @param object The object to register as a property. The property will be a QVariant containing this QObject.
        @return If the registration succeeds, this returns true. Otherwise either 'object' pointer was null,
               or a property with that name was registered already.
        @note There is no unregister option. It can be implemented if someone finds it useful, but at this point
         we are going with a "unload-only-on-close" behavior. */
    bool RegisterDynamicObject(QString name, QObject *object);

    /// Cancel a pending exit
    void CancelExit();

    /// Force immediate exit, with no possibility to cancel it
    void ForceExit();

#ifdef PROFILING
    /// Returns the default profiler used by all normal profiling blocks. For profiling code, use PROFILE-macro.
    Profiler *GetProfiler() const;
#endif

    /// Returns the main QApplication
    Application *App() const;

    /// @cond PRIVATE
    /// Registers the system Renderer object.
    /** @note Please don't use this function. Called only by the OgreRenderingModule which implements the rendering subsystem. */
    void RegisterRenderer(IRenderer *renderer);
    /// @endcond

    /// Returns the system Renderer object.
    /** @note Please don't use this function. It exists for dependency inversion purposes only.
        Instead, call framework->Module<OgreRenderingModule>()->Renderer(); to directly obtain the renderer,
        as that will make the dependency explicit. The IRenderer interface is not continuously updated to match the real Renderer implementation. */
    IRenderer *Renderer() const;

    /// Stores the Framework instance. Call this inside each plugin DLL main function that will have a copy of the static instance pointer.
    static void SetInstance(Framework *fw) { instance = fw; }

    /// @cond PRIVATE
    /// Returns the global Framework instance.
    /** @note DO NOT CALL THIS FUNCTION. Every point where this function is called will cause a serious portability issue when we intend
        to run multiple instances inside a single process (inside a browser memory space). This function is intended to serve only for 
        carefully crafted re-entrant code (currently only logging and profiling). */
    static Framework *Instance() { return instance; }
    /// @endcond

    /// Returns the static plugin registry for systems where plugins are not loaded as dynamic libraries. Used by StaticPluginRegistry.
    static StaticPluginRegistry *StaticPluginRegistryInstance();

#ifdef ANDROID
    /// Returns the Java virtual machine instance on Android.
    static JavaVM* JavaVMInstance() { return javaVM; }
    /// Returns the Java environment instance on Android.
    static JNIEnv* JniEnvInstance() { return jniEnv; }
    /// Sets the Java virtual machine instance on Android. Called by JNIOnLoad in main.cpp.
    static void SetJavaVMInstance(JavaVM* instance) { javaVM = instance; }
    /// Sets the Java environment instance on Android. Called by main in main.cpp.
    static void SetJniEnvInstance(JNIEnv* instance) { jniEnv = instance; }
#endif
    // DEPRECATED
    template <class T> T *GetModule() const { return Module<T>(); } /**< @deprecated Use Module instead. @todo Remove. */

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

    /// Returns core API Plugin object.
    PluginAPI *Plugins() const;

    /// Returns raw module pointer.
    /** @param name Name of the module.
        @note Do not store the returned raw module pointer anywhere or make a weak_ptr/shared_ptr out of it. */
    IModule *ModuleByName(const QString &name) const;

    /// Returns if we're running the application in headless or not.
    bool IsHeadless() const { return headless; }

    /// Signals the framework to exit
    void Exit();

    /// Returns true if framework is in the process of exiting (will exit at next possible opportunity)
    bool IsExiting() const { return exitSignal; }

    /// Returns whether or not the command line arguments contain a specific value.
    /** @param value Key or value with possible prefixes, case-insensitive. */
    bool HasCommandLineParameter(const QString &value) const;

    /// Returns list of command line parameter values for a specific @c key, f.ex. "--file".
    /** Value is considered to be the command line argument following the @c key.
        If the argument following @c key is another key-type argument (--something), it's not appended to the return list.
        @param key Key with possible prefixes, case-insensitive */
    QStringList CommandLineParameters(const QString &key) const;

    /// Returns list of all the config XML filenames specified on command line or within another config XML
    QStringList ConfigFiles() const { return configFiles; }

    /// Prints to console all the used startup options.
    void PrintStartupOptions();

    /// Prints to console all the registered dynamic objects.
    void PrintDynamicObjects();

    // DEPRECATED
    IModule *GetModuleByName(const QString &name) const { return ModuleByName(name); } /**< @deprecated Use ModuleByName instead. @todo Add deprecation warning print. @todo Remove. */

private:
    /// Appends all found startup options from the given file to the startupOptions member.
    void LoadStartupOptionsFromXML(QString configurationFile);

    /// Appends startup options from a commandline file, Android only
#ifdef ANDROID
    void LoadCommandLineFromFile();
#endif

    bool exitSignal; ///< If true, exit application.
#ifdef PROFILING
    Profiler *profiler;
#endif
    ProfilerQObj *profilerQObj; ///< We keep this QObject always alive, even when profiling is not enabled, so that scripts don't have to check whether profiling is enabled or disabled.
    bool headless; ///< Are we running in the headless mode.
    Application *application; ///< The main QApplication object.
    FrameAPI *frame;
    ConsoleAPI *console;
    UiAPI *ui;
    InputAPI *input;
    AssetAPI *asset;
    AudioAPI *audio;
    SceneAPI *scene;
    ConfigAPI *config;
    PluginAPI *plugin;
    IRenderer *renderer;

    /// Stores all command line parameters and expanded options specified in the Config XML files, except for the config file(s) themselves.
    QStringList startupOptions;

    /// Stores config XML filenames
    QStringList configFiles;

    /// Framework owns the memory of all the modules in the system. These are freed when Framework is exiting.
    std::vector<shared_ptr<IModule> > modules;

    static Framework *instance;
    int argc; ///< Command line argument count as supplied by the operating system.
    char **argv; ///< Command line arguments as supplied by the operating system.

#ifdef ANDROID
    static JavaVM* javaVM;
    static JNIEnv* jniEnv;
#endif 
};

template <class T>
T *Framework::Module() const
{
    for(size_t i = 0; i < modules.size(); ++i)
    {
        T *module = dynamic_cast<T*>(modules[i].get());
        if (module)
            return module;
    }

    return 0;
}
