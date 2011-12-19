// For conditions of distribution and use, see copyright notice in license.txt

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

#include "Application.h"
#include "UiGraphicsView.h"
#include "Scene.h"
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

Q_DECLARE_METATYPE(EC_Placeable*);
Q_DECLARE_METATYPE(EC_Camera*);

// Clamp elapsed frame time to avoid Ogre controllers going crazy
static const float MAX_FRAME_TIME = 0.1f;

#if defined(DIRECTX_ENABLED) && !defined(WIN32)
#undef DIRECTX_ENABLED
#endif

#ifdef DIRECTX_ENABLED
#undef SAFE_DELETE
#undef SAFE_DELETE_ARRAY

#include <d3d9.h>
#include <RenderSystems/Direct3D9/OgreD3D9HardwarePixelBuffer.h>
#include <RenderSystems/Direct3D9/OgreD3D9RenderWindow.h>
#endif

#include <QCloseEvent>
#include <QSize>
#include <QDir>

#ifdef PROFILING
#include "InputAPI.h"
#endif

#include "MemoryLeakCheck.h"

namespace OgreRenderer
{
    /// @cond PRIVATE
    class OgreLogListener : public Ogre::LogListener
    {
        bool hideBenignOgreMessages;
    public:
        explicit OgreLogListener(bool hideBenignOgreMessages_)
        :hideBenignOgreMessages(hideBenignOgreMessages_)
        {
        }

        void messageLogged(const Ogre::String &message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String &logName)
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

            if (lml == Ogre::LML_CRITICAL)
                // Some Ogre Critical messages are actually not errors. For example MaterialSerializer's log messages.
                LogError(message);
            else if (lml == Ogre::LML_TRIVIAL)
                LogDebug(message);
            else // lml == Ogre::LML_NORMAL here.
            {
                // Because Ogre distinguishes different log levels *VERY POORLY* (practically all messages come in the LML_NORMAL), 
                // we need to use manual format checks to guess between Info/Warning/Error/Critical channels.
                if ((str.contains("error ", Qt::CaseInsensitive) || str.contains("error:", Qt::CaseInsensitive)) || str.contains("critical", Qt::CaseInsensitive))
                    LogError(message);
                else if (str.contains("warning", Qt::CaseInsensitive) || str.contains("unexpected", Qt::CaseInsensitive) || str.contains("unknown", Qt::CaseInsensitive) || str.contains("cannot", Qt::CaseInsensitive) || str.contains("can not", Qt::CaseInsensitive))
                    LogWarning(message);
                else if (str.startsWith("*-*-*"))
                    LogInfo(message);
                else
                    LogDebug(message);
            }
        }
    };
    /// @endcond

    Renderer::Renderer(Framework* framework, const std::string& config, const std::string& plugins, const std::string& window_title) :
        initialized_(false),
        framework_(framework),
//        bufferManager(0), ///< @todo Unused - delete for good?
        defaultScene_(0),
        dummyDefaultCamera(0),
        mainViewport(0),
        object_id_(0),
        group_id_(0),
        config_filename_(config),
        plugins_filename_(plugins),
        window_title_(window_title),
        renderWindow(0),
        lastWidth(0),
        lastHeight(0),
        resizedDirty(0),
        viewDistance(500.0f),
        shadowQuality(Shadows_High),
        textureQuality(Texture_Normal)
    {
        compositionHandler = new OgreCompositionHandler();
        logListener = new OgreLogListener(framework_->HasCommandLineParameter("--hide_benign_ogre_messages"));

        timerFrequency = GetCurrentClockFreq();
        PrepareConfig();
    }

    Renderer::~Renderer()
    {
        if (framework_->Ui() && framework_->Ui()->MainWindow())
            framework_->Ui()->MainWindow()->SaveWindowSettingsToFile();

        // Delete all worlds that still exist
        ogreWorlds_.clear();
        
        // Delete the default camera & scene
        if (defaultScene_)
        {
            defaultScene_->destroyCamera(dummyDefaultCamera);
            dummyDefaultCamera = 0;
            root_->destroySceneManager(defaultScene_);
            defaultScene_ = 0;
        }
        
        root_.reset();
        SAFE_DELETE(compositionHandler);
        SAFE_DELETE(logListener);
        SAFE_DELETE(renderWindow);
    }

    void Renderer::PrepareConfig()
    {
        ConfigData configData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING);
        // View distance, as double to keep human readable and configurable
        if (!framework_->Config()->HasValue(configData, "view distance"))
            framework_->Config()->Set(configData, "view distance", (double)viewDistance);
        // Shadow quality
        if (!framework_->Config()->HasValue(configData, "shadow quality"))
            framework_->Config()->Set(configData, "shadow quality", 2);
        // Texture quality
        if (!framework_->Config()->HasValue(configData, "texture quality"))
            framework_->Config()->Set(configData, "texture quality", 1);
        // Soft shadow
        if (!framework_->Config()->HasValue(configData, "soft shadow"))
            framework_->Config()->Set(configData, "soft shadow", false);
        // Rendering plugin
#ifdef _WINDOWS
        if (!framework_->Config()->HasValue(configData, "rendering plugin"))
            framework_->Config()->Set(configData, "rendering plugin", "Direct3D9 Rendering Subsystem");
#else
        if (!framework_->Config()->HasValue(configData, "rendering plugin"))
            framework_->Config()->Set(configData, "rendering plugin", "OpenGL Rendering Subsystem");
#endif
    }

    void Renderer::Initialize()
    {
        assert(!initialized_);
        if (initialized_)
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
        static Ogre::LogManager *overriddenLogManager = 0;
        overriddenLogManager = new Ogre::LogManager; ///\bug This pointer is never freed. We leak memory here, but cannot free due to Ogre singletons being accessed.
        overriddenLogManager->createLog("", true, false, true);
        Ogre::LogManager::getSingleton().getDefaultLog()->setDebugOutputEnabled(false); // Disable Ogre from outputting to std::cerr by itself.
        Ogre::LogManager::getSingleton().getDefaultLog()->addListener(logListener); // Make all Ogre log output to come to our log listener.
        Ogre::LogManager::getSingleton().getDefaultLog()->setLogDetail(Ogre::LL_NORMAL); // This is probably the default level anyway, but be explicit.

        root_ = OgreRootPtr(new Ogre::Root("", config_filename_, logfilepath));

        //Ogre::LogManager::getSingleton().setLogDetail(Ogre::LL_LOW);

// On Windows, when running with Direct3D in headless mode, preallocating the DefaultHardwareBufferManager singleton will crash.
// On linux, when running with OpenGL in headless mode, *NOT* preallocating the DefaultHardwareBufferManager singleton will crash.
///\todo Perhaps this #ifdef should instead be if(Ogre Render System == OpenGL) (test how Windows + OpenGL behaves)
#ifdef UNIX
        if (framework_->IsHeadless())
        {
            // This has side effects that make Ogre not crash in headless mode (but would crash in headful mode)
            new Ogre::DefaultHardwareBufferManager();
        }
#endif

#include "EnableMemoryLeakCheck.h"

        ConfigData configData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING);
        viewDistance = framework_->Config()->Get(configData, "view distance").toFloat();

        // Load plugins
        QStringList loadedPlugins = LoadPlugins(plugins_filename_);

#ifdef _WINDOWS
        // WIN default to DirectX
        rendersystem_name = framework_->Config()->Get(configData, "rendering plugin").toString().toStdString();
        if (framework_->IsHeadless() && (loadedPlugins.contains("RenderSystem_NULL", Qt::CaseInsensitive) || loadedPlugins.contains("RenderSystem_NULL_d", Qt::CaseInsensitive)))
            rendersystem_name = "NULL Rendering Subsystem";
#else
        // X11/MAC default to OpenGL
        rendersystem_name = framework_->Config()->Get(configData, "rendering plugin").toString().toStdString();
#endif

        textureQuality = (Renderer::TextureQualitySetting)framework_->Config()->Get(configData, "texture quality").toInt();

        // Ask Ogre if rendering system is available
        rendersystem = root_->getRenderSystemByName(rendersystem_name);

#ifdef _WINDOWS
        // If windows did not have DirectX fallback to OpenGL
        if (!rendersystem)
            rendersystem = root_->getRenderSystemByName("OpenGL Rendering Subsystem");
#endif
        if (!rendersystem)
            throw Exception("Could not find Ogre rendersystem.");

        // Report rendering plugin to log so user can check what actually got loaded
        LogInfo("Renderer: Using " + rendersystem->getName());

        // Allow PSSM mode shadows only on DirectX
        // On OpenGL (arbvp & arbfp) it runs out of vertex shader outputs
        shadowQuality = (Renderer::ShadowQualitySetting)framework_->Config()->Get(configData, "shadow quality").toInt();
        if ((shadowQuality == Shadows_High) && (rendersystem->getName() != "Direct3D9 Rendering Subsystem"))
            shadowQuality = Shadows_Low;

        // This is needed for QWebView to not lock up!!!
        Ogre::ConfigOptionMap& map = rendersystem->getConfigOptions();
        if (map.find("Floating-point mode") != map.end())
            rendersystem->setConfigOption("Floating-point mode", "Consistent");

        // Set the found rendering system
        root_->setRenderSystem(rendersystem);

        // Initialise but don't create rendering window yet
        root_->initialise(false);

        if (!framework_->IsHeadless())
        {
            try
            {
                int width = framework_->Ui()->GraphicsView()->viewport()->size().width();
                int height = framework_->Ui()->GraphicsView()->viewport()->size().height();
                int window_left = 0;
                int window_top = 0;
                renderWindow = new RenderWindow();
                bool fullscreen = false;

                // On some systems, the Ogre rendering output is overdrawn by the Windows desktop compositing manager, but the actual cause of this
                // is uncertain.
                // As a workaround, it is possible to have Ogre output directly on the main window HWND of the ui chain. On other systems, this gives
                // graphical issues, so it cannot be used as a permanent mechanism. Therefore this workaround is enabled only as a command-line switch.
                if (framework_->HasCommandLineParameter("--ogrecapturetopwindow"))
                    renderWindow->CreateRenderWindow(framework_->Ui()->MainWindow(), window_title_.c_str(), width, height, window_left, window_top, false);
                else if (framework_->HasCommandLineParameter("--nouicompositing"))
                    renderWindow->CreateRenderWindow(0, window_title_.c_str(), width, height, window_left, window_top, false);
                else // Normally, we want to render Ogre onto the UiGraphicsview viewport window.
                    renderWindow->CreateRenderWindow(framework_->Ui()->GraphicsView()->viewport(), window_title_.c_str(), width, height, window_left, window_top, false);

                connect(framework_->Ui()->GraphicsView(), SIGNAL(WindowResized(int, int)), renderWindow, SLOT(Resize(int, int)));
                renderWindow->Resize(framework_->Ui()->GraphicsView()->width(), framework_->Ui()->GraphicsView()->height());

                if (fullscreen)
                    framework_->Ui()->MainWindow()->showFullScreen();
                else
                    framework_->Ui()->MainWindow()->show();
            }
            catch(Ogre::Exception &/*e*/)
            {
                LogError("Could not create ogre rendering window!");
                throw;
            }

            LogInfo("Renderer: Loading Ogre resources");
            SetupResources();

            /// Create the default scene manager, which is used for nothing but rendering emptiness in case we have no framework scenes
            defaultScene_ = root_->createSceneManager(Ogre::ST_GENERIC, "DefaultEmptyScene");
            dummyDefaultCamera = defaultScene_->createCamera("DefaultCamera");
        
            mainViewport = renderWindow->OgreRenderWindow()->addViewport(dummyDefaultCamera);
            compositionHandler->SetViewport(mainViewport);
        }

        initialized_ = true;
    }

    void Renderer::SetFullScreen(bool value)
    {
        // In headless mode, we can safely ignore Fullscreen mode requests.
        if (framework_->IsHeadless())
            return;

        if (value)
            framework_->Ui()->MainWindow()->showFullScreen();
        else
            framework_->Ui()->MainWindow()->showNormal();
    }

    bool Renderer::IsFullScreen() const
    {
        if (!framework_->IsHeadless())
            return framework_->Ui()->MainWindow()->isFullScreen();
        else
            return false;
    }

    void Renderer::SetShadowQuality(ShadowQualitySetting quality)
    {
        // We cannot effect the new setting immediately, so save only to config
        framework_->Config()->Set(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "shadow quality", (int)quality);
    }

    void Renderer::SetTextureQuality(TextureQualitySetting quality)
    {
        // We cannot effect the new setting immediately, so save only to config
        framework_->Config()->Set(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "texture quality", (int)quality);
    }

    QStringList Renderer::LoadPlugins(const std::string& plugin_filename)
    {
        QStringList loadedPlugins;

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
                root_->loadPlugin(plugin_dir + plugins[i]);
                loadedPlugins.append(QString::fromStdString(plugins[i]));
            }
            catch(Ogre::Exception &/*e*/)
            {
                LogError("Plugin " + plugins[i] + " failed to load");
            }
        }

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
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
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
        framework_->Config()->Set(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "view distance", (double)viewDistance);
    }

    void Renderer::DoFullUIRedraw()
    {
        if (framework_->IsHeadless())
            return;

        PROFILE(Renderer_DoFullUIRedraw);

        UiGraphicsView *view = framework_->Ui()->GraphicsView();

        QImage *backBuffer = view->BackBuffer();
        if (!backBuffer)
        {
            LogWarning("Renderer::DoFullUIRedraw: UiGraphicsView does not have a backbuffer initialized!");
            return;
        }

        QSize viewsize(view->viewport()->size());
        QRect viewrect(QPoint(0, 0), viewsize);

        QSize gviewsize(view->size());

        QSize mainwindowSize(framework_->Ui()->MainWindow()->size());
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
            
        if (!initialized_)
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
        if (framework_->IsHeadless())
        {
            for (std::map<Scene*, OgreWorldPtr>::const_iterator i = ogreWorlds_.begin(); i != ogreWorlds_.end(); ++i)
            {
                OgreWorld* world = i->second.get();
                if (world)
                {
                    Ogre::SceneManager* mgr = world->GetSceneManager();
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

        UiGraphicsView *view = framework_->Ui()->GraphicsView();
        assert(view);

#ifdef DIRECTX_ENABLED
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
    //                HRESULT hr = surface->LockRect(&lock, 0, D3DLOCK_DISCARD); // for full UI redraw.
                    RECT lockRect = { dirty.left(), dirty.top(), dirty.right(), dirty.bottom() };
                    HRESULT hr = surface->LockRect(&lock, &lockRect, 0);
                    if (FAILED(hr))
                    {
                        LogError("Renderer::Render: D3D9SURFACE9::LockRect failed!");
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
        if (framework_->Input()->IsKeyDown(Qt::Key_F8))
            renderWindow->OgreOverlay()->hide();
        else
            renderWindow->OgreOverlay()->show();
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
                root_->_fireFrameStarted(evt);
                root_->_updateAllRenderTargets();
                root_->_fireFrameEnded();
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
        SceneMap scenes = framework_->Scene()->Scenes();
        if (scenes.size() > 0)
            return scenes.begin()->second.get();

        return 0;
    }

    void Renderer::SetMainCamera(Entity *mainCameraEntity)
    {
        activeMainCamera = mainCameraEntity ? mainCameraEntity->shared_from_this() : boost::shared_ptr<Entity>();

        Ogre::Camera *newActiveCamera = 0;
        EC_Camera *cameraComponent = mainCameraEntity ? mainCameraEntity->GetComponent<EC_Camera>().get() : 0;
        if (cameraComponent)
            newActiveCamera = cameraComponent->GetCamera();
        else
        {
            activeMainCamera.reset();
            LogWarning("Cannot activate camera \"" + (mainCameraEntity ? mainCameraEntity->Name() : "(null)") + "\": It does not have a EC_Camera component!");
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
        return prefix + "_" + ToString<uint>(object_id_++);
    }

    void Renderer::AddResourceDirectory(const QString &qdirectory)
    {
        std::string directory = qdirectory.toStdString();

        // Check to not add the same directory more than once
        for(uint i = 0; i < added_resource_directories_.size(); ++i)
            if (added_resource_directories_[i] == directory)
                return;

        Ogre::ResourceGroupManager& resgrpmgr = Ogre::ResourceGroupManager::getSingleton();

        std::string groupname = "grp" + ToString<uint>(group_id_++);

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

        added_resource_directories_.push_back(directory);
    }

    void Renderer::OnScriptEngineCreated(QScriptEngine* engine)
    {
        qScriptRegisterQObjectMetaType<EC_Placeable*>(engine);
        qScriptRegisterQObjectMetaType<EC_Camera*>(engine);
    }

}
