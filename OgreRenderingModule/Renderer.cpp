// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Renderer.h"
#include "RendererEvents.h"
#include "ResourceHandler.h"
#include "OgreRenderingModule.h"
#include "OgreConversionUtils.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreCamera.h"
#include "EC_OgreMovableTextOverlay.h"
#include "QOgreUIView.h"
#include "QOgreWorldView.h"

#include "SceneEvents.h"
#include "SceneManager.h"
#include "ConfigurationManager.h"
#include "EventManager.h"
#include "Platform.h"
#include "CoreException.h"
#include "Entity.h"

#include "EC_HoveringIcon.h"

#include <Ogre.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QIcon>
#include <QVBoxLayout>
#include <QGraphicsScene>
#include <QDebug>

#include "MemoryLeakCheck.h"

///\todo Bring in the D3D9RenderSystem includes to fix Ogre & Qt fighting over SetCursor.
//#include <OgreD3D9RenderWindow.h>

using namespace Foundation;

namespace OgreRenderer
{
    //! Ogre log listener, for passing ogre log messages
    class LogListener : public Ogre::LogListener
    {
    public:
        LogListener() : Ogre::LogListener() {}
        virtual ~LogListener() {}

        //! Subscribe new listener for ogre log messages
        void SubscribeListener(const Foundation::LogListenerPtr &listener)
        {
            listeners_.push_back(listener);
        }

        //! Unsubscribe listener for ogre log messages
        void UnsubscribeListener(const Foundation::LogListenerPtr &listener)
        {
            listeners_.remove(listener);
        }

        void messageLogged(const std::string& message, Ogre::LogMessageLevel lml, bool maskDebug, const std::string &logName)
        {
            for (ListenerList::iterator it = listeners_.begin(); it != listeners_.end(); ++it)
                (*it)->LogMessage(message);
        }

    private:
        typedef std::list<Foundation::LogListenerPtr> ListenerList;

        //! list of subscribed listeners
        ListenerList listeners_;
    };

    Renderer::Renderer(Framework* framework, const std::string& config, const std::string& plugins, const std::string& window_title) :
        initialized_(false),
        framework_(framework),
        scenemanager_(0),
        default_camera_(0),
        camera_(0),
        renderwindow_(0),
        viewport_(0),
        object_id_(0),
        group_id_(0),
        resource_handler_(ResourceHandlerPtr(new ResourceHandler(framework))),
        config_filename_(config),
        plugins_filename_(plugins),
        ray_query_(0),
        window_title_(window_title),
        main_window_(0),
        q_ogre_ui_view_(0),
        resized_dirty_(0),
        view_distance_(500.0)
    {
        InitializeQt();
        InitializeEvents();
    }

    Renderer::~Renderer()
    {
        RemoveLogListener();

        if (ray_query_)
            if (scenemanager_)
                scenemanager_->destroyQuery(ray_query_);
            else
                OgreRenderingModule::LogWarning("Could not free Ogre::RaySceneQuery: The scene manager to which it belongs is not present anymore!");

        if (renderwindow_)
        {
            QDesktopWidget *desktop = QApplication::desktop();
            int desktop_max_width = 0;
            int desktop_max_height = desktop->screenGeometry().height();
            for (int index = 0; index < desktop->screenCount(); index++)
                desktop_max_width += desktop->screenGeometry(index).width();

            int width, height, left, top;
            left = main_window_->geometry().x();
            if (left < 0 || left > desktop_max_width)
                left = 0;
            top = main_window_->geometry().y();
            if (top < 0 || top > desktop_max_height)
                top = 0;
            width = main_window_->geometry().width();
            if (width < 0 || width > desktop_max_width)
                width = 800;
            height = main_window_->geometry().height();
            if (height < 0 || height > desktop_max_height)
                height = 600;

            bool maximized = main_window_->isMaximized();

            // Do not store the maximized geometry
            if (!maximized)
            {
                framework_->GetDefaultConfig().SetSetting("OgreRenderer", "window_width", width);
                framework_->GetDefaultConfig().SetSetting("OgreRenderer", "window_height", height);
                framework_->GetDefaultConfig().SetSetting("OgreRenderer", "window_left", left);
                framework_->GetDefaultConfig().SetSetting("OgreRenderer", "window_top", top);
            }
            framework_->GetDefaultConfig().SetSetting("OgreRenderer", "window_maximized", maximized);
            framework_->GetDefaultConfig().SetSetting("OgreRenderer", "view_distance", view_distance_);
        }

        resource_handler_.reset();
        root_.reset();
        SAFE_DELETE(q_ogre_world_view_);
        /** @note   We cannot delete main window here because it will cause many dangling pointers
         *          in UiModule which is uninitalized after destruction of Renderer. Probably we will 
         *          refactor the UiModule to be authorative of the main window instead of Renderer.
         */
         //SAFE_DELETE(main_window_);
    }

    void Renderer::RemoveLogListener()
    {
        if (log_listener_.get())
        {
            Ogre::LogManager::getSingleton().getDefaultLog()->removeListener(log_listener_.get());
            log_listener_.reset();
        }
    }

    void Renderer::InitializeQt()
    {
        ///\todo Memory leak below, see very end of ~Renderer() for comments.
        main_window_ = new QWidget;
        q_ogre_ui_view_ = new QOgreUIView(main_window_);

        // Lets disable icon for now, put real one here when one is created for Naali
        QPixmap pm(16,16);
        pm.fill(Qt::transparent);
        main_window_->setWindowIcon(QIcon(pm));
        ///\todo Memory leak below, see very end of ~Renderer() for comments.
        main_window_->setLayout(new QVBoxLayout(main_window_));
        main_window_->layout()->setMargin(0);
        main_window_->layout()->addWidget(q_ogre_ui_view_);

        // Ownership of uiview passed to framework
        framework_->SetUIView(std::auto_ptr<QGraphicsView>(q_ogre_ui_view_));
    }

    void Renderer::InitializeEvents()
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        renderercategory_id_ = event_manager->RegisterEventCategory("Renderer");
        event_manager->RegisterEvent(renderercategory_id_, Events::POST_RENDER, "PostRender");
        event_manager->RegisterEvent(renderercategory_id_, Events::WINDOW_CLOSED, "WindowClosed");
        event_manager->RegisterEvent(renderercategory_id_, Events::WINDOW_RESIZED, "WindowResized");
    }

    void Renderer::Initialize()
    {
        if (initialized_)
            return;

        std::string logfilepath, rendersystem_name;
        Ogre::RenderSystem *rendersystem = 0;

        // Some pretty printing
        OgreRenderingModule::LogDebug("INITIALIZING OGRE");

        // Create Ogre root with logfile
        logfilepath = framework_->GetPlatform()->GetUserDocumentsDirectory();
        logfilepath += "/Ogre.log";
#include "DisableMemoryLeakCheck.h"
        root_ = OgreRootPtr(new Ogre::Root("", config_filename_, logfilepath));
#include "EnableMemoryLeakCheck.h"

        // Setup Ogre logger (use LL_NORMAL for more prints of init)
        Ogre::LogManager::getSingleton().getDefaultLog()->setLogDetail(Ogre::LL_LOW);
        log_listener_ = OgreLogListenerPtr(new LogListener);
        Ogre::LogManager::getSingleton().getDefaultLog()->addListener(log_listener_.get());

        // Read naali config
        int width = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_width", 800);
        int height = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_height", 600);
        int window_left = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_left", -1);
        int window_top = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_top", -1);
        bool maximized = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_maximized", false); 
        bool fullscreen = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "fullscreen", false);
        view_distance_ = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "view_distance", 500.0);

        // Be sure that window is not out of boundaries.
        if (window_left < 0)
            window_left = 0;
        if (window_top < 25)
            window_top = 25;

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

        try
        {
            // Setup Qts mainwindow with title and geometry
            main_window_->setWindowTitle(QString(window_title_.c_str()));
            main_window_->setGeometry(window_left, window_top, width, height);
            if (maximized)
                main_window_->showMaximized();
            q_ogre_ui_view_->scene()->setSceneRect(q_ogre_ui_view_->rect());
            main_window_->show();

            // Create rendeing window with QOgreUIView (will pass a Qt winID for rendering
            renderwindow_ = q_ogre_ui_view_->CreateRenderWindow(window_title_, width, height, window_left, window_top, fullscreen);

            // Create QOgreWorldView that controls ogres window and ui overlay
            q_ogre_world_view_ = new QOgreWorldView(renderwindow_);
            q_ogre_ui_view_->SetWorldView(q_ogre_world_view_);
            q_ogre_world_view_->InitializeOverlay(q_ogre_ui_view_->viewport()->width(), q_ogre_ui_view_->viewport()->height());
        }
        catch (Ogre::Exception &/*e*/)
        {
            OgreRenderingModule::LogError("Could not create ogre rendering window!");
        }

        if (renderwindow_)
        {
            OgreRenderingModule::LogDebug("Initializing resources, may take a while...");
            renderwindow_->setDeactivateOnFocusChange(false);
            SetupResources();
            SetupScene();
            initialized_ = true;
        }
        else
            throw Exception("Could not create Ogre rendering window");
    }

    void Renderer::PostInitialize()
    {
        resource_handler_->PostInitialize();
    }

    void Renderer::LoadPlugins(const std::string& plugin_filename)
    {
        Ogre::ConfigFile file;
        try
        {
            file.load(plugin_filename);
        }
        catch (Ogre::Exception e)
        {
            OgreRenderingModule::LogError("Could not load Ogre plugins configuration file");
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
        
        for (uint i = 0; i < plugins.size(); ++i)
        {
            try
            {
                root_->loadPlugin(plugin_dir + plugins[i]);
            }
            catch (Ogre::Exception &/*e*/)
            {
                OgreRenderingModule::LogError("Plugin " + plugins[i] + " failed to load");
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

        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    }

    void Renderer::SetupScene()
    {
        scenemanager_ = root_->createSceneManager(Ogre::ST_GENERIC, "SceneManager");
        default_camera_ = scenemanager_->createCamera("DefaultCamera");
        viewport_ = renderwindow_->addViewport(default_camera_);

        default_camera_->setNearClipDistance(0.1f);
        default_camera_->setFarClipDistance(2000.f);
        default_camera_->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);
        default_camera_->roll(Ogre::Radian(Ogre::Math::HALF_PI));
        default_camera_->setAspectRatio(Ogre::Real(viewport_->getActualWidth()) / Ogre::Real(viewport_->getActualHeight()));
        default_camera_->setAutoAspectRatio(true);

        camera_ = default_camera_;

        ray_query_ = scenemanager_->createRayQuery(Ogre::Ray());
        ray_query_->setSortByDistance(true); 

        c_handler_.Initialize(framework_ ,viewport_);
    }

    int Renderer::GetWindowWidth() const
    {
        int size = 0;
        if (renderwindow_)
            size = renderwindow_->getWidth();
        return size;
    }
    int Renderer::GetWindowHeight() const
    {
        int size = 0;
        if (renderwindow_)
            size = renderwindow_->getHeight();
        return size;
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

    void Renderer::Render()
    {
        if (!initialized_) 
            return;

        PROFILE(Renderer_Render);

        // Update the ui overlay every 20 ms
        if (ui_update_timer_.elapsed() > 20)
        {
            if (last_width_ != GetWindowWidth() || last_height_ != GetWindowHeight())
            {
                last_width_ = GetWindowWidth();
                last_height_ = GetWindowHeight();
                resized_dirty_ = 2;
            }

            if (q_ogre_ui_view_->isDirty() || resized_dirty_)
            {
                PROFILE(Renderer_Render_QtBlit);

                QSize viewsize(q_ogre_ui_view_->viewport()->size());
                QRect viewrect(QPoint(0, 0), viewsize);

                // If window size changed update ui buffer size
                if (last_view_rect_ != viewrect)
                {
                    ui_buffer_ = QImage(viewsize, QImage::Format_ARGB32_Premultiplied);
                    last_view_rect_ = viewrect;
                }

                ui_buffer_.fill(Qt::transparent);

                // Paint ui view into buffer
                QPainter painter(&ui_buffer_);
                q_ogre_ui_view_->viewport()->render(&painter, QPoint(0,0), QRegion(viewrect), QWidget::DrawChildren);

                // Blit ogre view into buffer
                Ogre::Box bounds(0, 0, viewsize.width(), viewsize.height());
                Ogre::PixelBox bufbox(bounds, Ogre::PF_A8R8G8B8, (void *)ui_buffer_.bits());

                q_ogre_world_view_->OverlayUI(bufbox);
                ui_update_timer_.start();

                if (resized_dirty_ > 0) resized_dirty_--;
            }
        }

        // Update Ogre every main loop cycle
        q_ogre_world_view_->RenderOneFrame();
        q_ogre_ui_view_->setDirty(false);
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

    bool Renderer::CheckInfoIconIntersection(const Ogre::Ray &ray)
    {
        QList<EC_HoveringIcon*> visible_icons;

        Scene::ScenePtr current_scene = framework_->GetDefaultWorldScene();
        if (!current_scene.get())
            return false;

        Scene::SceneManager::iterator iter = current_scene->begin();
        Scene::SceneManager::iterator end = current_scene->end();
        while (iter != end)
        {
            Scene::EntityPtr entity = (*iter);
            ++iter;
            if (!entity.get())
                continue;

            EC_HoveringIcon* icon = entity->GetComponent<EC_HoveringIcon>().get();
            if (icon && icon->IsVisible())
            {
                visible_icons.append(icon);
            }
        }
        for(int i=0; i< visible_icons.size();i++)
        {
            EC_HoveringIcon* icon = visible_icons.at(i);
            Ogre::Billboard board = icon->GetBillboard();
            Ogre::Vector3 vec = board.getPosition();
            Ogre::Real billboard_h = icon->GetBillBoardSet().getDefaultHeight();
            Ogre::Real billboard_w = icon->GetBillBoardSet().getDefaultWidth();

            Ogre::Matrix4 mat;
            icon->GetBillBoardSet().getWorldTransforms(&mat);

            Ogre::AxisAlignedBox box(vec.x, vec.y + billboard_w/2, vec.z + billboard_h/2, vec.x , vec.y - billboard_w/2, vec.z - billboard_h/2);
            
            box.transform(mat);
            if(ray.intersects(box).first)
            {
                OgreRenderingModule::LogInfo("HIT!");
            }

        }

        return true;
    }

    Foundation::RaycastResult Renderer::Raycast(int x, int y)
    {
        Foundation::RaycastResult result;
        result.entity_ = 0; 
        if (!initialized_)
            return result;

        Real screenx = x / (Real)renderwindow_->getWidth();
        Real screeny = y / (Real)renderwindow_->getHeight();

        Ogre::Ray ray = camera_->getCameraToViewportRay(screenx, screeny);
        
        CheckInfoIconIntersection(ray);

        ray_query_->setRay(ray);
        Ogre::RaySceneQueryResult &results = ray_query_->execute();

        // The minimum priority to use if we're picking an Entity that doesn't have the component that contains priority.
        const int minimum_priority = -1000000;

        int best_priority = minimum_priority;
        // Prepass: get best available priority for breaking early
        for (size_t i = 0; i < results.size(); ++i)
        {
            Ogre::RaySceneQueryResultEntry &entry = results[i];
            if (!entry.movable)
                continue;

            Ogre::Any any = entry.movable->getUserAny();
            if (any.isEmpty())
                continue;



            Scene::Entity *entity = 0;
            try
            {
                entity = Ogre::any_cast<Scene::Entity*>(any);
            }
            catch (Ogre::InvalidParametersException)
            {
                continue;
            }

            EC_OgrePlaceable *placeable = entity->GetComponent<EC_OgrePlaceable>().get();
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

        for (size_t i = 0; i < results.size(); ++i)
        {
            Ogre::RaySceneQueryResultEntry &entry = results[i];
            // Stop checking if we have found a raycast hit that is closer
            // than all remaining entities, and the priority found is best possible
            if ((closest_distance >= 0.0f) && (closest_distance < entry.distance) && (closest_priority >= best_priority))
                break;

            if (!entry.movable)
                continue;

            Ogre::Any any = entry.movable->getUserAny();
            if (any.isEmpty())
                continue;

            Scene::Entity *entity = 0;
            try
            {
                entity = Ogre::any_cast<Scene::Entity*>(any);
            }
            catch (Ogre::InvalidParametersException)
            {
                continue;
            }

            int current_priority = minimum_priority;
            {
                EC_OgrePlaceable *placeable = entity->GetComponent<EC_OgrePlaceable>().get();
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
                for (int j = 0; j < ((int)indices.size())-2; j += 3)
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

        return result;
    }

  /* was the first non-qt version
    Foundation::RaycastResult Renderer::FrustumQuery(int left, int top, int right, int bottom)
    {
        Foundation::RaycastResult result;
        result.entity_ = 0; 
        if (!initialized_)
            return result;
     } */

    //qt wrapper / upcoming replacement for the one above
    QVariantList Renderer::FrustumQuery(QRect &viewrect)
    {
        QVariantList l;
        l << 1;
        l << 2;
        l << 3;
        return l;
    }

    std::string Renderer::GetUniqueObjectName()
    {
        return "obj" + ToString<uint>(object_id_++);
    }

    ResourcePtr Renderer::GetResource(const std::string& id, const std::string& type)
    {
        return resource_handler_->GetResource(id, type);
    }

    request_tag_t Renderer::RequestResource(const std::string& id, const std::string& type)
    {
        return resource_handler_->RequestResource(id, type);
    }

    void Renderer::RemoveResource(const std::string& id, const std::string& type)
    {
        return resource_handler_->RemoveResource(id, type);
    }

    void Renderer::TakeScreenshot(const std::string& filePath, const std::string& fileName)
    {
        if (renderwindow_)
        {
            Ogre::String file = filePath + fileName;
            renderwindow_->writeContentsToFile(file);
        }
    }

    void Renderer::CaptureWorldAndAvatarToFile(
        const Vector3Df &avatar_position,
        const Quaternion &avatar_orientation,
        const std::string& worldfile,
        const std::string& avatarfile)
    {
        if (!renderwindow_)
            return;

        if (worldfile.length() == 0 || avatarfile.length() == 0)
        {
            OgreRenderingModule::LogError("Empty filename for world or avatarfile, cannot save.");
            return;
        }

        // Set as active window so we wont take 
        // screenshots of other applications
        if (!main_window_->isActiveWindow())
            main_window_->activateWindow();

        // Hide ui
        q_ogre_world_view_->HideUiOverlay();

        /*** World image ***/
        int window_width = renderwindow_->getWidth();
        int window_height = renderwindow_->getHeight();
        Ogre::Box bounds(0, 0, window_width, window_height);
        Ogre::uchar* pixelData = new Ogre::uchar[window_width * window_height * 4];
        Ogre::PixelBox pixels(bounds, Ogre::PF_A8R8G8B8, pixelData);

        // Just use rendererwindow:s view for this
        renderwindow_->copyContentsToMemory(pixels);

        // Save pixeldata to image
        Ogre::Image screenshot;
        screenshot.loadDynamicImage(pixelData, pixels.getWidth(), pixels.getHeight(), 1, Ogre::PF_A8R8G8B8);
        screenshot.save(worldfile);

        /*** Avatar image ***/
        Ogre::Camera *screenshot_cam = GetSceneManager()->createCamera("ScreenshotCamera");
        Ogre::Vector3 ogre_avatar_pos = ToOgreVector3(avatar_position);
        Ogre::Quaternion ogre_avatar_orientation = ToOgreQuaternion(avatar_orientation);

        // Setup camera
        screenshot_cam->setNearClipDistance(0.1f);
        screenshot_cam->setFarClipDistance(2000.f);
        screenshot_cam->setAspectRatio(Ogre::Real(viewport_->getActualWidth() / Ogre::Real(viewport_->getActualHeight())));
        screenshot_cam->setAutoAspectRatio(true);

        // Calculate positions
        Vector3df pos = avatar_position;
        pos += (avatar_orientation * Vector3df::UNIT_X * 0.6f);
        pos += (avatar_orientation * Vector3df::NEGATIVE_UNIT_Z * 0.5f);
        Vector3df lookat = avatar_position + avatar_orientation * Vector3df(0,0,-0.4f);

        // Create scenenode and attach camera to it
        Ogre::SceneNode *cam_node = GetSceneManager()->createSceneNode("ScreenShotNode");
        cam_node->attachObject(screenshot_cam);

        // Setup camera to look at the avatar
        cam_node->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);
        cam_node->setPosition(Ogre::Vector3(pos.x, pos.y, pos.z));
        cam_node->lookAt(Ogre::Vector3(lookat.x, lookat.y, lookat.z), Ogre::Node::TS_WORLD, Ogre::Vector3::NEGATIVE_UNIT_Z);

        // Render camera view to texture and save to file
        Ogre::TexturePtr avatar_screenshot = Ogre::TextureManager::getSingleton().createManual(
            "ScreenshotTexture", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::TEX_TYPE_2D, window_width, window_height, 0, Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET);

        Ogre::RenderTexture *render_texture = avatar_screenshot->getBuffer()->getRenderTarget();
        Ogre::Viewport *vp = render_texture->addViewport(screenshot_cam);
        render_texture->update();

        pixelData = new Ogre::uchar[window_width * window_height * 4];
        pixels = Ogre::PixelBox(bounds, Ogre::PF_A8R8G8B8, pixelData);
        render_texture->copyContentsToMemory(pixels, Ogre::RenderTarget::FB_AUTO);

        screenshot.loadDynamicImage(pixelData, pixels.getWidth(), pixels.getHeight(), 1, Ogre::PF_A8R8G8B8);
        screenshot.save(avatarfile);

        // Cleanup
        Ogre::TextureManager::getSingleton().remove("ScreenshotTexture");
        GetSceneManager()->destroySceneNode(cam_node);
        GetSceneManager()->destroyCamera(screenshot_cam);

        // Show ui
        q_ogre_world_view_->ShowUiOverlay();
    }

    void Renderer::AddResourceDirectory(const std::string& directory)
    {
        // Check to not add the same directory more than once
        for (uint i = 0; i < added_resource_directories_.size(); ++i)
            if (added_resource_directories_[i] == directory)
                return;

        Ogre::ResourceGroupManager& resgrpmgr = Ogre::ResourceGroupManager::getSingleton();

        std::string groupname = "grp" + ToString<uint>(group_id_++);

        // Check if resource group already exists (should not).
        bool exists = false;
        Ogre::StringVector groups = resgrpmgr.getResourceGroups();
        for (uint i = 0; i < groups.size(); ++i)
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
            catch (...) {}
        }
        
        // Add directory as a resource location, then initialize group
        try
        {
            resgrpmgr.addResourceLocation(directory, "FileSystem", groupname);
            resgrpmgr.initialiseResourceGroup(groupname);
        }
        catch (...) {}

        added_resource_directories_.push_back(directory);
    }

    void Renderer::RepaintUi()
    {
        if (resized_dirty_ < 1)
            resized_dirty_  = 1;
    }

    void Renderer::UpdateKeyBindings(Foundation::KeyBindings *bindings) 
    { 
        if (q_ogre_ui_view_) 
            q_ogre_ui_view_->UpdateKeyBindings(bindings); 
    }
}
