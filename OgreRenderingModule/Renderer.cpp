// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Renderer.h"
#include "RendererEvents.h"
#include "ResourceHandler.h"
#include "OgreRenderingModule.h"
#include "EC_OgrePlaceable.h"

#include "SceneEvents.h"
#include "Entity.h"

#include <Ogre.h>

///\todo Bring in the D3D9RenderSystem includes to fix Ogre & Qt fighting over SetCursor.
//#include <OgreD3D9RenderWindow.h>

using namespace Foundation;

namespace OgreRenderer
{
    //! Responds to Ogre's window events and manages the realXtend world render window.
    class EventListener : public Ogre::WindowEventListener
    {
    public:
        EventListener(Renderer* renderer) :
            renderer_(renderer)
        {
        }

        ~EventListener() 
        {
        }
        
        void windowResized(Ogre::RenderWindow* rw)
        {
            if (rw == renderer_->renderwindow_)
            {
                if (renderer_->camera_)
                    renderer_->camera_->setAspectRatio(Ogre::Real(rw->getWidth() / Ogre::Real(rw->getHeight())));

                Events::WindowResized data(rw->getWidth(), rw->getHeight());
                renderer_->framework_->GetEventManager()->SendEvent(renderer_->renderercategory_id_, Events::WINDOW_RESIZED, &data);         
            }
        }
    
        void windowClosed(Ogre::RenderWindow* rw)
        {
            if (rw == renderer_->renderwindow_)
            {
                renderer_->OnWindowClosed();
            }
        }
        
    private:
        Renderer* renderer_;
    };

/////////////////////////////////////////////////////////////

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

        void messageLogged( const std::string& message, Ogre::LogMessageLevel lml, bool maskDebug, const std::string &logName )
        {
            for (ListenerList::iterator listener = listeners_.begin() ;
                  listener != listeners_.end() ; 
                  ++listener)
            {
                (*listener)->LogMessage(message);
            }
        }
    private:
        typedef std::list<Foundation::LogListenerPtr> ListenerList;

        //! list of subscribed listeners
        ListenerList listeners_;
    };

    Renderer::Renderer(Framework* framework, const std::string& config, const std::string& plugins, const std::string& window_title) :
        initialized_(false),
        framework_(framework),
        scenemanager_(NULL),
        camera_(NULL),
        renderwindow_(NULL),
        object_id_(0),
        group_id_(0),
        main_window_handle_(0),
        listener_(EventListenerPtr(new EventListener(this))),
        log_listener_(OgreLogListenerPtr(new LogListener)),
        resource_handler_(ResourceHandlerPtr(new ResourceHandler(framework))),
        config_filename_ (config),
        plugins_filename_ (plugins),
        ray_query_(NULL),
        window_title_(window_title)
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        
        renderercategory_id_ = event_manager->RegisterEventCategory("Renderer");
        event_manager->RegisterEvent(renderercategory_id_, Events::POST_RENDER, "PostRender");
        event_manager->RegisterEvent(renderercategory_id_, Events::WINDOW_CLOSED, "WindowClosed");
        event_manager->RegisterEvent(renderercategory_id_, Events::WINDOW_RESIZED, "WindowResized");
    }
    
    Renderer::~Renderer()
    {
        if (ray_query_)
        {
            if (scenemanager_)
                scenemanager_->destroyQuery(ray_query_);
            else
                OgreRenderingModule::LogWarning("Could not free Ogre::RaySceneQuery: The scene manager to which it belongs is not present anymore!");
        }

        if (renderwindow_)
        {
            unsigned int width, height, depth;
            int left, top;
            renderwindow_->getMetrics(width, height, depth, left, top);
            framework_->GetDefaultConfig().SetSetting("OgreRenderer", "window_width", width);
            framework_->GetDefaultConfig().SetSetting("OgreRenderer", "window_height", height);
            framework_->GetDefaultConfig().SetSetting("OgreRenderer", "window_left", left);
            framework_->GetDefaultConfig().SetSetting("OgreRenderer", "window_top", top);
        }

        if (initialized_)
        {
            Ogre::LogManager::getSingleton().getDefaultLog()->removeListener(log_listener_.get());
            Ogre::WindowEventUtilities::removeWindowEventListener(renderwindow_, listener_.get());
        }
        log_listener_.reset();

        resource_handler_.reset();

        root_.reset();
    }
    
    void Renderer::Initialize()
    {
        if (initialized_)
            return;
            
        std::string logfilepath = framework_->GetPlatform()->GetUserDocumentsDirectory();
        logfilepath += "/Ogre.log";

        root_ = OgreRootPtr(new Ogre::Root("", config_filename_, logfilepath));
        //Setting low logging level can potentially make things like mesh/skeleton loading faster
        //Ogre::LogManager::getSingleton().getDefaultLog()->setLogDetail(Ogre::LL_LOW);
        Ogre::LogManager::getSingleton().getDefaultLog()->addListener(log_listener_.get());
        LoadPlugins(plugins_filename_);
        
#ifdef _WINDOWS
        std::string rendersystem_name = framework_->GetDefaultConfig().DeclareSetting<std::string>("OgreRenderer", "rendersystem", "Direct3D9 Rendering Subsystem");
#else
        std::string rendersystem_name = "OpenGL Rendering Subsystem";
        framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "RenderSystem", rendersystem_name);
#endif
        int width = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_width", 800);
        int height = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_height", 600);
        int window_left = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_left", -1);
        int window_top = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "window_top", -1);
        bool fullscreen = framework_->GetDefaultConfig().DeclareSetting("OgreRenderer", "fullscreen", false);
        
        Ogre::RenderSystem* rendersystem = root_->getRenderSystemByName(rendersystem_name);
#ifdef _WINDOWS
        // OpenGL fallback
        if (!rendersystem)
        {
            rendersystem = root_->getRenderSystemByName("OpenGL Rendering Subsystem");
        }
#endif
        
        if (!rendersystem)
        {
            throw Core::Exception("Could not find Ogre rendersystem.");
        }

        // GTK's pango/cairo/whatever's font rendering doesn't work if the floating point mode is not set to strict.
        // This however creates undefined behavior for D3D (refrast + D3DX), but we aren't using those anyway.
        // Note: GTK is currently not used in the main UI, but don't know what gstreamer requires
        Ogre::ConfigOptionMap& map = rendersystem->getConfigOptions();
        if (map.find("Floating-point mode") != map.end())
            rendersystem->setConfigOption("Floating-point mode", "Consistent");
        
        root_->setRenderSystem(rendersystem);
        root_->initialise(false);

        Ogre::NameValuePairList params;

        /// \todo These could be removed for good once we're sure we don't need to embed Ogre into GTK or Qt *ever*.
//        if (!external_window_parameter_.empty()) 
//            params["externalWindowHandle"] = external_window_parameter_;
//        else
//            params["externalWindowHandle"] = framework_->GetApplicationMainWindowHandle();

        //! \todo -1 is actually valid value for window position but not sure how to properly handle 'invalid' value so leave like this for now. -cm
        if (window_left != -1)
            params["left"] = Core::ToString(window_left);
        if (window_top != -1)
            params["top"] = Core::ToString(window_top);

        try
        {
            renderwindow_ = root_->createRenderWindow(window_title_, width, height, fullscreen, &params);

            ///\todo To disable Ogre setting the default arrow mouse cursor, use the following: Commented out for now
            /// since we don't have Ogre D3D9RenderSystem in Ogre deps. Do something similar for other Ogre Render
            /// Systems as well.
            /*
            Ogre::D3D9RenderWindow *renderWnd = dynamic_cast<Ogre::D3D9RenderWindow*>(renderwindow_);
            if (renderWnd)
            {
                HWND mainWnd = renderWnd->getWindowHandle();
                SetClassLong(mainWnd, GCL_HCURSOR, NULL);
            }
            */
        }
        catch (Ogre::Exception e) {}
        
        if (!renderwindow_)
        {
            throw Core::Exception("Could not create Ogre rendering window");
        }
        renderwindow_->setDeactivateOnFocusChange(false);

        SetupResources();
        SetupScene();

        Ogre::WindowEventUtilities::addWindowEventListener(renderwindow_, listener_.get());
              
        initialized_ = true;
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
        
        for (Core::uint i = 0; i < plugins.size(); ++i)
        {
            try
            {
                root_->loadPlugin(plugin_dir + plugins[i]);
            }
            catch (Ogre::Exception e)
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
        camera_ = scenemanager_->createCamera("Camera");
        camera_->setNearClipDistance(0.1f);
        camera_->setFarClipDistance(2000.f);
        camera_->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);
        camera_->roll(Ogre::Radian(Ogre::Math::HALF_PI));
        Ogre::Viewport* viewport = renderwindow_->addViewport(camera_);
        camera_->setAspectRatio(Ogre::Real(viewport->getActualWidth()) / Ogre::Real(viewport->getActualHeight()));

        ray_query_ = scenemanager_->createRayQuery(Ogre::Ray());
        ray_query_->setSortByDistance(true); 
    }

    size_t Renderer::GetWindowHandle() const
    {
        size_t window_handle = 0;

        if (external_window_parameter_.empty())
        {
            if (renderwindow_)
                renderwindow_->getCustomAttribute("WINDOW", &window_handle);
        } else
        {
            window_handle = main_window_handle_;
        }

        return window_handle;
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
    
    void Renderer::Update(Core::f64 frametime)
    {
        Ogre::WindowEventUtilities::messagePump();
    }
    
    void Renderer::Resize(Core::uint width, Core::uint height)
    {
        if (renderwindow_)
        {
            renderwindow_->resize(width, height);
            renderwindow_->windowMovedOrResized();
            
            if (camera_)
            {
                camera_->setAspectRatio(Ogre::Real(renderwindow_->getWidth() / Ogre::Real(renderwindow_->getHeight())));

                Events::WindowResized data(renderwindow_->getWidth(), renderwindow_->getHeight()); 
                framework_->GetEventManager()->SendEvent(renderercategory_id_, Events::WINDOW_RESIZED, &data);
            }       
        }
    }

    void Renderer::OnWindowClosed()
    {
        framework_->Exit();
        framework_->GetEventManager()->SendEvent(renderercategory_id_, Events::WINDOW_CLOSED, NULL);
    }

    void Renderer::Render()
    {
        if (!initialized_) return;
        
        PROFILE(Renderer_Render);
        root_->_fireFrameStarted();
        
        // Render without swapping buffers first
        Ogre::RenderSystem* renderer = root_->getRenderSystem();
        renderer->_updateAllRenderTargets(false);
        root_->_fireFrameRenderingQueued();
        // Send postrender event, so that custom rendering may be added
        framework_->GetEventManager()->SendEvent(renderercategory_id_, Events::POST_RENDER, NULL);
        // Swap buffers now
        renderer->_swapAllRenderTargetBuffers(renderer->getWaitForVerticalBlank());

        root_->_fireFrameEnded();
    }

    Core::uint GetSubmeshFromIndexRange(Core::uint index, const std::vector<Core::uint>& submeshstartindex)
    {
        for (Core::uint i = 0; i < submeshstartindex.size(); ++i)
        {
            Core::uint start = submeshstartindex[i];
            Core::uint end;
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
    void GetMeshInformation(Ogre::Entity *entity,
                                    std::vector<Ogre::Vector3>& vertices,
                                    std::vector<Ogre::Vector2>& texcoords,
                                    std::vector<Core::uint>& indices,
                                    std::vector<Core::uint>& submeshstartindex,                                  
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
       {
          entity->_updateAnimation();
       }

        submeshstartindex.resize(mesh->getNumSubMeshes());
        
        // Calculate how many vertices and indices we're going to need
        for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
        {
            Ogre::SubMesh* submesh = mesh->getSubMesh( i );

            // We only need to add the shared vertices once
            if(submesh->useSharedVertices)
            {
                if( !added_shared )
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
        for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
        {
            Ogre::SubMesh* submesh = mesh->getSubMesh(i);

          //----------------------------------------------------------------
          // GET VERTEXDATA
          //----------------------------------------------------------------

            //Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;
          Ogre::VertexData* vertex_data;

          //When there is animation:
          if(useSoftwareBlendingVertices)
             vertex_data = submesh->useSharedVertices ? entity->_getSkelAnimVertexData() : entity->getSubEntity(i)->_getSkelAnimVertexData();
          else
             vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;


            if((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared))
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
                float* pReal;

                for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
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

            bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

            unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
            unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);


            size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

            if ( use32bitindexes )
            {
                for ( size_t k = 0; k < numTris*3; ++k)
                {
                    indices[index_offset++] = pLong[k] + static_cast<Core::uint>(offset);
                }
            }
            else
            {
                for ( size_t k = 0; k < numTris*3; ++k)
                {
                    indices[index_offset++] = static_cast<Core::uint>(pShort[k]) +
                        static_cast<unsigned long>(offset);
                }
            }

            ibuf->unlock();
            current_offset = next_offset;
        }
    } 
    
    Ogre::Vector2 FindUVs(const Ogre::Ray& ray, float distance, const std::vector<Ogre::Vector3>& vertices, const std::vector<Ogre::Vector2>& texcoords, const std::vector<Core::uint> indices, Core::uint foundindex)
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

    Foundation::RaycastResult Renderer::Raycast(int x, int y)
    {
        Foundation::RaycastResult result;
        result.entity_ = 0; 
        if (!initialized_) return result;

        Core::Real screenx = x / (Core::Real)renderwindow_->getWidth();
        Core::Real screeny = y / (Core::Real)renderwindow_->getHeight();

        Ogre::Ray ray = camera_->getCameraToViewportRay(screenx, screeny);
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

            Scene::Entity *entity = NULL;
            try
            {
                entity = Ogre::any_cast<Scene::Entity*>(any);
            } catch (Ogre::InvalidParametersException)
            {
                continue;
            }
            Foundation::ComponentPtr component = entity->GetComponent(EC_OgrePlaceable::NameStatic());
            if (!component)
                continue;
                
            EC_OgrePlaceable *placeable = checked_static_cast<EC_OgrePlaceable*>(component.get());
            int current_priority = placeable->GetSelectPriority();
            if (current_priority > best_priority)
                best_priority = current_priority;
        }

        // Now do the real pass
        Ogre::Real closest_distance = -1.0f;
        int closest_priority = minimum_priority;
        Ogre::Vector2 closest_uv;

        std::vector<Ogre::Vector3> vertices;
        std::vector<Ogre::Vector2> texcoords;
        std::vector<Core::uint> indices;
        std::vector<Core::uint> submeshstartindex;

        for (size_t i = 0; i < results.size(); ++i)
        {
            Ogre::RaySceneQueryResultEntry &entry = results[i];
            // Stop checking if we have found a raycast hit that is closer
            // than all remaining entities, and the priority found is best possible
            if ((closest_distance >= 0.0f) &&
                (closest_distance < entry.distance) && (closest_priority >= best_priority))
            {
                break;
            }

            if (!entry.movable)
                continue;
                
            Ogre::Any any = entry.movable->getUserAny();
            if (any.isEmpty())
                continue;
                
            
            Scene::Entity *entity = NULL;
            try
            {
                entity = Ogre::any_cast<Scene::Entity*>(any);
            } catch (Ogre::InvalidParametersException)
            {
                continue;
            }
                
            int current_priority = minimum_priority;
            {
                Foundation::ComponentPtr component = entity->GetComponent(EC_OgrePlaceable::NameStatic());
                if (component)
                {
                    EC_OgrePlaceable *placeable = checked_static_cast<EC_OgrePlaceable*>(component.get());
                    if (placeable)
                        current_priority = placeable->GetSelectPriority();
//                    if (current_priority < closest_priority)
//                        continue;
                }
            }

            // Mesh entity check: triangle intersection
            if (entry.movable->getMovableType().compare("Entity") == 0)
            {
                Ogre::Entity* ogre_entity = static_cast<Ogre::Entity*>(entry.movable);
                assert(ogre_entity != 0);
            
                // get the mesh information
                GetMeshInformation( ogre_entity, vertices, texcoords, indices, submeshstartindex,
                            ogre_entity->getParentNode()->_getDerivedPosition(),
                            ogre_entity->getParentNode()->_getDerivedOrientation(),
                            ogre_entity->getParentNode()->_getDerivedScale());
        
                // test for hitting individual triangles on the mesh
                for (Core::uint j = 0; j < indices.size(); j += 3)
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
                                result.pos_ = Core::Vector3df(point.x, point.y, point.z);
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
                        result.pos_ = Core::Vector3df(point.x, point.y, point.z);
                        result.submesh_ = 0;
                        result.u_ = 0.0f;
                        result.v_ = 0.0f;
                    }
                }
            }
        }
        
        return result;
    }
    
    std::string Renderer::GetUniqueObjectName()
    {
        return "obj" + Core::ToString<Core::uint>(object_id_++);
    }

    ResourcePtr Renderer::GetResource(const std::string& id, const std::string& type)
    {
        return resource_handler_->GetResource(id, type);
    }
    
    Core::request_tag_t Renderer::RequestResource(const std::string& id, const std::string& type)   
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
			//std::cout << "Took a screenshot!" << std::endl;
		}
	}
	
	void Renderer::AddResourceDirectory(const std::string& directory)
	{       
        // Check to not add the same directory more than once
        for (Core::uint i = 0; i < added_resource_directories_.size(); ++i)
        {
            if (added_resource_directories_[i] == directory)
                return;
        }
        
        Ogre::ResourceGroupManager& resgrpmgr = Ogre::ResourceGroupManager::getSingleton();

        std::string groupname = "grp" + Core::ToString<Core::uint>(group_id_++);
        
        // Check if resource group already exists (should not)        
        bool exists = false;
        Ogre::StringVector groups = resgrpmgr.getResourceGroups();
        for (Core::uint i = 0; i < groups.size(); ++i)
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
}

