// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Application.h"
#include "Renderer.h"
#include "OgreRenderingModule.h"
#include "OgreConversionUtils.h"
#include "OgreWorld.h"
#include "EC_Placeable.h"
#include "EC_Camera.h"
#include "RenderWindow.h"
#include "UiGraphicsView.h"
#include "OgreShadowCameraSetupFocusedPSSM.h"
#include "CompositionHandler.h"
#include "OgreDefaultHardwareBufferManager.h"
#include "SceneManager.h"
#include "CoreException.h"
#include "Entity.h"
#include "SceneAPI.h"
#include "Profiler.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "UiGraphicsView.h"
#include "LoggingFunctions.h"
#include "ConfigAPI.h"

#include <boost/filesystem.hpp>
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

namespace OgreRenderer
{
    Renderer::Renderer(Framework* framework, const std::string& config, const std::string& plugins, const std::string& window_title) :
        initialized_(false),
        framework_(framework),
        buffermanager_(0),
        defaultSceneManager_(0),
        defaultCamera_(0),
        cameraComponent_(0),
        viewport_(0),
        object_id_(0),
        group_id_(0),
        config_filename_(config),
        plugins_filename_(plugins),
        window_title_(window_title),
        renderWindow(0),
        last_width_(0),
        last_height_(0),
        capture_screen_pixel_data_(0),
        resized_dirty_(0),
        view_distance_(500.0f),
        shadowquality_(Shadows_High),
        texturequality_(Texture_Normal),
        c_handler_(new CompositionHandler)
    {
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
        if (defaultSceneManager_)
        {
            defaultSceneManager_->destroyCamera(defaultCamera_);
            defaultCamera_ = 0;
            root_->destroySceneManager(defaultSceneManager_);
            defaultSceneManager_ = 0;
        }
        
        root_.reset();
        SAFE_DELETE(c_handler_);
        SAFE_DELETE(renderWindow);
    }

    void Renderer::PrepareConfig()
    {
        ConfigData configData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING);
        // View distance, as double to keep human readable and configurable
        if (!framework_->Config()->HasValue(configData, "view distance"))
            framework_->Config()->Set(configData, "view distance", (double)view_distance_);
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
        if (initialized_)
            return;

        std::string logfilepath, rendersystem_name;
        Ogre::RenderSystem *rendersystem = 0;

        LogDebug("INITIALIZING OGRE");

        //const boost::program_options::variables_map &options = framework_->ProgramOptions();

        // Create Ogre root with logfile
        QDir logDir(Application::UserDataDirectory());
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

        ConfigData configData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING);
        view_distance_ = framework_->Config()->Get(configData, "view distance").toFloat();

        // Load plugins
        LoadPlugins(plugins_filename_);

#ifdef _WINDOWS
        // WIN default to DirectX
        rendersystem_name = framework_->Config()->Get(configData, "rendering plugin").toString().toStdString();
        
#else
        // X11/MAC default to OpenGL
        rendersystem_name = framework_->Config()->Get(configData, "rendering plugin").toString().toStdString();
#endif

        // Allow PSSM mode shadows only on DirectX
        // On OpenGL (arbvp & arbfp) it runs out of vertex shader outputs
        shadowquality_ = (ShadowQuality)framework_->Config()->Get(configData, "shadow quality").toInt();
        if ((shadowquality_ == Shadows_High) && (rendersystem_name != "Direct3D9 Rendering Subsystem"))
            shadowquality_ = Shadows_Low;

        texturequality_ = (TextureQuality)framework_->Config()->Get(configData, "texture quality").toInt();

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
                renderWindow = new RenderWindow();
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

            /// Create the default scene manager, which is used for nothing but rendering emptiness in case we have no framework scenes
            defaultSceneManager_ = root_->createSceneManager(Ogre::ST_GENERIC, "DefaultEmptyScene");
            defaultCamera_ = defaultSceneManager_->createCamera("DefaultCamera");
        
            viewport_ = renderWindow->OgreRenderWindow()->addViewport(defaultCamera_);
            c_handler_->Initialize(framework_ ,viewport_);
        }

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
        framework_->Config()->Set(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "shadow quality", (int)newquality);
    }

    void Renderer::SetTextureQuality(TextureQuality newquality)
    {
        // We cannot effect the new setting immediately, so save only to config
        framework_->Config()->Set(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "texture quality", (int)newquality);
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
        if (plugin_dir == ".")
            plugin_dir = boost::filesystem::path(plugin_filename).branch_path().string();
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
                if (QDir::isRelativePath(arch_name.c_str()))
                    arch_name = QDir::cleanPath(Application::InstallationDirectory() + arch_name.c_str()).toStdString();

                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(arch_name, type_name, sec_name);
            }
        }

        // Add supershader program definitions directory according to the shadow quality level
        switch (shadowquality_)
        {
        case Shadows_Off:
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(Application::InstallationDirectory().toStdString() + "media/materials/scripts/shadows_off", "FileSystem", "General");
            break;
        case Shadows_High:
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(Application::InstallationDirectory().toStdString() + "media/materials/scripts/shadows_high", "FileSystem", "General");
            break;
        default:
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(Application::InstallationDirectory().toStdString() + "media/materials/scripts/shadows_low", "FileSystem", "General");
            break;
        }
        
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
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

    void Renderer::Update(f64 frametime)
    {
    }
    
    void Renderer::SetViewDistance(float distance)
    {
        view_distance_ = distance;
        // As double to keep human readable and configurable
        framework_->Config()->Set(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "view distance", (double)view_distance_);
    }

    void Renderer::SetActiveCamera(EC_Camera* camera)
    {
        Ogre::Camera* ogreCamera = defaultCamera_;
        
        if ((camera) && (camera->GetCamera()))
        {
            cameraComponent_ = camera;
            ogreCamera = camera->GetCamera();
        }
        else
            cameraComponent_ = 0;
        
        if (viewport_)
            viewport_->setCamera(ogreCamera);
    }

    void Renderer::DoFullUIRedraw()
    {
        if (framework_->IsHeadless())
            return;
            
        PROFILE(Renderer_DoFullUIRedraw);

        UiGraphicsView *view = framework_->Ui()->GraphicsView();

        QImage *backBuffer = view->BackBuffer();
        if (!backBuffer)
            return;

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
        return Raycast(x, y, 0xffffffff);
    }

    RaycastResult* Renderer::Raycast(int x, int y, unsigned layerMask)
    {
        OgreWorldPtr world = GetActiveOgreWorld();
        if (world)
            return world->Raycast(x, y, layerMask);
        else
            return 0;
    }
    
    QList<Entity*> Renderer::FrustumQuery(QRect &viewrect)
    {
        OgreWorldPtr world = GetActiveOgreWorld();
        if (world)
            return world->FrustumQuery(viewrect);
        else
            return QList<Entity*>();
    }

    void Renderer::Render()
    {
        using namespace std;
        
        if ((!initialized_) || (framework_->IsHeadless()))
            return;

        PROFILE(Renderer_Render);

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

        UiGraphicsView *view = framework_->Ui()->GraphicsView();

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

                if ((uint)dirty.right() > desc.Width) dirty.setRight(desc.Width);
                if ((uint)dirty.bottom() > desc.Height) dirty.setBottom(desc.Height);
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
                    assert((uint)lock.Pitch >= desc.Width*4);
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
        if (view->IsViewDirty() || resized_dirty_)
        {
            DoFullUIRedraw();
        }
#endif

        if (resized_dirty_ > 0)
            resized_dirty_--;

        // Clear the RenderableListener(s) of all Ogre scenes
        for (std::map<SceneManager*, OgreWorldPtr>::iterator i = ogreWorlds_.begin(); i != ogreWorlds_.end(); ++i)
            i->second->ClearVisibleEntities();

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
            PROFILE(Renderer_Render_OgreRoot_renderOneFrame);
            if (viewport_->getCamera())
                root_->renderOneFrame();
        } catch(const std::exception &e)
        {
            std::cout << "Ogre::Root::renderOneFrame threw an exception: " << (e.what() ? e.what() : "(null)") << std::endl;
            LogCritical(std::string("Ogre::Root::renderOneFrame threw an exception: ") + (e.what() ? e.what() : "(null)"));
        }

        view->MarkViewUndirty();
    }

    IComponent* Renderer::GetActiveCamera() const
    {
        return cameraComponent_;
    }

    OgreWorldPtr Renderer::GetActiveOgreWorld() const
    {
        if (!cameraComponent_)
            return OgreWorldPtr();
        Entity* entity = cameraComponent_->GetParentEntity();
        if (!entity)
            return OgreWorldPtr();
        SceneManager* scene = entity->GetScene();
        if (scene)
            return scene->GetWorld<OgreWorld>();
        else
            return OgreWorldPtr();
    }
    
    Ogre::Camera* Renderer::GetActiveOgreCamera() const
    {
        if (cameraComponent_)
            return cameraComponent_->GetCamera();
        else
            return 0;
    }

    Ogre::RenderWindow *Renderer::GetCurrentRenderWindow() const
    {
        return renderWindow->OgreRenderWindow();
    }

    std::string Renderer::GetUniqueObjectName(const std::string &prefix)
    {
        return prefix + "_" + ToString<uint>(object_id_++);
    }
/*
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
        ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
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

            cam_entity->AddComponent(framework_->Scene()->CreateComponent<EC_Placeable>());
            cam_entity->AddComponent(framework_->Scene()->CreateComponent<EC_Camera>());
            scene->EmitEntityCreated(cam_entity);
            
            ComponentPtr component_placable = cam_entity->GetComponent(EC_Placeable::TypeNameStatic());
            EC_Camera *ec_camera = cam_entity->GetComponent<EC_Camera>().get();
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
        EC_Camera *ec_camera = texture_rendering_cam_entity_->GetComponent<EC_Camera>().get();
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
*/
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
    
}