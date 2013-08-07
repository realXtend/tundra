// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Renderer.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"
#include "EC_Placeable.h"
#include "EC_Camera.h"
#include "RenderWindow.h"
#include "OgreShadowCameraSetupFocusedPSSM.h"
#include "OgreCompositionHandler.h"
#include "UiPlane.h"
#include "TextureAsset.h"
#include "OgreMeshAsset.h"
#include "OgreMaterialAsset.h"
#include "OgreSkeletonAsset.h"
#include "OgreParticleAsset.h"

#include "Application.h"
#include "UiGraphicsView.h"
#include "Scene/Scene.h"
#include "CoreException.h"
#include "Entity.h"
#include "SceneAPI.h"
#include "Profiler.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "UiGraphicsView.h"
#include "LoggingFunctions.h"
#include "ConfigAPI.h"
#include "QScriptEngineHelpers.h"

#include <Ogre.h>
#include <OgreDefaultHardwareBufferManager.h>
#include <OgreOverlaySystem.h>

#ifdef ANDROID
#define OGRE_STATIC_GLES2
#define OGRE_STATIC_ParticleFX
#define OGRE_STATIC_OctreeSceneManager
#include "OgreStaticPluginLoader.h"
#include <OgreRTShaderSystem.h>
#include <OgreShaderGenerator.h>
#endif

Q_DECLARE_METATYPE(EC_Placeable*)
Q_DECLARE_METATYPE(EC_Camera*)
Q_DECLARE_METATYPE(UiPlane*)
// Ogre asset types and ptr typedefs
Q_DECLARE_METATYPE(OgreMeshAsset*)
Q_DECLARE_METATYPE(TextureAsset*)
Q_DECLARE_METATYPE(OgreMaterialAsset*)
Q_DECLARE_METATYPE(OgreSkeletonAsset*)
Q_DECLARE_METATYPE(OgreParticleAsset*)
Q_DECLARE_METATYPE(TextureAssetPtr)
Q_DECLARE_METATYPE(OgreMeshAssetPtr)
Q_DECLARE_METATYPE(OgreMaterialAssetPtr)
Q_DECLARE_METATYPE(OgreSkeletonAssetPtr)
Q_DECLARE_METATYPE(OgreParticleAssetPtr)

// Clamp elapsed frame time to avoid Ogre controllers going crazy
static const float MAX_FRAME_TIME = 0.1f;

// Default texture budget (megabytes)
static const int DEFAULT_TEXTURE_BUDGET = 512;
// Minimum texture budget
static const int MINIMUM_TEXTURE_BUDGET = 1;

#if defined(DIRECTX_ENABLED) && !defined(WIN32)
#undef DIRECTX_ENABLED
#endif

#ifdef DIRECTX_ENABLED
#undef SAFE_DELETE
#undef SAFE_DELETE_ARRAY

#include <d3d9.h>
#include <OgreD3D9HardwarePixelBuffer.h>
#include <OgreD3D9RenderWindow.h>
#endif

#include <QCloseEvent>
#include <QSize>
#include <QDir>

#ifdef PROFILING
#include "InputAPI.h"
#endif

#include "MemoryLeakCheck.h"

#ifdef ANDROID
/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2012 Torus Knot Software Ltd
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
 */
class ShaderGeneratorTechniqueResolverListener : public Ogre::MaterialManager::Listener
{
public:
    ShaderGeneratorTechniqueResolverListener(Ogre::RTShader::ShaderGenerator* pShaderGenerator)
    {
        mShaderGenerator = pShaderGenerator;
    }

    /** This is the hook point where shader based technique will be created.
    It will be called whenever the material manager won't find appropriate technique
    that satisfy the target scheme name. If the scheme name is out target RT Shader System
    scheme name we will try to create shader generated technique for it. 
    */
    virtual Ogre::Technique* handleSchemeNotFound(unsigned short schemeIndex, 
        const Ogre::String& schemeName, Ogre::Material* originalMaterial, unsigned short lodIndex, 
        const Ogre::Renderable* rend)
    {    
        Ogre::Technique* generatedTech = NULL;

        // Case this is the default shader generator scheme.
        if (schemeName == Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME)
        {
            bool techniqueCreated;

            // Create shader generated technique for this material.
            techniqueCreated = mShaderGenerator->createShaderBasedTechnique(
                originalMaterial->getName(),
                Ogre::MaterialManager::DEFAULT_SCHEME_NAME,
                schemeName);

            // Case technique registration succeeded.
            if (techniqueCreated)
            {
                // Force creating the shaders for the generated technique.
                mShaderGenerator->validateMaterial(schemeName, originalMaterial->getName());
                
                // Grab the generated technique.
                Ogre::Material::TechniqueIterator itTech = originalMaterial->getTechniqueIterator();

                while (itTech.hasMoreElements())
                {
                    Ogre::Technique* curTech = itTech.getNext();

                    if (curTech->getSchemeName() == schemeName)
                    {
                        generatedTech = curTech;
                        break;
                    }
                }
            }
        }

        return generatedTech;
    }

protected:
    Ogre::RTShader::ShaderGenerator* mShaderGenerator; // The shader generator instance.
};
#endif

namespace OgreRenderer
{
    /// A DIRTY HACK to enable the Ogre logger to actually report to the user about which material parsing failed.
    QString lastLoadedOgreMaterial = "";
    
    /// @cond PRIVATE
    class OgreLogListener : public Ogre::LogListener
    {
        bool hideBenignOgreMessages;
    public:
        explicit OgreLogListener(bool hideBenignOgreMessages_)
        :hideBenignOgreMessages(hideBenignOgreMessages_)
        {
        }

#if OGRE_VERSION_MAJOR <= 1 && OGRE_VERSION_MINOR < 8
        void messageLogged(const Ogre::String &message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String &logName)
#else
        void messageLogged(const Ogre::String &message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String &logName, bool &)
#endif
        {
            QString str = message.c_str();

            // We can ignore some Ogre messages which don't give any extra information, if requested.
            if (hideBenignOgreMessages)
            {
                if (str.contains("disabling VSync in windowed"))  // "D3D9 : WARNING - disabling VSync in windowed mode can cause timing issues at lower frame rates, turn VSync on if you observe this problem."
                    return;
                if (str.contains("Cannot locate resource") && str.contains(".skeleton in resource group")) // "Error: OGRE EXCEPTION(6:FileNotFoundException): Cannot locate resource filename.skeleton in resource group General or any other group. in ResourceGroupManager::openResource at .\src\OgreResourceGroupManager.cpp (line 753)"
                    return; // We download assets through the network, and by the time the mesh is loaded, its skeleton will often not be, but Ogre will not like this.
                if (str.contains("is an older format ([MeshSerializer_")) // "Warning: WARNING: filename.mesh is an older format ([MeshSerializer_v1.40]); you should upgrade it as soon as possible using the OgreMeshUpgrade tool."
                    return; // Loading an old mesh version. Ogre can still load them up fine, and the end user should not be conserned of the version number.
                if (str.contains("more than 4 bone assignments.")) // "Warning: WARNING: the mesh 'EC_Mesh_clone_169' includes vertices with more than 4 bone assignments. The lowest weighted assignments beyond this limit have been removed, so your animation may look slightly different. To eliminate this, reduce the number of bone assignments per vertex on your mesh to 4."
                    return; // The end user cannot control this.
                if (str.contains("Cannot locate an appropriate 2D texture coordinate set")) // "Cannot locate an appropriate 2D texture coordinate set for all the vertex data in this mesh to create tangents from."
                    return; // This is benign, meshes without normals do not need to get tangents either.
            }

            if (str.contains("Compiler error") && !lastLoadedOgreMaterial.isEmpty())
                str = "When compiling material " + lastLoadedOgreMaterial + ": " + str;

            if (lml == Ogre::LML_CRITICAL)
            {
                // Some Ogre Critical messages are actually not errors. For example MaterialSerializer's log messages.
                LogError(str);
            }
            else if (lml == Ogre::LML_TRIVIAL)
                LogDebug(str);
            else // lml == Ogre::LML_NORMAL here.
            {
                // Because Ogre distinguishes different log levels *VERY POORLY* (practically all messages come in the LML_NORMAL), 
                // we need to use manual format checks to guess between Info/Warning/Error/Critical channels.
                if ((str.contains("error ", Qt::CaseInsensitive) || str.contains("error:", Qt::CaseInsensitive)) || str.contains("critical", Qt::CaseInsensitive))
                    LogError(str);
                else if (str.contains("warning", Qt::CaseInsensitive) || str.contains("unexpected", Qt::CaseInsensitive) || str.contains("unknown", Qt::CaseInsensitive) || str.contains("cannot", Qt::CaseInsensitive) || str.contains("can not", Qt::CaseInsensitive))
                    LogWarning(str);
                else if (str.startsWith("*-*-*"))
                    LogInfo(str);
                else
                    LogDebug(str);
            }
        }
    };
    /// @endcond

    Renderer::Renderer(Framework* fw, const std::string& config, const std::string& plugins, const std::string& window_title) :
        initialized(false),
        framework(fw),
//        bufferManager(0), ///< @todo Unused - delete for good?
        defaultScene(0),
        dummyDefaultCamera(0),
        mainViewport(0),
#ifdef ANDROID
        shaderGenerator(0),
#endif
        overlaySystem(0),
        uniqueObjectId(0),
        uniqueGroupId(0),
        configFilename(config),
        pluginsFilename(plugins),
        windowTitle(window_title),
        renderWindow(0),
        lastWidth(0),
        lastHeight(0),
        resizedDirty(0),
        viewDistance(500.0f),
        shadowQuality(Shadows_High),
        textureQuality(Texture_Normal),
        textureBudget(DEFAULT_TEXTURE_BUDGET)
    {
        compositionHandler = new OgreCompositionHandler();
        logListener = new OgreLogListener(fw->HasCommandLineParameter("--hideBenignOgreMessages") ||
            fw->HasCommandLineParameter("--hide_benign_ogre_messages")); /**< @todo Remove support for the deprecated underscore version at some point. */

        timerFrequency = GetCurrentClockFreq();
        PrepareConfig();
    }

    Renderer::~Renderer()
    {
        if (framework->Ui() && framework->Ui()->MainWindow())
            framework->Ui()->MainWindow()->SaveWindowSettingsToFile();

        // Delete all UiPlanes that still exist.
        while(uiPlanes.size() > 0)
            DeleteUiPlane(uiPlanes.front());

        // Delete all worlds that still exist
        ogreWorlds.clear();
        
        // Delete the default camera & scene
        if (defaultScene)
        {
            defaultScene->destroyCamera(dummyDefaultCamera);
            dummyDefaultCamera = 0;
#ifdef ANDROID
            if (shaderGenerator)
                shaderGenerator->removeSceneManager(defaultScene);
#endif
            ogreRoot->destroySceneManager(defaultScene);
            defaultScene = 0;
        }

        SAFE_DELETE(overlaySystem);
#ifdef ANDROID
        Ogre::RTShader::ShaderGenerator::finalize();
#endif

        ogreRoot.reset();
        SAFE_DELETE(compositionHandler);
        SAFE_DELETE(logListener);
        SAFE_DELETE(renderWindow);
    }

    void Renderer::PrepareConfig()
    {
        ConfigData configData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING);
        // View distance, as double to keep human readable and configurable
        if (!framework->Config()->HasValue(configData, "view distance"))
            framework->Config()->Set(configData, "view distance", (double)viewDistance);
        // Shadow quality
        if (!framework->Config()->HasValue(configData, "shadow quality"))
            framework->Config()->Set(configData, "shadow quality", 2);
        // Texture quality
        if (!framework->Config()->HasValue(configData, "texture quality"))
            framework->Config()->Set(configData, "texture quality", 1);
        // Soft shadow
        if (!framework->Config()->HasValue(configData, "soft shadow"))
            framework->Config()->Set(configData, "soft shadow", false);
        // Rendering plugin
#ifdef _WINDOWS
        if (!framework->Config()->HasValue(configData, "rendering plugin"))
            framework->Config()->Set(configData, "rendering plugin", "Direct3D9 Rendering Subsystem");
#else
        if (!framework->Config()->HasValue(configData, "rendering plugin"))
            framework->Config()->Set(configData, "rendering plugin", "OpenGL Rendering Subsystem");
#endif
    }

    void Renderer::Initialize()
    {
        assert(!initialized);
        if (initialized)
        {
            LogError("Renderer::Initialize: Called when Renderer has already been initialized!");
            return;
        }

        std::string logfilepath, rendersystem_name;
        Ogre::RenderSystem *rendersystem = 0;

        LogInfo("Renderer: Initializing Ogre");

        // Create Ogre root with logfile
        QDir logDir(Application::UserDataDirectory());
        if (!logDir.exists("logs"))
            logDir.mkdir("logs");
        logDir.cd("logs");

        logfilepath = logDir.absoluteFilePath("Ogre.log").toStdString(); ///<\todo Unicode support
#include "DisableMemoryLeakCheck.h"
// On Android instantiating our own LogManager results in a crash during Ogre initialization. Ogre has its own Android logging hook, so this can be skipped for now
#ifndef ANDROID
        static Ogre::LogManager *overriddenLogManager = 0;
        overriddenLogManager = new Ogre::LogManager; ///\bug This pointer is never freed. We leak memory here, but cannot free due to Ogre singletons being accessed.
        overriddenLogManager->createLog("", true, false, true);
        Ogre::LogManager::getSingleton().getDefaultLog()->setDebugOutputEnabled(false); // Disable Ogre from outputting to std::cerr by itself.
        Ogre::LogManager::getSingleton().getDefaultLog()->addListener(logListener); // Make all Ogre log output to come to our log listener.
        Ogre::LogManager::getSingleton().getDefaultLog()->setLogDetail(Ogre::LL_NORMAL); // This is probably the default level anyway, but be explicit.
#endif
        ogreRoot = OgreRootPtr(new Ogre::Root("", configFilename, logfilepath));

#include "EnableMemoryLeakCheck.h"

        ConfigData configData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING);
        viewDistance = framework->Config()->Get(configData, "view distance").toFloat();

        // Load plugins
        QStringList loadedPlugins = LoadPlugins(pluginsFilename);

        // Read the default rendersystem from Config API.
        rendersystem_name = framework->Config()->Get(configData, "rendering plugin").toString().toStdString();

#ifdef _WINDOWS
        // If --direct3d9 is specified, it overrides the option that was set in config.
        if (framework->HasCommandLineParameter("--d3d9") || framework->HasCommandLineParameter("--direct3d9"))
            rendersystem_name = "Direct3D9 Rendering Subsystem";
#endif

        // If --opengl is specified, it overrides the option that was set in config.
        if (framework->HasCommandLineParameter("--opengl"))
            rendersystem_name = "OpenGL Rendering Subsystem";

        // --nullrenderer disables all Ogre rendering ops.
        if (framework->HasCommandLineParameter("--nullrenderer"))
            rendersystem_name = "NULL Rendering Subsystem";

        textureQuality = (Renderer::TextureQualitySetting)framework->Config()->Get(configData, "texture quality").toInt();
        textureBudget = framework->Config()->Get(configData, "texture budget").toInt();
        
        if (framework->HasCommandLineParameter("--texturebudget"))
        {
            QStringList sizeParam = framework->CommandLineParameters("--texturebudget");
            if (sizeParam.size() > 0)
                SetTextureBudget(sizeParam.first().toInt());
        }

        // Ask Ogre if rendering system is available
        rendersystem = ogreRoot->getRenderSystemByName(rendersystem_name);

#ifdef _WINDOWS
        // If windows did not have Direct3D fallback to OpenGL.
        if (!rendersystem)
            rendersystem = ogreRoot->getRenderSystemByName("OpenGL Rendering Subsystem");

        // If windows did not have OpenGL fallback to Direct3D.
        if (!rendersystem)
            rendersystem = ogreRoot->getRenderSystemByName("Direct3D9 Rendering Subsystem");
#endif

#ifdef ANDROID
        // Android: use the first available rendersystem, should be GLES2
        if (!rendersystem)
            rendersystem = ogreRoot->getAvailableRenderers().at(0);
#endif

        if (!rendersystem)
            throw Exception("Could not find Ogre rendersystem.");

        // Report rendering plugin to log so user can check what actually got loaded
        LogInfo("Renderer: Using " + rendersystem->getName());

        // Allow PSSM mode shadows only on DirectX
        // On OpenGL (arbvp & arbfp) it runs out of vertex shader outputs
        shadowQuality = (Renderer::ShadowQualitySetting)framework->Config()->Get(configData, "shadow quality").toInt();
        if ((shadowQuality == Shadows_High) && (rendersystem->getName() != "Direct3D9 Rendering Subsystem"))
            shadowQuality = Shadows_Low;

        // This is needed for QWebView to not lock up!!!
        Ogre::ConfigOptionMap& map = rendersystem->getConfigOptions();
        if (map.find("Floating-point mode") != map.end())
            rendersystem->setConfigOption("Floating-point mode", "Consistent");

        if (framework->IsHeadless())
        {
            // If we are running in headless mode, initialize the Ogre 'DefaultHardwareBufferManager', which is a software-emulation
            // mode for Ogre's HardwareBuffers (i.e. can create meshes into CPU memory).
#include "DisableMemoryLeakCheck.h"
            new Ogre::DefaultHardwareBufferManager(); // This creates a Ogre manager singleton, so can discard the return value from operator new.
#include "EnableMemoryLeakCheck.h"
        }
        else
        {
            // Set the found rendering system
            ogreRoot->setRenderSystem(rendersystem);

            // Instantiate overlay system
            overlaySystem = new Ogre::OverlaySystem();

            // Initialise but don't create rendering window yet
            ogreRoot->initialise(false);

            try
            {
                int width = framework->Ui()->GraphicsView()->viewport()->size().width();
                int height = framework->Ui()->GraphicsView()->viewport()->size().height();
                int window_left = 0;
                int window_top = 0;
                renderWindow = new RenderWindow();
                bool fullscreen = framework->HasCommandLineParameter("--fullscreen");
#ifdef Q_WS_MAC
                // Fullscreen causes crash on Mac OS X. See https://github.com/realXtend/naali/issues/522
                if (fullscreen)
                {
                    LogWarning("Fullscreen is not currently supported on Mac OS X. The application will run in windowed-mode");
                    fullscreen = false;
                }
#endif
                // On some systems, the Ogre rendering output is overdrawn by the Windows desktop compositing manager, but the actual cause of this
                // is uncertain.
                // As a workaround, it is possible to have Ogre output directly on the main window HWND of the ui chain. On other systems, this gives
                // graphical issues, so it cannot be used as a permanent mechanism. Therefore this workaround is enabled only as a command-line switch.
                if (framework->HasCommandLineParameter("--ogrecapturetopwindow"))
                {
                    // In this mode the toolbar will not be rendered anyway, but if we don't hide it you have to click all UI ~20-25 pixels 
                    // below the actual position to hit it. It's an error in our ui overlay to InputAPI mapping and gets fixed by hiding the toolbar.
                    if (fullscreen && framework->Ui()->MainWindow()->menuBar())
                        framework->Ui()->MainWindow()->menuBar()->hide();
                    renderWindow->CreateRenderWindow(framework->Ui()->MainWindow(), windowTitle.c_str(), width, height, window_left, window_top, fullscreen, framework);
                }
                else if (framework->HasCommandLineParameter("--nouicompositing"))
                    renderWindow->CreateRenderWindow(0, windowTitle.c_str(), width, height, window_left, window_top, fullscreen, framework);
                else // Normally, we want to render Ogre onto the UiGraphicsview viewport window.
                {
                    // Even if the user has requested fullscreen mode, init Ogre in windowed mode, since the main graphics view is not a top-level window, and Ogre cannot
                    // initialize into fullscreen with a non-top-level window handle. As D3D9 is initialized in windowed mode, this has the effect that vsync cannot be enabled.
                    // To set up vsync, specify the
                    if (framework->CommandLineParameters("--vsync").length() > 0 && ParseBool(framework->CommandLineParameters("--vsync").first()))
                        LogWarning("--vsync was specified, but Ogre is initialized in windowed mode to a non-top-level window. VSync will probably *not* be active. To enable vsync in full screen mode, "
                            "specify the flags --vsync, --fullscreen and --ogrecapturetopwindow together.");
                    renderWindow->CreateRenderWindow(framework->Ui()->GraphicsView()->viewport(), windowTitle.c_str(), width, height, window_left, window_top, false, framework);
                }

                connect(framework->Ui()->GraphicsView(), SIGNAL(WindowResized(int, int)), renderWindow, SLOT(Resize(int, int)));
                renderWindow->Resize(framework->Ui()->GraphicsView()->width(), framework->Ui()->GraphicsView()->height());

                if (fullscreen)
                    framework->Ui()->MainWindow()->showFullScreen();
                else
                    framework->Ui()->MainWindow()->show();
            }
            catch(Ogre::Exception &/*e*/)
            {
                LogError("Could not create ogre rendering window!");
                throw;
            }

            LogInfo("Renderer: Loading Ogre resources");
            SetupResources();
            CreateInstancingShaders();

#ifdef ANDROID
            // Android: initialize RT shader system
            Ogre::RTShader::ShaderGenerator::initialize();
            shaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
            Ogre::MaterialManager::getSingleton().addListener(new ShaderGeneratorTechniqueResolverListener(shaderGenerator));
#endif

            /// Create the default scene manager, which is used for nothing but rendering emptiness in case we have no framework scenes
            defaultScene = ogreRoot->createSceneManager(Ogre::ST_GENERIC, "DefaultEmptyScene");
            defaultScene->addRenderQueueListener(overlaySystem);
#ifdef ANDROID
            if (shaderGenerator)
                shaderGenerator->addSceneManager(defaultScene);
#endif
            dummyDefaultCamera = defaultScene->createCamera("DefaultCamera");
        
            mainViewport = renderWindow->OgreRenderWindow()->addViewport(dummyDefaultCamera);
            compositionHandler->SetViewport(mainViewport);
        }

        initialized = true;
    }

    void Renderer::SetFullScreen(bool value)
    {
        // In headless mode, we can safely ignore Fullscreen mode requests.
        if (framework->IsHeadless())
            return;

#ifndef Q_WS_MAC
        // Fullscreen causes crash on Mac OS X. See https://github.com/realXtend/naali/issues/522
        if (value)
            framework->Ui()->MainWindow()->showFullScreen();
        else
#endif
            framework->Ui()->MainWindow()->showNormal();
    }

    bool Renderer::IsFullScreen() const
    {
        if (!framework->IsHeadless())
            return framework->Ui()->MainWindow()->isFullScreen();
        else
            return false;
    }

    void Renderer::SetShadowQuality(ShadowQualitySetting quality)
    {
        // We cannot effect the new setting immediately, so save only to config
        framework->Config()->Set(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "shadow quality", (int)quality);
    }

    void Renderer::SetTextureQuality(TextureQualitySetting quality)
    {
        // We cannot effect the new setting immediately, so save only to config
        framework->Config()->Set(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "texture quality", (int)quality);
    }
    
    void Renderer::SetTextureBudget(int budget)
    {
        textureBudget = Max(budget, MINIMUM_TEXTURE_BUDGET);
        framework->Config()->Set(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "texture budget", textureBudget);
    }

    float Renderer::TextureBudgetUse(size_t loadDataSize) const
    {
        return (float)(Ogre::TextureManager::getSingletonPtr()->getMemoryUsage() + loadDataSize) / (1024.f*1024.f) / (float)textureBudget;
    }

    QStringList Renderer::LoadPlugins(const std::string& plugin_filename)
    {
        QStringList loadedPlugins;

#ifndef ANDROID
        Ogre::ConfigFile file;
        try
        {
            file.load(plugin_filename);
        }
        catch(Ogre::Exception &/*e*/)
        {
            LogError("Could not load Ogre plugins configuration file");
            return loadedPlugins;
        }

        Ogre::String plugin_dir = file.getSetting("PluginFolder");
        if (plugin_dir == ".")
            plugin_dir = QDir::toNativeSeparators(QFileInfo(plugin_filename.c_str()).dir().path()).toStdString(); ///<\todo Unicode support?
        Ogre::StringVector plugins = file.getMultiSetting("Plugin");

        if (plugin_dir.length() && plugin_dir[plugin_dir.length() - 1] != '\\' && plugin_dir[plugin_dir.length() - 1] != '/')
            plugin_dir += QDir::separator().toAscii();

        for(uint i = 0; i < plugins.size(); ++i)
        {
            try
            {
                ogreRoot->loadPlugin(plugin_dir + plugins[i]);
                loadedPlugins.append(QString::fromStdString(plugins[i]));
            }
            catch(Ogre::Exception &/*e*/)
            {
                LogError("Plugin " + plugins[i] + " failed to load");
            }
        }
#else
        // On Android, load Ogre plugins statically
        staticPluginLoader = new Ogre::StaticPluginLoader();
        staticPluginLoader->load();
#endif
        return loadedPlugins;
    }

    void Renderer::SetupResources()
    {
        Ogre::ConfigFile cf;
        cf.load("resources.cfg");

        Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
        Ogre::String sec_name, type_name, arch_name;

        while(seci.hasMoreElements())
        {
            sec_name = seci.peekNextKey();
            Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();
            Ogre::ConfigFile::SettingsMultiMap::iterator i;
            for(i = settings->begin(); i != settings->end(); ++i)
            {
                type_name = i->first;
                arch_name = i->second;
                if (QDir::isRelativePath(arch_name.c_str()))
                    arch_name = QDir::cleanPath(Application::InstallationDirectory() + arch_name.c_str()).toStdString(); ///<\todo Unicode support

                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch_name, type_name, sec_name);
            }
        }

        // Add supershader program definitions directory according to the shadow quality level
        std::string shadowPath = Application::InstallationDirectory().toStdString(); ///<\todo Unicode support
        switch(shadowQuality)
        {
        case Shadows_Off:
            shadowPath.append("media/materials/scripts/shadows_off");
            break;
        case Shadows_High:
            shadowPath.append("media/materials/scripts/shadows_high");
            break;
        default:
            shadowPath.append("media/materials/scripts/shadows_low");
            break;
        }

        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(shadowPath, "FileSystem", "General");
#if ANDROID
        // Initialize RTShader resources
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(Application::InstallationDirectory().toStdString() + "media/RTShaderLib", "FileSystem", "General");
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(Application::InstallationDirectory().toStdString() + "media/RTShaderLib/materials", "FileSystem", "General");
#endif
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    }

    void Renderer::CreateInstancingShaders()
    {
        Ogre::ResourceManager::ResourceMapIterator shader_iter = ((Ogre::ResourceManager*)Ogre::HighLevelGpuProgramManager::getSingletonPtr())->getResourceIterator();
        while(shader_iter.hasMoreElements())
        {
            Ogre::ResourcePtr resource = shader_iter.getNext();
            Ogre::HighLevelGpuProgram* program = dynamic_cast<Ogre::HighLevelGpuProgram*>(resource.get());
            if (program)
            {
                if (program->getType() == Ogre::GPT_VERTEX_PROGRAM && program->getLanguage() == "cg")
                {
                    QString name(program->getName().c_str());
                    if (!name.contains("instanced", Qt::CaseInsensitive) && !name.contains("instancing", Qt::CaseInsensitive))
                    {
                        try
                        {
                            Ogre::HighLevelGpuProgram* cloneProgram = Ogre::HighLevelGpuProgramManager::getSingletonPtr()->createProgram(program->getName()
                                + "/Instanced", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "cg", Ogre::GPT_VERTEX_PROGRAM).get();
                            if (cloneProgram)
                            {
                                cloneProgram->setSourceFile(program->getSourceFile());
                                cloneProgram->setParameter("profiles", program->getParameter("profiles"));
                                cloneProgram->setParameter("entry_point", program->getParameter("entry_point"));
                                cloneProgram->setParameter("compile_arguments", program->getParameter("compile_arguments") + " -DINSTANCING");
                                cloneProgram->load();
                                Ogre::GpuProgramParametersSharedPtr srcParams = program->getDefaultParameters();
                                Ogre::GpuProgramParametersSharedPtr destParams = cloneProgram->getDefaultParameters();
                                destParams->copyMatchingNamedConstantsFrom(*srcParams);
                                if (destParams->_findNamedConstantDefinition("viewProjMatrix"))
                                    destParams->setNamedAutoConstant("viewProjMatrix", Ogre::GpuProgramParameters::ACT_VIEWPROJ_MATRIX); // Add viewproj matrix parameter for SuperShader
                                // If doing shadow mapping, map the lightViewProj matrices to not include world transform
                                if (destParams->_findNamedConstantDefinition("lightViewProj0"))
                                    destParams->setNamedAutoConstant("lightViewProj0", Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX, 0);
                                if (destParams->_findNamedConstantDefinition("lightViewProj1"))
                                    destParams->setNamedAutoConstant("lightViewProj1", Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX, 1);
                                if (destParams->_findNamedConstantDefinition("lightViewProj2"))
                                    destParams->setNamedAutoConstant("lightViewProj2", Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX, 2);
                            }
                            else
                                LogError("Could not clone vertex program " + program->getName() + " for instancing.");
                        }
                        catch(const Ogre::Exception &)
                        {
                            LogError("Could not clone vertex program " + program->getName() + " for instancing.");
                        }
                    }
                }
            }
        }
    }

    int Renderer::WindowWidth() const
    {
        if (renderWindow)
            return renderWindow->OgreRenderWindow()->getWidth();
        return 0;
    }

    int Renderer::WindowHeight() const
    {
        if (renderWindow)
            return renderWindow->OgreRenderWindow()->getHeight();
        return 0;
    }

    void Renderer::SetViewDistance(float distance)
    {
        /// @todo view distance not currently used for anything
        viewDistance = distance;
        // As double to keep human readable and configurable
        framework->Config()->Set(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "view distance", (double)viewDistance);
    }

    void Renderer::DoFullUIRedraw()
    {
        if (framework->IsHeadless())
            return;

        if (!renderWindow->OgreOverlay())
            return;

        PROFILE(Renderer_DoFullUIRedraw);

        UiGraphicsView *view = framework->Ui()->GraphicsView();

        QImage *backBuffer = view->BackBuffer();
        if (!backBuffer)
        {
            LogWarning("Renderer::DoFullUIRedraw: UiGraphicsView does not have a backbuffer initialized!");
            return;
        }

        QSize viewsize(view->viewport()->size());
        QRect viewrect(QPoint(0, 0), viewsize);

        QSize gviewsize(view->size());

        QSize mainwindowSize(framework->Ui()->MainWindow()->size());
        QSize renderWindowSize(renderWindow->OgreRenderWindow()->getWidth(), renderWindow->OgreRenderWindow()->getHeight());

        {
            PROFILE(Renderer_DoFullUIRedraw_backbufferfill);
            backBuffer->fill(Qt::transparent);
        }

        // Paint ui view into buffer
        {
            PROFILE(Renderer_DoFullUIRedraw_GraphicsViewPaint);
            QPainter painter(backBuffer);
            view->viewport()->render(&painter, QPoint(0,0), QRegion(viewrect), QWidget::DrawChildren);
        }

        renderWindow->UpdateOverlayImage(*backBuffer);
    }

    RaycastResult* Renderer::Raycast(int x, int y)
    {
        OgreWorldPtr world = GetActiveOgreWorld();
        if (world)
            return world->Raycast(x, y, 0xFFFFFFFF);
        else
            return 0;
    }

    void Renderer::Render(float frameTime)
    {
        using namespace std;

        // Force a show of the window that has been passed to Ogre. It must be visible at all times to have the Ogre 3D render output show.
        // On windows systems with Pen & Touch input available, Qt doesn't want to actually display the QGgraphicsView even with a call
        // to QGraphicsView::show().
#ifdef WIN32
        if (framework->Ui() && framework->Ui()->GraphicsView())
            ShowWindow(((HWND)framework->Ui()->GraphicsView()->winId()), SW_SHOW);
#endif

        if (!initialized)
        {
            LogError("Renderer::Render called when Renderer is not initialized!");
            return;
        }

        if (frameTime > MAX_FRAME_TIME)
            frameTime = MAX_FRAME_TIME;
            
        PROFILE(Renderer_Render);

        // The message pump must be called on X11 systems,
        // but on Windows it is redundant (Qt already manages this), and has been profiled to take as much as 10ms per frame in some situations.
#ifdef UNIX
        Ogre::WindowEventUtilities::messagePump();
#endif
        // If we are headless, only update the scenegraphs of all Ogre worlds
        if (framework->IsHeadless())
        {
            for (std::map<Scene*, OgreWorldPtr>::const_iterator i = ogreWorlds.begin(); i != ogreWorlds.end(); ++i)
            {
                OgreWorld* world = i->second.get();
                if (world)
                {
                    Ogre::SceneManager* mgr = world->OgreSceneManager();
                    if (mgr)
                    {
                        PROFILE(Ogre_SceneManager_updateSceneGraph);
                        mgr->_updateSceneGraph(0);
                    }
                }
            }
            return;
        }
        
        // If rendering into different size window, dirty the UI view for now & next frame
        if (lastWidth != WindowWidth() || lastHeight != WindowHeight())
        {
            lastWidth = WindowWidth();
            lastHeight = WindowHeight();
            resizedDirty = 2;
#if 0
            backBuffer = QImage(lastWidth, lastHeight, QImage::Format_ARGB32);
            backBuffer.fill(Qt::transparent);
#endif
        }

        UiGraphicsView *view = framework->Ui()->GraphicsView();
        assert(view);

#ifdef DIRECTX_ENABLED
        if (!view->BackBuffer())
        {
            LogError("UI compositing failed! Null backbuffer!");
            return;
        }
        if (view->IsViewDirty() || resizedDirty)
        {
            PROFILE(Renderer_Render_QtBlit);

            QRectF dirtyRectangle = view->DirtyRectangle();
            if (resizedDirty > 0)
                dirtyRectangle = QRectF(0, 0, WindowWidth(), WindowHeight());

            QSize viewsize(view->viewport()->size());
            QRect viewrect(QPoint(0, 0), viewsize);

            QRect dirty((int)dirtyRectangle.left(), (int)dirtyRectangle.top(), (int)dirtyRectangle.width(), (int)dirtyRectangle.height());
            if (dirty.left() < 0) dirty.setLeft(0);
            if (dirty.top() < 0) dirty.setTop(0);
            if (dirty.right() > view->BackBuffer()->width()) dirty.setRight(view->BackBuffer()->width());
            if (dirty.bottom() > view->BackBuffer()->height()) dirty.setBottom(view->BackBuffer()->height());
            if (dirty.left() > dirty.right()) dirty.setLeft(dirty.right());
            if (dirty.top() > dirty.bottom()) dirty.setTop(dirty.bottom());

            {
                PROFILE(QPainter_Render);

                // Paint ui view into buffer
                QPainter painter(view->BackBuffer());
                painter.setCompositionMode(QPainter::CompositionMode_Source);
                painter.fillRect((int)dirtyRectangle.left(), (int)dirtyRectangle.top(), (int)dirtyRectangle.width(), (int)dirtyRectangle.height(),
                    Qt::transparent);
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                view->viewport()->render(&painter, QPoint((int)dirtyRectangle.left(), (int)dirtyRectangle.top()), QRegion(dirty), QWidget::DrawChildren);
            }

            Ogre::D3D9RenderWindow *d3d9rw = dynamic_cast<Ogre::D3D9RenderWindow*>(renderWindow->OgreRenderWindow());
            if (!d3d9rw) // We're not using D3D9.
            {
                DoFullUIRedraw();
            }
            else
            {
                Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(renderWindow->OverlayTextureName());
                Ogre::HardwarePixelBufferSharedPtr pb = texture->getBuffer();
                Ogre::D3D9HardwarePixelBuffer *pixelBuffer = dynamic_cast<Ogre::D3D9HardwarePixelBuffer*>(pb.get());
                assert(pixelBuffer);
                LPDIRECT3DSURFACE9 surface = pixelBuffer->getSurface(d3d9rw ? d3d9rw->getD3D9Device() : 0);
                if (!surface)
                {
                    LogError("Renderer::Render: Failed to get LPDIRECT3DSURFACE9 for surface!");
                    return;
                }
                D3DSURFACE_DESC desc;
                HRESULT hr = surface->GetDesc(&desc);
                if (FAILED(hr))
                {
                    LogError("Renderer::Render: D3DSURFACE_DESC::GetDesc failed!");
                    return;
                }

                if ((uint)dirty.right() > desc.Width) dirty.setRight(desc.Width);
                if ((uint)dirty.bottom() > desc.Height) dirty.setBottom(desc.Height);
                if (dirty.left() > dirty.right()) dirty.setLeft(dirty.right());
                if (dirty.top() > dirty.bottom()) dirty.setTop(dirty.bottom());
                
                // If graphics items are moved/animated outside the scene rect some dirty rect(s) can be outside the scene rect, check for it.
                if (!viewrect.contains(dirty.topLeft()) || !viewrect.contains(dirty.bottomRight()))
                {
                    if (viewrect.intersects(dirty))
                    {
                        LogWarning(QString("Renderer::Render: Dirty rect %1,%2 %3x%4 not inside view %5,%6 %7x%8, correcting.")
                            .arg(dirty.x()).arg(dirty.y()).arg(dirty.width()).arg(dirty.height())
                            .arg(viewrect.x()).arg(viewrect.y()).arg(viewrect.width()).arg(viewrect.height()));
                        dirty = viewrect.intersected(dirty);
                    }
                }

                const int copyableHeight = min<int>(dirty.height(), min<int>(view->BackBuffer()->height() - dirty.top(), desc.Height - dirty.top()));
                const int copyableWidthBytes = 4*min<int>(dirty.width(), min<int>(view->BackBuffer()->width() - dirty.left(), desc.Width - dirty.left()));
                if (copyableHeight <= 0 || copyableWidthBytes <= 0)
                {
                    LogError("Renderer::Render: Nothing to blit!");
                    return;
                }

                D3DLOCKED_RECT lock;

                {
                    PROFILE(LockRect);
                    //HRESULT hr = surface->LockRect(&lock, 0, D3DLOCK_DISCARD); // for full UI redraw.
                    RECT lockRect = { dirty.left(), dirty.top(), dirty.right(), dirty.bottom() };
                    HRESULT hr = surface->LockRect(&lock, &lockRect, 0);
                    if (FAILED(hr))
                    {
                        LogError(QString("Renderer::Render: D3D9SURFACE9::LockRect failed (view %1,%2 %3x%4 rect %5,%6 %7x%8)")
                            .arg(viewrect.x()).arg(viewrect.y()).arg(viewrect.width()).arg(viewrect.height())
                            .arg(dirty.x()).arg(dirty.y()).arg(dirty.width()).arg(dirty.height()));
                        return; // Instead of returning, could try doing a full surface lock. See commented line above.
                    }
                    assert((uint)lock.Pitch >= desc.Width*4);
                }
                char *surfacePtr = (char *)lock.pBits;
                assert(surfacePtr);

                char *scanlines = (char*)view->BackBuffer()->bits();
                assert(scanlines);
                if (!scanlines)
                {
                    LogError("Cannot get UiGraphicsView backbuffer data!");
                    return;
                }

                {
                    PROFILE(surface_memcpy);

    // Full UI blit: Profiled to be slower than the non-D3DLOCK_DISCARD -update.
    //                for(int y = 0; y < desc.Height; ++y)
    //                    memcpy(surfacePtr + y * lock.Pitch, &scanlines[y * backBuffer.width()*4], min<int>(desc.Width, backBuffer.width())*4);

                    // Update the regions that have changed.
                    for(int y = 0; y < copyableHeight; ++y)
                        memcpy(surfacePtr + y * lock.Pitch, &scanlines[dirty.left()*4 + (y+dirty.top()) * view->BackBuffer()->width()*4], copyableWidthBytes);
                }

                {
                    PROFILE(UnlockRect);
                    hr = surface->UnlockRect();
                    if (FAILED(hr))
                    {
                        LogError("Renderer::Render: Unlock Failed!");
                        return;
                    }
                }
            }
        }
#else // Not using the subrectangle blit - just do a full UI blit.
        if (view->IsViewDirty() || resizedDirty)
        {
            DoFullUIRedraw();
        }
#endif

        if (resizedDirty > 0)
            resizedDirty--;
        
#ifdef PROFILING
        // Performance debugging: Toggle the UI overlay visibility based on a debug key.
        // Allows testing whether the GPU is majorly fill rate bound.
        Ogre::Overlay* overlay = renderWindow->OgreOverlay();
        if (overlay)
        {
            if (framework->Input()->IsKeyDown(Qt::Key_F8))
                renderWindow->OgreOverlay()->hide();
            else
                renderWindow->OgreOverlay()->show();
        }
#endif

        // Flush debug geometry into vertex buffer now
        OgreWorldPtr world = GetActiveOgreWorld();
        if (world)
        {
            PROFILE(Renderer_Render_UpdateDebugGeometry)
            world->FlushDebugGeometry();
        }
        
        try
        {
            PROFILE(Renderer_Render_OgreRoot_renderOneFrame);
            if (mainViewport->getCamera())
            {
                // Control the frame time manually
                Ogre::FrameEvent evt;
                evt.timeSinceLastFrame = frameTime;
                ogreRoot->_fireFrameStarted(evt);
                ogreRoot->_updateAllRenderTargets();
                ogreRoot->_fireFrameEnded();
            }
        } catch(const std::exception &e)
        {
            std::cout << "Ogre::Root::renderOneFrame threw an exception: " << (e.what() ? e.what() : "(null)") << std::endl;
            LogError(std::string("Ogre::Root::renderOneFrame threw an exception: ") + (e.what() ? e.what() : "(null)"));
        }

        view->MarkViewUndirty();
    }

    Entity *Renderer::MainCamera()
    {
        Entity *mainCameraEntity = activeMainCamera.lock().get();
        if (!mainCameraEntity)
            return 0;

        if (!mainCameraEntity->ParentScene() || !mainCameraEntity->GetComponent<EC_Camera>())
        {
            SetMainCamera(0);
            return 0;
        }
        return mainCameraEntity;
    }

    EC_Camera *Renderer::MainCameraComponent()
    {
        Entity *mainCamera = MainCamera();
        if (!mainCamera)
            return 0;
        return mainCamera->GetComponent<EC_Camera>().get();
    }

    Scene *Renderer::MainCameraScene()
    {
        Entity *mainCamera = MainCamera();
        Scene *scene = mainCamera ? mainCamera->ParentScene() : 0;
        if (scene)
            return scene;

        // If there is no active camera, return the first scene on the list.
        SceneMap scenes = framework->Scene()->Scenes();
        if (scenes.size() > 0)
            return scenes.begin()->second.get();

        return 0;
    }

    void Renderer::SetMainCamera(Entity *mainCameraEntity)
    {
        activeMainCamera = mainCameraEntity ? mainCameraEntity->shared_from_this() : shared_ptr<Entity>();

        Ogre::Camera *newActiveCamera = 0;
        EC_Camera *cameraComponent = mainCameraEntity ? mainCameraEntity->GetComponent<EC_Camera>().get() : 0;
        if (cameraComponent)
            newActiveCamera = cameraComponent->GetCamera();
        else
        {
            activeMainCamera.reset();
            if (mainCameraEntity)
                LogWarning("Cannot activate camera '" + mainCameraEntity->Name() + "': It does not have a EC_Camera component!");
        }
        if (mainCameraEntity && !mainCameraEntity->ParentScene()) // If the new to-be camera is not in a scene, don't add it as active.
        {
            LogWarning("Cannot activate camera \"" + mainCameraEntity->Name() + "\": It is not attached to a scene!");
            activeMainCamera.reset();
            newActiveCamera = 0;
        }

        if (!activeMainCamera.lock() || !newActiveCamera)
            LogWarning("Setting main window camera to null!");

        // Avoid setting a null camera to Ogre, instead set a dummy placeholder camera if user wanted to set to null.
        if (!newActiveCamera)
            newActiveCamera = dummyDefaultCamera;

        if (mainViewport)
        {
            mainViewport->setCamera(newActiveCamera);
            if (compositionHandler)
                compositionHandler->CameraChanged(mainViewport, newActiveCamera);
        }

        emit MainCameraChanged(mainCameraEntity);
    }

    UiPlane *Renderer::CreateUiPlane(const QString &name)
    {
        UiPlane *p = new UiPlane(framework, renderWindow);
        p->setObjectName(name);
        p->UpdateOgreOverlay();
        uiPlanes.push_back(p);
        return p;
    }

    void Renderer::DeleteUiPlane(UiPlane *plane)
    {
        if (!plane)
            return;

        for(size_t i = 0; i < uiPlanes.size(); ++i)
            if (uiPlanes[i] == plane)
            {
                uiPlanes.erase(uiPlanes.begin() + i);
                delete plane;
                return;
            }
        LogError("Trying to delete nonexisting UiPlane!");
    }

    OgreWorldPtr Renderer::GetActiveOgreWorld() const
    {
        Entity *entity = activeMainCamera.lock().get();
        if (!entity)
            return OgreWorldPtr();
        Scene *scene = entity->ParentScene();
        if (scene)
            return scene->GetWorld<OgreWorld>();
        else
            return OgreWorldPtr();
    }
    
    Ogre::Camera *Renderer::MainOgreCamera() const
    {
        Ogre::Viewport *mainViewport = MainViewport();
        return mainViewport ? mainViewport->getCamera() : 0;
    }

    Ogre::RenderWindow *Renderer::GetCurrentRenderWindow() const
    {
        return renderWindow->OgreRenderWindow();
    }

    std::string Renderer::GetUniqueObjectName(const std::string &prefix)
    {
        return QString("%1_%2").arg(prefix.c_str()).arg(uniqueObjectId++).toStdString();
    }

    void Renderer::AddResourceDirectory(const QString &qdirectory)
    {
        std::string directory = qdirectory.toStdString();

        // Check to not add the same directory more than once
        for(uint i = 0; i < resourceDirectories.size(); ++i)
            if (resourceDirectories[i] == directory)
                return;

        Ogre::ResourceGroupManager& resgrpmgr = Ogre::ResourceGroupManager::getSingleton();

        std::string groupname = "grp" + QString::number(uniqueGroupId++).toStdString();

        // Check if resource group already exists (should not).
        bool exists = false;
        Ogre::StringVector groups = resgrpmgr.getResourceGroups();
        for(uint i = 0; i < groups.size(); ++i)
        {
            if (groups[i] == groupname)
            {
                exists = true;
                break;
            }
        }

        // Create if doesn't exist
        if (!exists)
        {
            try
            {
                resgrpmgr.createResourceGroup(groupname);
            }
            catch(...) {}
        }
        
        // Add directory as a resource location, then initialize group
        try
        {
            resgrpmgr.addResourceLocation(directory, "FileSystem", groupname);
            resgrpmgr.initialiseResourceGroup(groupname);
        }
        catch(...) {}

        resourceDirectories.push_back(directory);
    }

    void Renderer::OnScriptEngineCreated(QScriptEngine* engine)
    {
        qScriptRegisterQObjectMetaType<EC_Placeable*>(engine);
        qScriptRegisterQObjectMetaType<EC_Camera*>(engine);
        qScriptRegisterQObjectMetaType<UiPlane*>(engine);
        // Ogre asset types and ptr typedefs
        qScriptRegisterQObjectMetaType<TextureAsset*>(engine);
        qScriptRegisterQObjectMetaType<OgreMeshAsset*>(engine);
        qScriptRegisterQObjectMetaType<OgreMaterialAsset*>(engine);
        qScriptRegisterQObjectMetaType<OgreSkeletonAsset*>(engine);
        qScriptRegisterQObjectMetaType<OgreParticleAsset*>(engine);
        qScriptRegisterMetaType<TextureAssetPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
        qScriptRegisterMetaType<OgreMeshAssetPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
        qScriptRegisterMetaType<OgreMaterialAssetPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
        qScriptRegisterMetaType<OgreSkeletonAssetPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
        qScriptRegisterMetaType<OgreParticleAssetPtr>(engine, qScriptValueFromBoostSharedPtr, qScriptValueToBoostSharedPtr);
    }
}
