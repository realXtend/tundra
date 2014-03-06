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
#include "CoreJsonUtils.h"
#include "Entity.h"
#include "SceneAPI.h"
#include "Profiler.h"
#include "AssetCache.h"
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



// Clamp elapsed frame time to avoid Ogre controllers going crazy
static const float MAX_FRAME_TIME = 0.1f;

// Default texture budget (megabytes)
static const int DEFAULT_TEXTURE_BUDGET = 512;
// Minimum texture budget
static const int MINIMUM_TEXTURE_BUDGET = 1;

static const char * const cD3D9RenderSystemName = "Direct3D9 Rendering Subsystem";
static const char * const cD3D9RenderSystemFilename  = "RenderSystem_Direct3D9";
static const char * const cOglRenderSystemName  = "OpenGL Rendering Subsystem";
static const char * const cOglRenderSystemFilename  = "RenderSystem_GL";

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

    Renderer::Renderer(Framework* fw) :
        initialized(false),
        framework(fw),
        defaultScene(0),
        dummyDefaultCamera(0),
        mainViewport(0),
        overlaySystem(0),
        uniqueObjectId(0),
        renderWindow(0),
        lastWidth(0),
        lastHeight(0),
        resizedDirty(0),
        viewDistance(500.0f),
        shadowQuality(Shadows_High),
        textureQuality(Texture_Normal),
        textureBudget(DEFAULT_TEXTURE_BUDGET)
#ifdef ANDROID
        , staticPluginLoader(0)
        , shaderGenerator(0)
#endif
    {
        compositionHandler = new OgreCompositionHandler();
        logListener = new OgreLogListener(fw->HasCommandLineParameter("--hideBenignOgreMessages") ||
            fw->HasCommandLineParameter("--hide_benign_ogre_messages")); /**< @todo Remove support for the deprecated underscore version at some point. */

        timerFrequency = GetCurrentClockFreq();

        ConfigAPI &cfg = *framework->Config();
        const ConfigData renderingCfg(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING);
        viewDistance = cfg.DeclareSetting(renderingCfg, "view distance", (double)viewDistance).toFloat(); // (as double to keep human-readable and configurable)
        shadowQuality = static_cast<ShadowQualitySetting>(cfg.DeclareSetting(renderingCfg, "shadow quality", shadowQuality).toInt());
        textureQuality = static_cast<TextureQualitySetting>(cfg.DeclareSetting(renderingCfg, "texture quality", textureQuality).toInt());
        cfg.DeclareSetting(renderingCfg, "soft shadow", false);
        cfg.DeclareSetting(renderingCfg, "rendering plugin",
#ifdef _WINDOWS
            cD3D9RenderSystemName
#else
            cOglRenderSystemName
#endif
            );
        textureBudget = framework->Config()->DeclareSetting(renderingCfg, "texture budget", DEFAULT_TEXTURE_BUDGET).toInt();
    }

    Renderer::~Renderer()
    {
        if (framework->Ui()->MainWindow())
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

    void Renderer::Initialize()
    {
        assert(!initialized);
        if (initialized)
        {
            LogError("Renderer::Initialize: Called when Renderer has already been initialized!");
            return;
        }

        std::string logFilePath, renderSystemName;
        Ogre::RenderSystem *renderSystem = 0;

        LogInfo("Renderer: Initializing Ogre");

        // Create Ogre root with log file
        QDir logDir(Application::UserDataDirectory());
        if (!logDir.exists("logs"))
            logDir.mkdir("logs");
        logDir.cd("logs");
        logFilePath = logDir.absoluteFilePath("Ogre.log").toStdString(); /**< @todo Unicode support */

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
        ogreRoot = MAKE_SHARED(Ogre::Root, "", "", logFilePath);

#include "EnableMemoryLeakCheck.h"

        // Write/read config.
        ConfigData configData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING);

        if (framework->HasCommandLineParameter("--texturebudget"))
        {
            QStringList sizeParam = framework->CommandLineParameters("--texturebudget");
            if (sizeParam.size() > 0)
                SetTextureBudget(sizeParam.first().toInt());
        }

        QString renderingConfig = QDir(Application::InstallationDirectory()).absoluteFilePath("tundra-rendering-ogre.json");
        const bool hasOgreConfig = framework->HasCommandLineParameter("--ogreConfig");
        const QStringList ogreConfigs = framework->CommandLineParameters("--ogreConfig");
        if (hasOgreConfig)
        {
            if (ogreConfigs.empty())
                LogError("Renderer::Initialize: --ogreConfig specified without a value. Using the default config.");
            else
                renderingConfig = framework->CommandLineParameters("--ogreConfig").first();
        }

        // Load plugins
        const QStringList loadedPlugins = LoadOgrePlugins(renderingConfig);

        // Read the default renderSystem from Config API.
        renderSystemName = framework->Config()->Read(configData, "rendering plugin").toString().toStdString();

        // Headless renderer is selected from Ogre config if defined.
        // This can be overridden with --d3d9 and --opengl cmd line params.
        if (framework->IsHeadless())
        {
            const QString headlessRenderer = HeadlessRenderingPluginName(renderingConfig);
            if (!headlessRenderer.isEmpty())
            {
                // Verify the headless renderer was actually loaded.
                // If not, try force loading the operating system default renderer.
                // Otherwise we will crash below as we cant find a valid renderer.
                const QString headlessRendereFileName = HeadlessRenderingPluginFilename(renderingConfig);
                if (!headlessRendereFileName.isEmpty() && !loadedPlugins.contains(headlessRendereFileName, Qt::CaseInsensitive))
                {
                    LogWarning(QString("Renderer: Failed to load headless render plugin %1, force loading system default renderer.").arg(headlessRendereFileName));
#ifdef _WINDOWS
                    if (!LoadOgrePlugin(renderingConfig, cD3D9RenderSystemFilename))
                        LoadOgrePlugin(renderingConfig, cOglRenderSystemFilename);
#else
                    LoadOgrePlugin(renderingConfig, cOglRenderSystemFilename);
#endif
                }
                else
                    renderSystemName = headlessRenderer.toStdString();
            }
        }

#ifdef _WINDOWS
        // If --direct3d9 is specified, it overrides the option that was set in config.
        if (framework->HasCommandLineParameter("--d3d9") || framework->HasCommandLineParameter("--direct3d9"))
            renderSystemName = cD3D9RenderSystemName;
#endif

        // If --opengl is specified, it overrides the option that was set in config.
        if (framework->HasCommandLineParameter("--opengl"))
            renderSystemName = cOglRenderSystemName;

        // Ask Ogre if rendering system is available
        renderSystem = ogreRoot->getRenderSystemByName(renderSystemName);

#ifdef _WINDOWS
        // If windows did not have the config/headless renderer fallback to Direct3D.
        if (!renderSystem && renderSystemName != cD3D9RenderSystemName)
            renderSystem = ogreRoot->getRenderSystemByName(cD3D9RenderSystemName);

        // If windows did not have Direct3D fallback to OpenGL.
        if (!renderSystem && renderSystemName != cOglRenderSystemName)
            renderSystem = ogreRoot->getRenderSystemByName(cOglRenderSystemName);
#endif

#ifdef ANDROID
        // Android: use the first available renderSystem, should be GLES2
        if (!renderSystem)
            renderSystem = ogreRoot->getAvailableRenderers().at(0);
#endif

        if (!renderSystem)
            throw Exception("Could not find Ogre renderSystem.");

        // Report rendering plugin to log so user can check what actually got loaded
        LogInfo(QString("Renderer: Using '%1'").arg(renderSystem->getName().c_str()));

        // Allow PSSM mode shadows only on DirectX
        // On OpenGL (arbvp & arbfp) it runs out of vertex shader outputs
        if (shadowQuality == Shadows_High && renderSystem->getName() != cD3D9RenderSystemName)
            shadowQuality = Shadows_Low;

        // This is needed for QWebView to not lock up!!!
        Ogre::ConfigOptionMap& map = renderSystem->getConfigOptions();
        if (map.find("Floating-point mode") != map.end())
            renderSystem->setConfigOption("Floating-point mode", "Consistent");

        if (framework->IsHeadless())
        {
            /** If we are running in headless mode, initialize the Ogre 'DefaultHardwareBufferManager',
                which is a software-emulation mode for Ogre's HardwareBuffers (i.e. can create meshes into CPU memory).

                @note If we are using a renderer that is intended for headless use, it may have already created the
                DefaultHardwareBufferManager singleton. This can be the case if the plugin knows Ogre::Root is not
                going to be initialized. */
            if (!Ogre::HardwareBufferManager::getSingletonPtr())
            {
#include "DisableMemoryLeakCheck.h"
                new Ogre::DefaultHardwareBufferManager();
#include "EnableMemoryLeakCheck.h"
            }
        }
        else
        {
            // Set the found rendering system
            ogreRoot->setRenderSystem(renderSystem);

            // Instantiate overlay system
            overlaySystem = new Ogre::OverlaySystem();

            // Initialize but don't create rendering window yet
            ogreRoot->initialise(false);

            try
            {
                renderWindow = new RenderWindow();

                QString windowTitle = Application::FullIdentifier();
                bool fullscreen = framework->HasCommandLineParameter("--fullscreen");
                int width = framework->Ui()->GraphicsView()->viewport()->size().width();
                int height = framework->Ui()->GraphicsView()->viewport()->size().height();
                int window_left = 0;
                int window_top = 0;

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
                    renderWindow->CreateRenderWindow(framework->Ui()->MainWindow(), windowTitle, width, height, window_left, window_top, fullscreen, framework);
                }
                else if (framework->HasCommandLineParameter("--nouicompositing"))
                    renderWindow->CreateRenderWindow(0, windowTitle, width, height, window_left, window_top, fullscreen, framework);
                else // Normally, we want to render Ogre onto the UiGraphicsview viewport window.
                {
                    // Even if the user has requested fullscreen mode, init Ogre in windowed mode, since the main graphics view is not a top-level window, and Ogre cannot
                    // initialize into fullscreen with a non-top-level window handle. As D3D9 is initialized in windowed mode, this has the effect that vsync cannot be enabled.
                    // To set up vsync, specify the
                    if (framework->CommandLineParameters("--vsync").length() > 0 && ParseBool(framework->CommandLineParameters("--vsync").first()))
                        LogWarning("--vsync was specified, but Ogre is initialized in windowed mode to a non-top-level window. VSync will probably *not* be active. To enable vsync in full screen mode, "
                            "specify the flags --vsync, --fullscreen and --ogrecapturetopwindow together.");
                    renderWindow->CreateRenderWindow(framework->Ui()->GraphicsView()->viewport(), windowTitle, width, height, window_left, window_top, false, framework);
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
            LoadOgreResourceLocations(renderingConfig);
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
        framework->Config()->Write(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "shadow quality", quality);
    }

    void Renderer::SetTextureQuality(TextureQualitySetting quality)
    {
        // We cannot effect the new setting immediately, so save only to config
        framework->Config()->Write(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "texture quality", quality);
    }
    
    void Renderer::SetTextureBudget(int budget)
    {
        textureBudget = Max(budget, MINIMUM_TEXTURE_BUDGET);
        framework->Config()->Write(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "texture budget", textureBudget);
    }

    float Renderer::TextureBudgetUse(size_t loadDataSize) const
    {
        return (float)(Ogre::TextureManager::getSingletonPtr()->getMemoryUsage() + loadDataSize) / (1024.f*1024.f) / (float)textureBudget;
    }

    QString Renderer::HeadlessRenderingPluginName(const QString &renderingConfig) const
    {
        bool ok = false;
        QVariantMap configData = TundraJson::ParseFile(renderingConfig, true, &ok).toMap();
        if (!ok)
        {
            LogError("HeadlessRenderingPluginName: Failed to parse Ogre config file: " + renderingConfig);
            return "";
        }
        QVariantMap pluginsSection = TundraJson::Value(configData, "Plugins").toMap();
        if (pluginsSection.isEmpty())
        {
            LogError("HeadlessRenderingPluginName: 'Plugins' section is empty in config file: " + renderingConfig);
            return "";
        }

        QVariantMap platformSection = TundraJson::Value(pluginsSection, RenderingConfigPlatform(), QVariantMap()).toMap();
        QVariantMap headlessRenderers = TundraJson::Value(platformSection, "Headless", QVariantMap()).toMap();
        return TundraJson::Value(headlessRenderers, "Renderer", "").toString();
    }

    QString Renderer::HeadlessRenderingPluginFilename(const QString &renderingConfig) const
    {
        bool ok = false;
        QVariantMap configData = TundraJson::ParseFile(renderingConfig, true, &ok).toMap();
        if (!ok)
        {
            LogError("HeadlessRenderingPluginName: Failed to parse Ogre config file: " + renderingConfig);
            return "";
        }
        QVariantMap pluginsSection = TundraJson::Value(configData, "Plugins").toMap();
        if (pluginsSection.isEmpty())
        {
            LogError("HeadlessRenderingPluginName: 'Plugins' section is empty in config file: " + renderingConfig);
            return "";
        }

        QVariantMap platformSection = TundraJson::Value(pluginsSection, RenderingConfigPlatform(), QVariantMap()).toMap();
        QVariantMap headlessSection = TundraJson::Value(platformSection, "Headless", QVariantMap()).toMap();
        QStringList plugins = TundraJson::Value(headlessSection, "Plugins", QStringList()).toStringList();
        // This is a semi hack, its just a list of Ogre plugins. Return the first
        // that matches the Ogre naming convention "RenderSystem_X".
        foreach(const QString pluginDllName, plugins)
            if (pluginDllName.startsWith("RenderSystem_", Qt::CaseInsensitive))
                return pluginDllName;
        return "";
    }

#ifdef ANDROID

    QStringList Renderer::LoadOgrePlugins()
    {
        // On Android, load Ogre plugins statically
        staticPluginLoader = new Ogre::StaticPluginLoader();
        staticPluginLoader->load();
        return QStringList();
    }

#else

    QStringList Renderer::LoadOgrePlugins(const QString &renderingConfig)
    {
        QStringList loadedPlugins;

        bool ok = false;
        QVariantMap configData = TundraJson::ParseFile(renderingConfig, true, &ok).toMap();
        if (!ok)
        {
            LogError("LoadOgrePlugins: Failed to parse Ogre config file: " + renderingConfig);
            return loadedPlugins;
        }
        QVariantMap pluginsSection = TundraJson::Value(configData, "Plugins").toMap();
        if (pluginsSection.isEmpty())
        {
            LogError("LoadOgrePlugins: 'Plugins' section is empty in config file: " + renderingConfig);
            return loadedPlugins;
        }

        // Read platform specific section.
        QVariantMap platformSection = TundraJson::Value(pluginsSection, RenderingConfigPlatform()).toMap();

        // Plugin folder. Relative paths will resolved to the config file dir.
        QString pluginFolder = TundraJson::Value(platformSection, "Folder", "./").toString();
        if (QFileInfo(pluginFolder).isRelative())
            pluginFolder = QFileInfo(renderingConfig).dir().absoluteFilePath(pluginFolder);
        QDir pluginsDir(pluginFolder);
        if (!pluginsDir.exists())
        {
            LogError(QString("LoadOgrePlugins: %1 platform plugins folder does not exist: %2")
                .arg(RenderingConfigPlatform()).arg(pluginFolder));
            return loadedPlugins;
        }

        // Platform plugin debug postfix.
        QString pluginPostFix = "";
#ifdef _DEBUG
        pluginPostFix = TundraJson::Value(platformSection, "DebugPostfix", "").toString();
#endif

        // Does this platform have headless plugin declaration?
        QStringList plugins;
        if (framework->IsHeadless())
        {
            QVariantMap headlessSection = TundraJson::Value(platformSection, "Headless").toMap();
            if (headlessSection.contains("Plugins"))
                plugins += TundraJson::Value(headlessSection, "Plugins", QStringList()).toStringList();
        }

        QStringList platformPlugins = TundraJson::Value(platformSection, "Plugins", QStringList()).toStringList();

        // No headless section found? Load the normal plugins.
        if (plugins.isEmpty())
            plugins = platformPlugins;

        // Cmd line param force to a certain renderer?
#ifdef _WINDOWS
        if (framework->HasCommandLineParameter("--d3d9") || framework->HasCommandLineParameter("--direct3d9"))
        {
            if (platformPlugins.contains("RenderSystem_Direct3D9", Qt::CaseSensitive) && !plugins.contains("RenderSystem_Direct3D9", Qt::CaseSensitive))
                plugins += "RenderSystem_Direct3D9";
        }
#endif
        if (framework->HasCommandLineParameter("--opengl"))
        {
            if (platformPlugins.contains("RenderSystem_GL", Qt::CaseSensitive) && !plugins.contains("RenderSystem_GL", Qt::CaseSensitive))
                plugins += "RenderSystem_GL";
        }

        // Common plugins for all platforms
        /** @note Do not call LoadOgrePlugin here for each item, this might cleanup this code up but
            that function re-reads the config every time so it would be slower. */
        plugins += TundraJson::Value(pluginsSection, "Common", QStringList()).toStringList();
        foreach(const QString &pluginName, plugins)
        {
            QString absolutePluginPath = QDir::toNativeSeparators(pluginsDir.absoluteFilePath(pluginName + pluginPostFix));
            try
            {
#ifndef Q_WS_MAC
                ogreRoot->loadPlugin(absolutePluginPath.toStdString());
#else
                // On Mac, Ogre is hardcoded to look for frameworks inside <EXECUTABLE_PATH_HERE>/Contents/Frameworks
                // Instead absolute paths, pass only the plugin names
                ogreRoot->loadPlugin(pluginName.toStdString());
#endif
                loadedPlugins << pluginName;
            }
            catch(Ogre::Exception &e)
            {
                LogError(QString("LoadOgrePlugins: Plugin %1 failed to load: %2")
                    .arg(absolutePluginPath).arg(e.what()));
            }
        }
        return loadedPlugins;
    }

    bool Renderer::LoadOgrePlugin(const QString &renderingConfig, const QString &pluginName)
    {
        bool ok = false;
        QVariantMap configData = TundraJson::ParseFile(renderingConfig, true, &ok).toMap();
        if (!ok)
        {
            LogError("LoadOgrePlugin: Failed to parse Ogre config file: " + renderingConfig);
            return false;
        }
        QVariantMap pluginsSection = TundraJson::Value(configData, "Plugins").toMap();
        if (pluginsSection.isEmpty())
        {
            LogError("LoadOgrePlugin: 'Plugins' section is empty in config file: " + renderingConfig);
            return false;
        }

        // Read platform specific section.
        QVariantMap platformSection = TundraJson::Value(pluginsSection, RenderingConfigPlatform()).toMap();

        // Plugin folder. Relative paths will resolved to the config file dir.
        QString pluginFolder = TundraJson::Value(platformSection, "Folder", "./").toString();
        if (QFileInfo(pluginFolder).isRelative())
            pluginFolder = QFileInfo(renderingConfig).dir().absoluteFilePath(pluginFolder);
        QDir pluginsDir(pluginFolder);
        if (!pluginsDir.exists())
        {
            LogError(QString("LoadOgrePlugin: %1 platform plugins folder does not exist: %2")
                .arg(RenderingConfigPlatform()).arg(pluginFolder));
            return false;
        }

        // Platform plugin debug postfix.
        QString pluginPostFix = "";
#ifdef _DEBUG
        pluginPostFix = TundraJson::Value(platformSection, "DebugPostfix", "").toString();
#endif

        QString absolutePluginPath = QDir::toNativeSeparators(pluginsDir.absoluteFilePath(pluginName + pluginPostFix));
        try
        {
#ifndef Q_WS_MAC
            ogreRoot->loadPlugin(absolutePluginPath.toStdString());
#else
            // On Mac, Ogre is hardcoded to look for frameworks inside <EXECUTABLE_PATH_HERE>/Contents/Frameworks
            // Instead absolute paths, pass only the plugin names
            ogreRoot->loadPlugin(pluginName.toStdString());
#endif
            return true;
        }
        catch(Ogre::Exception &e)
        {
            LogError(QString("LoadOgrePlugin: Plugin %1 failed to load: %2")
                .arg(absolutePluginPath).arg(e.what()));
        }
        return false;
    }

#endif

    void Renderer::LoadOgreResourceLocations(const QString &renderingConfig)
    {
        bool ok = false;
        QVariantMap configData = TundraJson::ParseFile(renderingConfig, true, &ok).toMap();
        if (!ok)
        {
            LogError("LoadOgreResourceLocations: Failed to parse Ogre config file: " + renderingConfig);
            return;
        }
        QVariantMap resourcesSection = TundraJson::Value(configData, "Resources").toMap();
        if (resourcesSection.isEmpty())
        {
            LogError("LoadOgreResourceLocations: 'Resources' section is empty in config file: " + renderingConfig);
            return;
        }

        QDir configDir = QFileInfo(renderingConfig).dir();

        // Load common resource locations first.
        ProcessOgreResourceLocations(TundraJson::Value(resourcesSection, "Common").toMap(), configDir);

        // Load platform specific resource locations first.
        ProcessOgreResourceLocations(TundraJson::Value(resourcesSection, RenderingConfigPlatform()).toMap(), configDir);

        // Load super shader program definitions directory according to the shadow quality level.
        ProcessOgreResourceLocation(QString("media/materials/scripts/") + (shadowQuality == Shadows_Off ? "shadows_off" : (shadowQuality == Shadows_High ? "shadows_high" : "shadows_low")),
            QString::fromStdString(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME));

        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        // Now that the resource groups are initialized, add Tundra cache path as a resource group.
        // This group is used for various Ogre asset type threaded loading to provide the disk source from Tundra cache.
        // Added after group initialization as we don't want Ogre to iterate over potentially thousands of files during startup.
        if (framework->Asset()->Cache())
            ProcessOgreResourceLocation(framework->Asset()->Cache()->CacheDirectory(), QString::fromStdString(OgreRenderingModule::CACHE_RESOURCE_GROUP));
    }

    bool Renderer::InitializeOgreResourceGroup(const QString &ogreResourceGroup)
    {
        std::string resourceGroup = ogreResourceGroup.toStdString();
        Ogre::ResourceGroupManager &resourceManager = Ogre::ResourceGroupManager::getSingleton();
        if (resourceManager.resourceGroupExists(resourceGroup))
        {
            if (!resourceManager.isResourceGroupInitialised(resourceGroup))
                resourceManager.initialiseResourceGroup(resourceGroup);
            return true;
        }
        return false;
    }

    void Renderer::ProcessOgreResourceLocation(const QString &directoryPath, const QString &ogreResourceGroup, bool removeLocation)
    {
        QString resourceLocation = directoryPath;
        if (QFileInfo(resourceLocation).isRelative())
            resourceLocation = QDir(Application::InstallationDirectory()).absoluteFilePath(resourceLocation);
        resourceLocation = QDir::toNativeSeparators(resourceLocation);

        if (!removeLocation)
        {
            if (!Ogre::ResourceGroupManager::getSingleton().resourceLocationExists(resourceLocation.toStdString(), ogreResourceGroup.toStdString()))
            {
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(resourceLocation.toStdString(),
                    "FileSystem", ogreResourceGroup.toStdString());
                LogDebug(QString("ProcessOgreResourceLocation: Added %1 to Ogre resource group %2.")
                    .arg(resourceLocation).arg(ogreResourceGroup));
            }
            else
                LogWarning(QString("ProcessOgreResourceLocation: Location %1 for Ogre resource group %2 already exists.")
                    .arg(resourceLocation).arg(ogreResourceGroup));
        }
        else
        {
            Ogre::ResourceGroupManager::getSingleton().removeResourceLocation(resourceLocation.toStdString(), ogreResourceGroup.toStdString());
            LogDebug(QString("ProcessOgreResourceLocation: Removed %1 from Ogre resource group %2.")
                .arg(resourceLocation).arg(ogreResourceGroup));
        }
    }

    void Renderer::ProcessOgreResourceLocations(const QVariantMap &resources, const QDir &rootDirectory, bool removeLocation)
    {
        foreach(const QString &ogreResourceGroup, resources.keys())
        {
            foreach(const QVariant &locationVariant, resources[ogreResourceGroup].toList())
            {
                QVariantMap location = locationVariant.toMap();
                foreach(const QString &ogreResourceLocationType, location.keys())
                {
                    QString ogreResourceLocation = location[ogreResourceLocationType].toString();
                    if (QFileInfo(ogreResourceLocation).isRelative())
                        ogreResourceLocation = rootDirectory.absoluteFilePath(ogreResourceLocation);
                    ogreResourceLocation = QDir::toNativeSeparators(ogreResourceLocation);

                    if (!removeLocation)
                    {
                        if (!Ogre::ResourceGroupManager::getSingleton().resourceLocationExists(ogreResourceLocation.toStdString(), ogreResourceGroup.toStdString()))
                        {
                            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(ogreResourceLocation.toStdString(),
                                ogreResourceLocationType.toStdString(), ogreResourceGroup.toStdString());
                            LogDebug(QString("ProcessOgreResourceLocations: Added %1 with type %2 to Ogre resource group %3.")
                                .arg(ogreResourceLocation).arg(ogreResourceLocationType).arg(ogreResourceGroup));
                        }
                        else
                            LogWarning(QString("ProcessOgreResourceLocation: Location %1 for Ogre resource group %2 already exists.")
                                .arg(ogreResourceLocation).arg(ogreResourceGroup));
                    }
                    else
                    {
                        Ogre::ResourceGroupManager::getSingleton().removeResourceLocation(ogreResourceLocation.toStdString(), ogreResourceGroup.toStdString());
                        LogDebug(QString("ProcessOgreResourceLocations: Removed %1 from Ogre resource group %2.")
                            .arg(ogreResourceLocation).arg(ogreResourceGroup));
                    }
                }
            }
        }
    }

    QString Renderer::RenderingConfigPlatform() const
    {
#ifdef ANDROID
        return QString("Android");
#elif defined(Q_WS_WIN)
        return QString("Windows");
#elif defined(Q_WS_MAC)
        return QString("Mac");
#elif defined(Q_WS_X11)
        return QString("Linux");
#else
        return QString();
#endif
    }

    void Renderer::EmitDeviceCreated()
    {
        LogDebug("[Renderer]: Emitting DeviceCreated");
        emit DeviceCreated();

        // The UI overlay has been recreated, do a full update.
        DoFullUIRedraw();
    }

    void Renderer::EmitDeviceReleased()
    {
        LogDebug("[Renderer]: Emitting DeviceReleased");
        emit DeviceReleased();
    }

    void Renderer::EmitDeviceLost()
    {
        LogDebug("[Renderer]: Emitting DeviceLost");
        emit DeviceLost();
    }

    void Renderer::EmitDeviceRestored()
    {
        LogDebug("[Renderer]: Emitting DeviceRestored");
        emit DeviceRestored();
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
        framework->Config()->Write(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "view distance", (double)viewDistance);
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
        OgreWorldPtr world = ActiveOgreWorld();
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
        if (framework->Ui()->GraphicsView())
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
        OgreWorldPtr world = ActiveOgreWorld();
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

        if (!mainCameraEntity->ParentScene() || !mainCameraEntity->Component<EC_Camera>())
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
        return mainCamera->Component<EC_Camera>().get();
    }

    Scene *Renderer::MainCameraScene()
    {
        Entity *mainCamera = MainCamera();
        Scene *scene = mainCamera ? mainCamera->ParentScene() : 0;
        if (scene)
            return scene;

        // If there is no active camera, return the first scene on the list.
        const SceneMap &scenes = framework->Scene()->Scenes();
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

    OgreWorldPtr Renderer::ActiveOgreWorld() const
    {
        Entity *entity = activeMainCamera.lock().get();
        if (!entity)
            return OgreWorldPtr();
        Scene *scene = entity->ParentScene();
        if (scene)
            return scene->Subsystem<OgreWorld>();
        else
            return OgreWorldPtr();
    }
    
    Ogre::Camera *Renderer::MainOgreCamera() const
    {
        Ogre::Viewport *mainViewport = MainViewport();
        return mainViewport ? mainViewport->getCamera() : 0;
    }

    Ogre::RenderWindow *Renderer::CurrentRenderWindow() const
    {
        return renderWindow->OgreRenderWindow();
    }

    std::string Renderer::GenerateUniqueObjectName(const std::string &prefix)
    {
        return QString("%1_%2").arg(prefix.c_str()).arg(uniqueObjectId++).toStdString();
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
