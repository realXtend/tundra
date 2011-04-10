// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Renderer.h"
#include "OgreRenderingModule.h"
#include "OgreConversionUtils.h"
#include "EC_Placeable.h"
#include "EC_OgreCamera.h"
#include "EC_OgreMovableTextOverlay.h"
#include "NaaliRenderWindow.h"
#include "NaaliGraphicsView.h"
#include "OgreShadowCameraSetupFocusedPSSM.h"
#include "CompositionHandler.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "SceneManager.h"
#include "ConfigurationManager.h"
#include "EventManager.h"
#include "Platform.h"
#include "CoreException.h"
#include "Entity.h"
#include "SceneAPI.h"

#include "UiAPI.h"
#include "NaaliMainWindow.h"
#include "NaaliGraphicsView.h"

#include <Ogre.h>

#ifdef USE_D3D9_SUBSURFACE_BLIT
#undef SAFE_DELETE
#undef SAFE_DELETE_ARRAY

#include <d3d9.h>
#include <OgreD3D9HardwarePixelBuffer.h>
#include <OgreD3D9RenderWindow.h>
#endif

#include <QApplication>
#include <QDesktopWidget>
#include <QIcon>
#include <QVBoxLayout>
#include <QGraphicsScene>
#include <QCloseEvent>
#include <QSize>
#include <QScrollBar>
#include <QDir>
#include <QUuid>

#ifdef PROFILING
#include "InputAPI.h"
#endif

#include "MemoryLeakCheck.h"

using namespace Foundation;

namespace OgreRenderer
{
    /// Ogre renderable listener to find out visible objects for each frame
    class RenderableListener : public Ogre::RenderQueue::RenderableListener
    {
    public:
        RenderableListener(Renderer* renderer) :
            renderer_(renderer)
        {
        }
        
        ~RenderableListener()
        {
        }
        
        virtual bool renderableQueued(Ogre::Renderable* rend, Ogre::uint8 groupID,
            Ogre::ushort priority, Ogre::Technique** ppTech, Ogre::RenderQueue* pQueue)
        {
            Ogre::Any any = rend->getUserAny();
            if (any.isEmpty())
                return true;
            
            Scene::Entity *entity = 0;
            try
            {
                entity = Ogre::any_cast<Scene::Entity*>(any);
                if (entity)
                    renderer_->visible_entities_.insert(entity->GetId());
            }
            catch(Ogre::InvalidParametersException &/*e*/)
            {
            }
            return true;
        }
        
    private:
        Renderer* renderer_;
    };

    /// Ogre log listener, for passing ogre log messages
    class LogListener : public Ogre::LogListener
    {
    public:
        LogListener() : Ogre::LogListener() {}
        virtual ~LogListener() {}

        /// Subscribe new listener for ogre log messages
        void SubscribeListener(const Foundation::LogListenerPtr &listener)
        {
            listeners_.push_back(listener);
        }

        /// Unsubscribe listener for ogre log messages
        void UnsubscribeListener(const Foundation::LogListenerPtr &listener)
        {
            listeners_.remove(listener);
        }

        void messageLogged(const std::string& message, Ogre::LogMessageLevel lml, bool maskDebug, const std::string &logName)
        {
            for(ListenerList::iterator it = listeners_.begin(); it != listeners_.end(); ++it)
                (*it)->LogMessage(message);
        }

    private:
        typedef std::list<Foundation::LogListenerPtr> ListenerList;

        /// list of subscribed listeners
        ListenerList listeners_;
    };

    Renderer::Renderer(Framework* framework, const std::string& config, const std::string& plugins, const std::string& window_title) :
        initialized_(false),
        framework_(framework),
        scenemanager_(0),
        buffermanager_(0),
        default_camera_(0),
        camera_(0),
        viewport_(0),
        object_id_(0),
        group_id_(0),
        config_filename_(config),
        plugins_filename_(plugins),
        ray_query_(0),
        window_title_(window_title),
        renderWindow(0),
        last_width_(0),
        last_height_(0),
        capture_screen_pixel_data_(0),
        resized_dirty_(0),
        view_distance_(500.0),
        shadowquality_(Shadows_High),
        texturequality_(Texture_Normal),
        c_handler_(new CompositionHandler),
        targetFpsLimit(60.f) // The default FPS to aim at is 60fps.
    {
        timerFrequency = GetCurrentClockFreq();
    }

    Renderer::~Renderer()
    {
        RemoveLogListener();

        if ((scenemanager_) && (scenemanager_->getRenderQueue()))
            scenemanager_->getRenderQueue()->setRenderableListener(0);

        if (ray_query_)
            if (scenemanager_) {
                scenemanager_->destroyQuery(ray_query_);
            } else {
                LogWarning("Could not free Ogre::RaySceneQuery: The scene manager to which it belongs is not present anymore!");
            }

        if (framework_->Ui() && framework_->Ui()->MainWindow())
            framework_->Ui()->MainWindow()->SaveWindowSettingsToFile();

        framework_->GetDefaultConfig().SetSetting("OgreRenderer", "view_distance", view_distance_);

        ///\todo Is compositorInstance->removeLister(listener) needed here?
        foreach(GaussianListener* listener, gaussianListeners_)
            SAFE_DELETE(listener);

        root_.reset();
        SAFE_DELETE(c_handler_);
        SAFE_DELETE(renderWindow);
    }

    void Renderer::RemoveLogListener()
    {
        if (log_listener_)
        {
            Ogre::LogManager::getSingleton().getDefaultLog()->removeListener(log_listener_.get());
            log_listener_.reset();
        }
    }

    void Renderer::DoFrameTimeLimiting()
    {
        if (targetFpsLimit > 1.f)
        {
            tick_t timeNow = GetCurrentClockTime();

            double msecsSpentInFrame = (double)(timeNow - lastPresentTime) * 1000.0 / timerFrequency;
            const double msecsPerFrame = 1000.0 / targetFpsLimit;
            if (msecsSpentInFrame < msecsPerFrame)
            {
                PROFILE(Renderer_DoFrameTimeLimiting);
                while(msecsSpentInFrame >= 0.0 && msecsSpentInFrame < msecsPerFrame)
                {
                    if (msecsSpentInFrame + 1.0 < msecsPerFrame)
                        boost::this_thread::sleep(boost::posix_time::milliseconds(1)); // Sleep in 1msec slices (which on most systems is far from guaranteed to be 1msec, but suits the purpose here)

                    msecsSpentInFrame = (double)(GetCurrentClockTime() - lastPresentTime) / timerFrequency * 1000.0;
                }

                // Busy-wait the rest of the time slice to avoid jittering and to produce smoother updates.
                while(msecsSpentInFrame >= 0 && msecsSpentInFrame < msecsPerFrame)
                    msecsSpentInFrame = (double)(GetCurrentClockTime() - lastPresentTime) / timerFrequency * 1000.0;
            }

            lastPresentTime = GetCurrentClockTime();

//            timeSleptLastFrame = (double)((timeNow - lastPresentTime) * 1000.0 / timerFrequency);
        }
//        else
//            timeSleptLastFrame = 0.0;
    }

    void Renderer::Initialize()
    {
        if (initialized_)
            return;

        std::string logfilepath, rendersystem_name;
        Ogre::RenderSystem *rendersystem = 0;

        // Some pretty printing
        LogDebug("INITIALIZING OGRE");

        const boost::program_options::variables_map &options = framework_->ProgramOptions();
        if (options.count("fpslimit") > 0)
        {
            targetFpsLimit = options["fpslimit"].as<float>();
            if (targetFpsLimit < 1.f)
                targetFpsLimit = 0.f;
        }
        else
#if QT_VERSION < 0x040700 && !defined(Q_WS_MAC)
            // Default FPS limit is 60.
            targetFpsLimit = 60.f;
#else
            // Dues to main window hanging up on Qt 4.7.x when fps limitter is used, that makes using the app impossible. 
            // Default FPS limit is 0 for non-headless run on Qt 4.7.1 untill the bug can be examined.
            // \todo Fix Qt 4.7.x hanging up the main window on focus and move events, so that FPS limitter can be set back to 60 by default.
            if (!framework_->IsHeadless())
                targetFpsLimit = 0.f; 
            else
                targetFpsLimit = 60.f;
#endif

        // Create Ogre root with logfile
        QDir logDir(framework_->GetPlatform()->GetApplicationDataDirectory().c_str());
        if (!logDir.exists("logs"))
            logDir.mkdir("logs");
        logDir.cd("logs");

        logfilepath = logDir.absoluteFilePath("Ogre.log").toStdString();
#include "DisableMemoryLeakCheck.h"
        root_ = OgreRootPtr(new Ogre::Root("", config_filename_, logfilepath));

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

        // Setup Ogre logger (use LL_NORMAL for more prints of init)
        Ogre::LogManager::getSingleton().getDefaultLog()->setLogDetail(Ogre::LL_LOW);
        log_listener_ = OgreLogListenerPtr(new LogListener);
        Ogre::LogManager::getSingleton().getDefaultLog()->addListener(log_listener_.get());

        view_distance_ = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "view_distance", 500.0);

        // Load plugins
        LoadPlugins(plugins_filename_);

#ifdef _WINDOWS
        // WIN default to DirectX
        rendersystem_name = framework_->GetDefaultConfig().DeclareSetting<std::string>(
            "OgreRenderer", "rendersystem", "Direct3D9 Rendering Subsystem");
#else
        // X11/MAC default to OpenGL
        rendersystem_name = "OpenGL Rendering Subsystem";
        framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "RenderSystem", rendersystem_name);
#endif

        // Allow PSSM mode shadows only on DirectX
        // On OpenGL (arbvp & arbfp) it runs out of vertex shader outputs
        shadowquality_ = (ShadowQuality)(framework_->GetDefaultConfig().DeclareSetting<int>(
            "OgreRenderer", "shadow_quality", 2));
        if ((shadowquality_ == Shadows_High) && (rendersystem_name != "Direct3D9 Rendering Subsystem"))
            shadowquality_ = Shadows_Low;

        texturequality_ = (TextureQuality)(framework_->GetDefaultConfig().DeclareSetting<int>(
            "OgreRenderer","texture_quality", 1));

        // Ask Ogre if rendering system is available
        rendersystem = root_->getRenderSystemByName(rendersystem_name);

#ifdef _WINDOWS
        // If windows did not have DirectX fallback to OpenGL
        if (!rendersystem)
            rendersystem = root_->getRenderSystemByName("OpenGL Rendering Subsystem");
#endif
        if (!rendersystem)
            throw Exception("Could not find Ogre rendersystem.");

        // This is needed for QWebView to not lock up!!!
        Ogre::ConfigOptionMap& map = rendersystem->getConfigOptions();
        if (map.find("Floating-point mode") != map.end())
            rendersystem->setConfigOption("Floating-point mode", "Consistent");

        // Set the found rendering system
        root_->setRenderSystem(rendersystem);

        // Initialise but dont create rendering window yet
        root_->initialise(false);

        if (!framework_->IsHeadless())
        {
            try
            {
                int width = framework_->Ui()->GraphicsView()->viewport()->size().width();
                int height = framework_->Ui()->GraphicsView()->viewport()->size().height();
                int window_left = 0;
                int window_top = 0;
                renderWindow = new NaaliRenderWindow();
                bool fullscreen = false;

                renderWindow->CreateRenderWindow(framework_->Ui()->GraphicsView()->viewport(), window_title_.c_str(), width, height, window_left, window_top, false);
                connect(framework_->Ui()->GraphicsView(), SIGNAL(WindowResized(int, int)), renderWindow, SLOT(Resize(int, int)));
                renderWindow->Resize(framework_->Ui()->GraphicsView()->width(), framework_->Ui()->GraphicsView()->height());

                if(fullscreen)
                {
                    framework_->Ui()->MainWindow()->showFullScreen();
                }
                else
                    framework_->Ui()->MainWindow()->show();
            }
            catch(Ogre::Exception &/*e*/)
            {
                LogError("Could not create ogre rendering window!");
                throw;
            }
            LogDebug("Initializing resources, may take a while...");
            SetupResources();
        }

        SetupScene();
        initialized_ = true;
    }

    bool Renderer::IsFullScreen() const
    {
        if (!framework_->IsHeadless())
            return framework_->Ui()->MainWindow()->isFullScreen();
        else
            return false;
    }

    void Renderer::PostInitialize()
    {
    }

    void Renderer::SetFullScreen(bool value)
    {
        // In headless mode, we can safely ignore Fullscreen mode requests.
        if (framework_->IsHeadless())
            return;

        if(value)
            framework_->Ui()->MainWindow()->showFullScreen();
        else
            framework_->Ui()->MainWindow()->showNormal();
    }

    void Renderer::SetShadowQuality(ShadowQuality newquality)
    {
        // We cannot effect the new setting immediately, so save only to config
        framework_->GetDefaultConfig().SetSetting<int>("OgreRenderer", "shadow_quality", (int)newquality);
    }

    void Renderer::SetTextureQuality(TextureQuality newquality)
    {
        // We cannot effect the new setting immediately, so save only to config
        framework_->GetDefaultConfig().SetSetting<int>("OgreRenderer", "texture_quality", (int)newquality);
    }

    void Renderer::LoadPlugins(const std::string& plugin_filename)
    {
        Ogre::ConfigFile file;
        try
        {
            file.load(plugin_filename);
        }
        catch(Ogre::Exception &/*e*/)
        {
            LogError("Could not load Ogre plugins configuration file");
            return;
        }

        Ogre::String plugin_dir = file.getSetting("PluginFolder");
        Ogre::StringVector plugins = file.getMultiSetting("Plugin");
        
        if (plugin_dir.length())
        {
            if ((plugin_dir[plugin_dir.length() - 1] != '\\') && (plugin_dir[plugin_dir.length() - 1] != '/'))
            {
#ifdef _WINDOWS
                plugin_dir += "\\";
#else
                plugin_dir += "/";
#endif
            }
        }
        
        for(uint i = 0; i < plugins.size(); ++i)
        {
            try
            {
                root_->loadPlugin(plugin_dir + plugins[i]);
            }
            catch(Ogre::Exception &/*e*/)
            {
                LogError("Plugin " + plugins[i] + " failed to load");
            }
        }
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
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch_name, type_name, sec_name);
            }
        }

        // Add supershader program definitions directory according to the shadow quality level
        switch (shadowquality_)
        {
        case Shadows_Off:
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./media/materials/scripts/shadows_off", "FileSystem", "General");
            break;
        case Shadows_High:
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./media/materials/scripts/shadows_high", "FileSystem", "General");
            break;
        default:
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./media/materials/scripts/shadows_low", "FileSystem", "General");
            break;
        }
        
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    }

    void Renderer::SetupScene()
    {
        scenemanager_ = root_->createSceneManager(Ogre::ST_GENERIC, "SceneManager");
        if (framework_->IsHeadless())
            return;
        
        default_camera_ = scenemanager_->createCamera("DefaultCamera");
        viewport_ = renderWindow->OgreRenderWindow()->addViewport(default_camera_);

        default_camera_->setNearClipDistance(0.1f);
        default_camera_->setFarClipDistance(2000.f);
        default_camera_->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);
        default_camera_->roll(Ogre::Radian(Ogre::Math::HALF_PI));
        default_camera_->setAspectRatio(Ogre::Real(viewport_->getActualWidth()) / Ogre::Real(viewport_->getActualHeight()));
        default_camera_->setAutoAspectRatio(true);

        camera_ = default_camera_;

        ray_query_ = scenemanager_->createRayQuery(Ogre::Ray());
        ray_query_->setSortByDistance(true); 
        
        renderable_listener_ = RenderableListenerPtr(new RenderableListener(this));
        scenemanager_->getRenderQueue()->setRenderableListener(renderable_listener_.get());

        InitShadows();

        c_handler_->Initialize(framework_ ,viewport_);
    }

    int Renderer::GetWindowWidth() const
    {
        if (renderWindow)
            return renderWindow->OgreRenderWindow()->getWidth();
        return 0;
    }
    int Renderer::GetWindowHeight() const
    {
        if (renderWindow)
            return renderWindow->OgreRenderWindow()->getHeight();
        return 0;
    }

    void Renderer::SubscribeLogListener(const Foundation::LogListenerPtr &listener)
    {
        log_listener_->SubscribeListener(listener);
    }

    void Renderer::UnsubscribeLogListener(const Foundation::LogListenerPtr &listener)
    {
        log_listener_->UnsubscribeListener(listener);
    }
    
    void Renderer::Update(f64 frametime)
    {
        Ogre::WindowEventUtilities::messagePump();
    }
    
    void Renderer::SetCurrentCamera(Ogre::Camera* camera)
    {
        if (!camera)
            camera = default_camera_;

        if (viewport_)
        {
            viewport_->setCamera(camera);
            camera_ = camera;
        }
    }

    void Renderer::DoFullUIRedraw()
    {
        if (framework_->IsHeadless())
            return;
            
        PROFILE(Renderer_Render_QtBlit);

        NaaliGraphicsView *view = framework_->Ui()->GraphicsView();

        QImage *backBuffer = view->BackBuffer();
        if (!backBuffer)
            return;

        QSize viewsize(view->viewport()->size());
        QRect viewrect(QPoint(0, 0), viewsize);

        QSize gviewsize(view->size());

        QSize mainwindowSize(framework_->Ui()->MainWindow()->size());
        QSize renderWindowSize(renderWindow->OgreRenderWindow()->getWidth(), renderWindow->OgreRenderWindow()->getHeight());

        backBuffer->fill(Qt::transparent);

        // Paint ui view into buffer
        QPainter painter(backBuffer);
        view->viewport()->render(&painter, QPoint(0,0), QRegion(viewrect), QWidget::DrawChildren);

        renderWindow->UpdateOverlayImage(*backBuffer);
    }

    void Renderer::Render()
    {
        using namespace std;
        
        if (!initialized_) 
            return;

        if (framework_->IsHeadless())
        {
            // In headless mode, do frame time limiting here to avoid busy-spinning in the main loop and taking up 100% CPU. In headless mode this could
            // also be safely done using Qt timers, which might be a slightly better approach.
            DoFrameTimeLimiting(); 
            return;
        }

        PROFILE(Renderer_Render);

        // If fog is FOG_NONE, force it to some default ineffective settings, because otherwise SuperShader shows just white
        if (scenemanager_->getFogMode() == Ogre::FOG_NONE)
            scenemanager_->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue::White, 0.001f, 2000.0f, 4000.0f);
            
        // If rendering into different size window, dirty the UI view for now & next frame
        if (last_width_ != GetWindowWidth() || last_height_ != GetWindowHeight())
        {
            last_width_ = GetWindowWidth();
            last_height_ = GetWindowHeight();
            resized_dirty_ = 2;
#if 0
            backBuffer = QImage(last_width_, last_height_, QImage::Format_ARGB32);
            backBuffer.fill(Qt::transparent);
#endif
        }

        bool applyFPSLimit = true;
        UNREFERENCED_PARAM(applyFPSLimit);

        NaaliGraphicsView *view = framework_->Ui()->GraphicsView();

#ifdef USE_D3D9_SUBSURFACE_BLIT
        if (view->IsViewDirty() || resized_dirty_)
        {
            PROFILE(Renderer_Render_QtBlit);

            QRectF dirtyRectangle = view->DirtyRectangle();
            if (resized_dirty_ > 0)
                dirtyRectangle = QRectF(0, 0, GetWindowWidth(), GetWindowHeight());

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
#ifdef _DEBUG
                    std::cout << "No D3DSurface!" << std::endl;
#endif
                    return;
                }
                D3DSURFACE_DESC desc;
                HRESULT hr = surface->GetDesc(&desc);
                if (FAILED(hr))
                {
#ifdef _DEBUG
                    std::cout << "surface->GetDesc failed!" << std::endl;
#endif
                    return;
                }

                if (dirty.right() > desc.Width) dirty.setRight(desc.Width);
                if (dirty.bottom() > desc.Height) dirty.setBottom(desc.Height);
                if (dirty.left() > dirty.right()) dirty.setLeft(dirty.right());
                if (dirty.top() > dirty.bottom()) dirty.setTop(dirty.bottom());

                const int copyableHeight = min<int>(dirty.height(), min<int>(view->BackBuffer()->height() - dirty.top(), desc.Height - dirty.top()));
                const int copyableWidthBytes = 4*min<int>(dirty.width(), min<int>(view->BackBuffer()->width() - dirty.left(), desc.Width - dirty.left()));
                if (copyableHeight <= 0 || copyableWidthBytes <= 0)
                {
#ifdef _DEBUG
                    std::cout << "Nothing to blit!" << std::endl;
#endif
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
#ifdef _DEBUG
                        std::cout << "SubRect Lock Failed!" << std::endl;
#endif
                        return;
/*
                        HRESULT hr = surface->LockRect(&lock, 0, 0);
                        if (FAILED(hr))
                        {
#ifdef _DEBUG
                            std::cout << "Surface LockRect Failed!" << std::endl;
#endif
                            return;
                        }
*/
                    }
                    assert(lock.Pitch >= desc.Width*4);
                }
                char *surfacePtr = (char *)lock.pBits;

                char *scanlines = (char*)view->BackBuffer()->bits();
                 assert(scanlines);
                 if (!scanlines)
                     return;

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
                        std::cout << "Unlock Failed!" << std::endl;
                        return;
                    }
                }
            }
        }
#else // Not using the subrectangle blit - just do a full UI blit.
        if (view->IsViewDirty())
        {
            DoFullUIRedraw();
        }
#endif

        if (resized_dirty_ > 0)
            resized_dirty_--;

        // The RenderableListener will fill in visible entities for this frame
        visible_entities_.clear();

#ifdef PROFILING
        // Performance debugging: Toggle the UI overlay visibility based on a debug key.
        // Allows testing whether the GPU is majorly fill rate bound.
        if (framework_->Input()->IsKeyDown(Qt::Key_F8))
            renderWindow->OgreOverlay()->hide();
        else
            renderWindow->OgreOverlay()->show();
#endif

        try
        {
            DoFrameTimeLimiting(); // Note: Performing limiting here is very prone to FPS jitter depending how much time renderOneFrame takes. The proper method
                                   // would be to perform the limiting inside renderOneFrame, but Ogre does not support it.
            root_->renderOneFrame();
        } catch(const std::exception &e)
        {
            std::cout << "Ogre::Root::renderOneFrame threw an exception: " << (e.what() ? e.what() : "(null)") << std::endl;
            LogCritical(std::string("Ogre::Root::renderOneFrame threw an exception: ") + (e.what() ? e.what() : "(null)"));
        }

        view->MarkViewUndirty();
    }

    uint GetSubmeshFromIndexRange(uint index, const std::vector<uint>& submeshstartindex)
    {
        for(uint i = 0; i < submeshstartindex.size(); ++i)
        {
            uint start = submeshstartindex[i];
            uint end;
            if (i < submeshstartindex.size() - 1)
                end = submeshstartindex[i+1];
            else
                end = 0x7fffffff;
            if ((index >= start) && (index < end))
                return i;
        }
        return 0; // should never happen
    }

    // Get the mesh information for the given mesh. Version which supports animation
    // Adapted from http://www.ogre3d.org/wiki/index.php/Raycasting_to_the_polygon_level
    void GetMeshInformation(
        Ogre::Entity *entity,
        std::vector<Ogre::Vector3>& vertices,
        std::vector<Ogre::Vector2>& texcoords,
        std::vector<uint>& indices,
        std::vector<uint>& submeshstartindex,
        const Ogre::Vector3 &position,
        const Ogre::Quaternion &orient,
        const Ogre::Vector3 &scale)
    {
        PROFILE(Renderer_GetMeshInformation);

        bool added_shared = false;
        size_t current_offset = 0;
        size_t shared_offset = 0;
        size_t next_offset = 0;
        size_t index_offset = 0;
        size_t vertex_count = 0;
        size_t index_count = 0;
        Ogre::MeshPtr mesh = entity->getMesh();

        bool useSoftwareBlendingVertices = entity->hasSkeleton();
        if (useSoftwareBlendingVertices)
            entity->_updateAnimation();

        submeshstartindex.resize(mesh->getNumSubMeshes());

        // Calculate how many vertices and indices we're going to need
        for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
        {
            Ogre::SubMesh* submesh = mesh->getSubMesh( i );
            // We only need to add the shared vertices once
            if (submesh->useSharedVertices)
            {
                if (!added_shared)
                {
                    vertex_count += mesh->sharedVertexData->vertexCount;
                    added_shared = true;
                }
            }
            else
            {
                vertex_count += submesh->vertexData->vertexCount;
            }

            // Add the indices
            submeshstartindex[i] = index_count;
            index_count += submesh->indexData->indexCount;
        }

        // Allocate space for the vertices and indices
        vertices.resize(vertex_count);
        texcoords.resize(vertex_count);
        indices.resize(index_count);

        added_shared = false;

        // Run through the submeshes again, adding the data into the arrays
        for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
        {
            Ogre::SubMesh* submesh = mesh->getSubMesh(i);

            // Get vertex data
            //Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;
            Ogre::VertexData* vertex_data;

            //When there is animation:
            if (useSoftwareBlendingVertices)
                vertex_data = submesh->useSharedVertices ? entity->_getSkelAnimVertexData() : entity->getSubEntity(i)->_getSkelAnimVertexData();
            else
                vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

            if ((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared))
            {
                if(submesh->useSharedVertices)
                {
                    added_shared = true;
                    shared_offset = current_offset;
                }

                const Ogre::VertexElement* posElem =
                    vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
                const Ogre::VertexElement *texElem = 
                    vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES);

                Ogre::HardwareVertexBufferSharedPtr vbuf =
                    vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

                unsigned char* vertex =
                    static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

                // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
                //  as second argument. So make it float, to avoid trouble when Ogre::Real will
                //  be comiled/typedefed as double:
                //      Ogre::Real* pReal;
                float* pReal = 0;

                for(size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
                {
                    posElem->baseVertexPointerToElement(vertex, &pReal);

                    Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);

                    vertices[current_offset + j] = (orient * (pt * scale)) + position;
                    if (texElem)
                    {
                        texElem->baseVertexPointerToElement(vertex, &pReal);
                        texcoords[current_offset + j] = Ogre::Vector2(pReal[0], pReal[1]);
                    }
                    else
                        texcoords[current_offset + j] = Ogre::Vector2(0.0f, 0.0f);
                }

                vbuf->unlock();
                next_offset += vertex_data->vertexCount;
            }

            Ogre::IndexData* index_data = submesh->indexData;
            size_t numTris = index_data->indexCount / 3;
            Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

            unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
            unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);
            size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

            bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
            if (use32bitindexes)
                for(size_t k = 0; k < numTris*3; ++k)
                    indices[index_offset++] = pLong[k] + static_cast<uint>(offset);
            else
                for(size_t k = 0; k < numTris*3; ++k)
                    indices[index_offset++] = static_cast<uint>(pShort[k]) + static_cast<unsigned long>(offset);

            ibuf->unlock();
            current_offset = next_offset;
        }
    }

    Ogre::Vector2 FindUVs(
        const Ogre::Ray& ray,
        float distance,
        const std::vector<Ogre::Vector3>& vertices,
        const std::vector<Ogre::Vector2>& texcoords,
        const std::vector<uint> indices, uint foundindex)
    {
        Ogre::Vector3 point = ray.getPoint(distance);

        Ogre::Vector3 t1 = vertices[indices[foundindex]];
        Ogre::Vector3 t2 = vertices[indices[foundindex+1]];
        Ogre::Vector3 t3 = vertices[indices[foundindex+2]];

        Ogre::Vector3 v1 = point - t1;
        Ogre::Vector3 v2 = point - t2;
        Ogre::Vector3 v3 = point - t3;

        float area1 = (v2.crossProduct(v3)).length() / 2.0f;
        float area2 = (v1.crossProduct(v3)).length() / 2.0f;
        float area3 = (v1.crossProduct(v2)).length() / 2.0f;
        float sum_area = area1 + area2 + area3;
        if (sum_area == 0.0)
            return Ogre::Vector2(0.0f, 0.0f);

        Ogre::Vector3 bary(area1 / sum_area, area2 / sum_area, area3 / sum_area);
        Ogre::Vector2 t = texcoords[indices[foundindex]] * bary.x + texcoords[indices[foundindex+1]] * bary.y + texcoords[indices[foundindex+2]] * bary.z;

        return t;
    }

    RaycastResult* Renderer::Raycast(int x, int y)
    {
        static RaycastResult result;
        
        result.entity_ = 0; 
        if (!initialized_)
            return &result;
        if (!renderWindow)
            return &result; // Headless

        float screenx = x / (float)renderWindow->OgreRenderWindow()->getWidth();
        float screeny = y / (float)renderWindow->OgreRenderWindow()->getHeight();

        Ogre::Ray ray = camera_->getCameraToViewportRay(screenx, screeny);
        ray_query_->setRay(ray);
        Ogre::RaySceneQueryResult &results = ray_query_->execute();

        // The minimum priority to use if we're picking an Entity that doesn't have the component that contains priority.
        const int minimum_priority = -1000000;

        int best_priority = minimum_priority;
        // Prepass: get best available priority for breaking early
        for(size_t i = 0; i < results.size(); ++i)
        {
            Ogre::RaySceneQueryResultEntry &entry = results[i];
            if (!entry.movable)
                continue;

            /// \todo Do we want results for invisible entities?
            if (!entry.movable->isVisible())
                continue;
            
            Ogre::Any any = entry.movable->getUserAny();
            if (any.isEmpty())
                continue;
            
            Scene::Entity *entity = 0;
            try
            {
                entity = Ogre::any_cast<Scene::Entity*>(any);
            }
            catch(Ogre::InvalidParametersException &/*e*/)
            {
                continue;
            }

            EC_Placeable *placeable = entity->GetComponent<EC_Placeable>().get();
            if (!placeable)
                continue;

            int current_priority = placeable->GetSelectPriority();
            if (current_priority > best_priority)
                best_priority = current_priority;
        }

        // Now do the real pass
        Ogre::Real closest_distance = -1.0f;
        int closest_priority = minimum_priority;
        Ogre::Vector2 closest_uv;

        static std::vector<Ogre::Vector3> vertices;
        static std::vector<Ogre::Vector2> texcoords;
        static std::vector<uint> indices;
        static std::vector<uint> submeshstartindex;
        vertices.clear();
        texcoords.clear();
        indices.clear();
        submeshstartindex.clear();

        for(size_t i = 0; i < results.size(); ++i)
        {
            Ogre::RaySceneQueryResultEntry &entry = results[i];
            // Stop checking if we have found a raycast hit that is closer
            // than all remaining entities, and the priority found is best possible
            if ((closest_distance >= 0.0f) && (closest_distance < entry.distance) && (closest_priority >= best_priority))
                break;

            if (!entry.movable)
                continue;

            /// \todo Do we want results for invisible entities?
            if (!entry.movable->isVisible())
                continue;
            
            Ogre::Any any = entry.movable->getUserAny();
            if (any.isEmpty())
                continue;

            Scene::Entity *entity = 0;
            try
            {
                entity = Ogre::any_cast<Scene::Entity*>(any);
            }
            catch(Ogre::InvalidParametersException &/*e*/)
            {
                continue;
            }

            int current_priority = minimum_priority;
            {
                EC_Placeable *placeable = entity->GetComponent<EC_Placeable>().get();
                if (placeable)
                {
                    current_priority = placeable->GetSelectPriority();
                    //if (current_priority < closest_priority)
                    //  continue;
                }
            }

            // Mesh entity check: triangle intersection
            if (entry.movable->getMovableType().compare("Entity") == 0)
            {
                Ogre::Entity* ogre_entity = static_cast<Ogre::Entity*>(entry.movable);
                assert(ogre_entity != 0);

                // get the mesh information
                GetMeshInformation(ogre_entity, vertices, texcoords, indices, submeshstartindex,
                    ogre_entity->getParentNode()->_getDerivedPosition(),
                    ogre_entity->getParentNode()->_getDerivedOrientation(),
                    ogre_entity->getParentNode()->_getDerivedScale());

                // test for hitting individual triangles on the mesh
                for(int j = 0; j < ((int)indices.size())-2; j += 3)
                {
                    // check for a hit against this triangle
                    std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ray, vertices[indices[j]],
                        vertices[indices[j+1]], vertices[indices[j+2]], true, false);
                    if (hit.first)
                    {
                        if ((closest_distance < 0.0f) || (hit.second < closest_distance) || (current_priority > closest_priority))
                        {
                            if (current_priority >= closest_priority)
                            {
                                // this is the closest/best so far, save it
                                closest_distance = hit.second;
                                closest_priority = current_priority;

                                Ogre::Vector2 uv = FindUVs(ray, hit.second, vertices, texcoords, indices, j); 
                                Ogre::Vector3 point = ray.getPoint(closest_distance);

                                result.entity_ = entity;
                                result.pos_ = Vector3df(point.x, point.y, point.z);
                                result.submesh_ = GetSubmeshFromIndexRange(j, submeshstartindex);
                                result.u_ = uv.x;
                                result.v_ = uv.y;
                            }
                        }
                    }
                }
            }
            else
            {
                // Not an entity, fall back to just using the bounding box - ray intersection
                if ((closest_distance < 0.0f) || (entry.distance < closest_distance) || (current_priority > closest_priority))
                {
                    if (current_priority >= closest_priority)
                    {
                        // this is the closest/best so far, save it
                        closest_distance = entry.distance;
                        closest_priority = current_priority;

                        Ogre::Vector3 point = ray.getPoint(closest_distance);

                        result.entity_ = entity;
                        result.pos_ = Vector3df(point.x, point.y, point.z);
                        result.submesh_ = 0;
                        result.u_ = 0.0f;
                        result.v_ = 0.0f;
                    }
                }
            }
        }

        return &result;
    }
    
    //qt wrapper / upcoming replacement for the one above
    QList<Scene::Entity*> Renderer::FrustumQuery(QRect &viewrect)
    {
        QList<Scene::Entity*>l;
        float w= (float)renderWindow->OgreRenderWindow()->getWidth();
        float h= (float)renderWindow->OgreRenderWindow()->getHeight();
        float left = (float)(viewrect.left()) / w, right = (float)(viewrect.right()) / w;
        float top = (float)(viewrect.top()) / h, bottom = (float)(viewrect.bottom()) / h;
        
        if(left > right) std::swap<float>(left, right);
        if(top > bottom) std::swap<float>(top, bottom);
        // don't do selection box is too small
        if((right - left) * (bottom-top) < 0.0001) return l;
        
        Ogre::PlaneBoundedVolumeList volumes;
        Ogre::PlaneBoundedVolume p = camera_->getCameraToViewportBoxVolume(left, top, right, bottom, true);
        volumes.push_back(p);

        Ogre::PlaneBoundedVolumeListSceneQuery *query = scenemanager_->createPlaneBoundedVolumeQuery(volumes);
        assert(query);

        Ogre::SceneQueryResult results = query->execute();
        for(Ogre::SceneQueryResultMovableList::iterator iter = results.movables.begin(); iter != results.movables.end(); ++iter)
        {
            Ogre::MovableObject *m = *iter;
            Scene::Entity *entity = 0;
            try
            {
                entity = Ogre::any_cast<Scene::Entity*>(m->getUserAny());
            }
            catch(Ogre::InvalidParametersException &/*e*/)
            {
                continue;
            }
            if(entity)
                l << entity;//->GetId();//std::cout << "Hit MovableObject:" << m << std::endl;
        }

        scenemanager_->destroyQuery(query);

        return l;
    }

    bool Renderer::IsEntityVisible(uint ent_id)
    {
        return (visible_entities_.find(ent_id) != visible_entities_.end());
    }

    Ogre::RenderWindow *Renderer::GetCurrentRenderWindow() const
    {
        return renderWindow->OgreRenderWindow();
    }

    std::string Renderer::GetUniqueObjectName(const std::string &prefix)
    {
        return prefix + "_" + ToString<uint>(object_id_++);
    }

    void Renderer::TakeScreenshot(const std::string& filePath, const std::string& fileName)
    {
        if (renderWindow)
        {
            Ogre::String file = filePath + fileName;
            renderWindow->OgreRenderWindow()->writeContentsToFile(file);
        }
    }

    void Renderer::PrepareImageRendering(int width, int height)
    {
        // Only do this once per connect as we create entitys here
        Scene::ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
        if (scene && image_rendering_texture_name_.empty())
        {
            image_rendering_texture_name_ = "ImageRenderingTexture-" + QUuid::createUuid().toString().toStdString();
            Ogre::TexturePtr image_rendering = Ogre::TextureManager::getSingleton().createManual(
                image_rendering_texture_name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET);
            image_rendering->getBuffer()->getRenderTarget()->setAutoUpdated(false);

            EntityPtr cam_entity = scene->CreateEntity(scene->GetNextFreeIdLocal());
            if (!cam_entity)
                return;

            cam_entity->AddComponent(framework_->GetComponentManager()->CreateComponent(EC_Placeable::TypeNameStatic()));
            cam_entity->AddComponent(framework_->GetComponentManager()->CreateComponent(EC_OgreCamera::TypeNameStatic()));
            scene->EmitEntityCreated(cam_entity);
            
            ComponentPtr component_placable = cam_entity->GetComponent(EC_Placeable::TypeNameStatic());
            EC_OgreCamera *ec_camera = cam_entity->GetComponent<EC_OgreCamera>().get();
            if (!component_placable.get() || !ec_camera)
                return;
            ec_camera->SetPlaceable(component_placable);
            texture_rendering_cam_entity_ = cam_entity;
        }
    }

    void Renderer::ResetImageRendering()
    {
        if (!image_rendering_texture_name_.empty())
        {
            Ogre::TextureManager::getSingleton().remove(image_rendering_texture_name_);
            image_rendering_texture_name_ = "";
        }
    }

    QPixmap Renderer::RenderImage(bool use_main_camera)
    {
        int window_width = renderWindow->OgreRenderWindow()->getWidth();
        int window_height = renderWindow->OgreRenderWindow()->getHeight();
        PrepareImageRendering(window_width, window_height);
        if (!texture_rendering_cam_entity_)
            return QPixmap();

        QImage captured_pixmap(QSize(window_width, window_height), QImage::Format_ARGB32_Premultiplied);
        captured_pixmap.fill(Qt::gray);

        // Get the camera ec
        EC_OgreCamera *ec_camera = texture_rendering_cam_entity_->GetComponent<EC_OgreCamera>().get();
        if (!ec_camera)
            return QPixmap::fromImage(captured_pixmap);

        Ogre::TexturePtr image_rendering = Ogre::TextureManager::getSingleton().getByName(image_rendering_texture_name_);
        if (image_rendering.isNull())
            return QPixmap::fromImage(captured_pixmap);

        // Resize rendering texture if needed
        if ((int)image_rendering->getWidth() != window_width || (int)image_rendering->getHeight() != window_height)
        {
            ResetImageRendering();
            PrepareImageRendering(window_width, window_height);
            image_rendering = Ogre::TextureManager::getSingleton().getByName(image_rendering_texture_name_);
            if (image_rendering.isNull())
                return QPixmap::fromImage(captured_pixmap);
        }

        // Set camera aspect ratio
        if (!use_main_camera)
            ec_camera->GetCamera()->setAspectRatio(Ogre::Real(window_width) / Ogre::Real(window_height));

        // Get rendering texture and update it
        Ogre::RenderTexture *render_texture = image_rendering->getBuffer()->getRenderTarget();
        if (render_texture)
        {
            render_texture->removeAllViewports();
            if (render_texture->getNumViewports() == 0)
            {
                // Use custom camera or main av camera
                Ogre::Viewport *vp = 0;
                if (use_main_camera)
                    vp = render_texture->addViewport(GetCurrentCamera());
                else
                    vp = render_texture->addViewport(ec_camera->GetCamera());
                // Exclude ui/name tag overlays
                vp->setOverlaysEnabled(false);
                // Exclude highlight mesh from rendering
                vp->setVisibilityMask(0x2);
            }
            render_texture->update(false);

            captured_pixmap = CreateQImageFromTexture(render_texture, window_width, window_height);
        }
        QPixmap return_pixmap = QPixmap::fromImage(captured_pixmap).copy(); // Deep copy so we can delete the buffer data
        SAFE_DELETE(capture_screen_pixel_data_);
        return return_pixmap;
    }

    QPixmap Renderer::RenderAvatar(const Vector3df &avatar_position, const Quaternion &avatar_orientation)
    {
        int window_width = renderWindow->OgreRenderWindow()->getWidth();
        int window_height = renderWindow->OgreRenderWindow()->getHeight();
        PrepareImageRendering(window_width, window_height);
        if (!texture_rendering_cam_entity_)
            return QPixmap();

        // Calculate positions
        Vector3df pos = avatar_position;
        pos += (avatar_orientation * Vector3df::UNIT_X * 0.6f);
        pos += (avatar_orientation * Vector3df::NEGATIVE_UNIT_Z * 0.5f);
        Vector3df lookat = avatar_position + avatar_orientation * Vector3df(0,0,-0.4f);

        EC_Placeable *cam_ec_placable = texture_rendering_cam_entity_->GetComponent<EC_Placeable>().get();
        if (!cam_ec_placable)
            return QPixmap();

        cam_ec_placable->SetPosition(pos);
        cam_ec_placable->LookAt(lookat);

        return RenderImage(false);
    }

    QImage Renderer::CreateQImageFromTexture(Ogre::RenderTexture *render_texture, int width, int height)
    {
        SAFE_DELETE(capture_screen_pixel_data_);
        capture_screen_pixel_data_ = new Ogre::uchar[width * height * 4];
        Ogre::Box bounds(0, 0, width, height);
        Ogre::PixelBox pixels = Ogre::PixelBox(bounds, Ogre::PF_A8R8G8B8, (void*)capture_screen_pixel_data_);
        render_texture->copyContentsToMemory(pixels, Ogre::RenderTarget::FB_AUTO);

        QImage image = QImage(capture_screen_pixel_data_, width, height, QImage::Format_ARGB32);
        if (image.isNull())
            LogError("Capturing render texture to a image failed");
        return image;
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

    void Renderer::RepaintUi()
    {
        if (resized_dirty_ < 1)
            resized_dirty_  = 1;
    }

    void Renderer::InitShadows()
    {
        Ogre::SceneManager* sceneManager = scenemanager_;
        // Debug mode Ogre might assert due to illegal shadow camera AABB, with empty scene. Disable shadows in debug mode.
#ifdef _DEBUG
            sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
            return;
#else
        bool using_pssm = (shadowquality_ == Shadows_High);
        bool soft_shadow = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "soft_shadow", false);
        
        //unsigned short shadowTextureSize = settings.value("depthmap_size", "1024").toInt();  */
        float shadowFarDist = 50;
        unsigned short shadowTextureSize = 2048;
        size_t shadowTextureCount = 1;
        
        if(using_pssm)
        {
            shadowTextureSize = 1024;
            shadowTextureCount = 3;
        }
        
        Ogre::ColourValue shadowColor(0.6f, 0.6f, 0.6f);

        // This is the default material to use for shadow buffer rendering pass, overridable in script.
        // Note that we use the same single material (vertex program) for each object, so we're relying on
        // that we use Ogre software skinning. Hardware skinning would require us to do different vertex programs
        // for skinned/nonskinned geometry.
        std::string ogreShadowCasterMaterial = "rex/ShadowCaster";
        
        if (shadowquality_ == Shadows_Off)
        {
            sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
            return;
        }
        
        sceneManager->setShadowColour(shadowColor);
        sceneManager->setShadowTextureCountPerLightType(Ogre::Light::LT_DIRECTIONAL, shadowTextureCount);
        sceneManager->setShadowTextureSettings(shadowTextureSize, shadowTextureCount, Ogre::PF_FLOAT32_R);
        sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
        sceneManager->setShadowTextureCasterMaterial(ogreShadowCasterMaterial.c_str());
        sceneManager->setShadowTextureSelfShadow(true);
        
        Ogre::ShadowCameraSetupPtr shadowCameraSetup;
        
        if(using_pssm)
        {
    #include "DisableMemoryLeakCheck.h"
            OgreShadowCameraSetupFocusedPSSM* pssmSetup = new OgreShadowCameraSetupFocusedPSSM();
    #include "EnableMemoryLeakCheck.h"

            OgreShadowCameraSetupFocusedPSSM::SplitPointList splitpoints;
            splitpoints.push_back(default_camera_->getNearClipDistance());
            //these splitpoints are hardcoded also to the shaders. If you modify these, also change them to shaders.
            splitpoints.push_back(3.5);
            splitpoints.push_back(11);
            splitpoints.push_back(shadowFarDist);
            pssmSetup->setSplitPoints(splitpoints);
            shadowCameraSetup = Ogre::ShadowCameraSetupPtr(pssmSetup);
        }
        else
        {
    #include "DisableMemoryLeakCheck.h"
            Ogre::FocusedShadowCameraSetup* focusedSetup = new Ogre::FocusedShadowCameraSetup();
    #include "EnableMemoryLeakCheck.h"
            shadowCameraSetup = Ogre::ShadowCameraSetupPtr(focusedSetup);
        }
        
        sceneManager->setShadowCameraSetup(shadowCameraSetup);
        sceneManager->setShadowFarDistance(shadowFarDist);
        
        // If set to true, problems with objects that clip into the ground
        sceneManager->setShadowCasterRenderBackFaces(false);
        
        //DEBUG
        /*if(renderer_.expired())
            return;
        Ogre::SceneManager *mngr = renderer_.lock()->GetSceneManager();
        Ogre::TexturePtr shadowTex;
        Ogre::String str("shadowDebug");
        Ogre::Overlay* debugOverlay = Ogre::OverlayManager::getSingleton().getByName(str);
        if(!debugOverlay)
            debugOverlay= Ogre::OverlayManager::getSingleton().create(str);
        for(int i = 0; i<shadowTextureCount;i++)
        {
                shadowTex = mngr->getShadowTexture(i);

                // Set up a debug panel to display the shadow
                Ogre::MaterialPtr debugMat = Ogre::MaterialManager::getSingleton().create(
                    "Ogre/DebugTexture" + Ogre::StringConverter::toString(i), 
                    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
                debugMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
                Ogre::TextureUnitState *t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState(shadowTex->getName());
                t->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
                //t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState("spot_shadow_fade.png");
                //t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
                //t->setColourOperation(LBO_ADD);

                Ogre::OverlayContainer* debugPanel = (Ogre::OverlayContainer*)
                    (Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", "Ogre/DebugTexPanel" + Ogre::StringConverter::toString(i)));
                debugPanel->_setPosition(0.8, i*0.25+ 0.05);
                debugPanel->_setDimensions(0.2, 0.24);
                debugPanel->setMaterialName(debugMat->getName());
                debugOverlay->add2D(debugPanel);
        }
        debugOverlay->show();*/

        if(soft_shadow)
        {
            for(int i=0;i<shadowTextureCount;i++)
            {
                GaussianListener* gaussianListener = new GaussianListener(); 
                Ogre::TexturePtr shadowTex = sceneManager->getShadowTexture(0);
                Ogre::RenderTarget* shadowRtt = shadowTex->getBuffer()->getRenderTarget();
                Ogre::Viewport* vp = shadowRtt->getViewport(0);
                Ogre::CompositorInstance *instance = Ogre::CompositorManager::getSingleton().addCompositor(vp, "Gaussian Blur");
                Ogre::CompositorManager::getSingleton().setCompositorEnabled(vp, "Gaussian Blur", true);
                instance->addListener(gaussianListener);
                gaussianListener->notifyViewportSize(vp->getActualWidth(), vp->getActualHeight());
                gaussianListeners_.push_back(gaussianListener);
            }
        }
#endif
    }
}
