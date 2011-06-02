// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreWorld.h"
#include "Renderer.h"
#include "Entity.h"
#include "EC_Camera.h"
#include "EC_Placeable.h"
#include "Scene.h"
#include "CompositionHandler.h"
#include "Profiler.h"
#include "ConfigAPI.h"
#include "FrameAPI.h"
#include "OgreShadowCameraSetupFocusedPSSM.h"

#include <Ogre.h>

OgreWorld::OgreWorld(OgreRenderer::Renderer* renderer, ScenePtr scene) :
    framework_(scene->GetFramework()),
    renderer_(renderer),
    scene_(scene),
    sceneManager_(0),
    rayQuery_(0)
{
    assert(renderer_->IsInitialized());
    
    Ogre::Root* root = Ogre::Root::getSingletonPtr();
    sceneManager_ = root->createSceneManager(Ogre::ST_GENERIC, scene->Name().toStdString());
    if (!framework_->IsHeadless())
    {
        rayQuery_ = sceneManager_->createRayQuery(Ogre::Ray());
        rayQuery_->setSortByDistance(true);

        // If fog is FOG_NONE, force it to some default ineffective settings, because otherwise SuperShader shows just white
        if (sceneManager_->getFogMode() == Ogre::FOG_NONE)
            sceneManager_->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue::White, 0.001f, 2000.0f, 4000.0f);
        
        SetupShadows();
    }
    
    connect(framework_->Frame(), SIGNAL(Updated(float)), this, SLOT(OnUpdated(float)));
}

OgreWorld::~OgreWorld()
{
    if (rayQuery_)
        sceneManager_->destroyQuery(rayQuery_);
    
    Ogre::Root* root = Ogre::Root::getSingletonPtr();
    root->destroySceneManager(sceneManager_);
}

std::string OgreWorld::GetUniqueObjectName(const std::string &prefix)
{
    return renderer_->GetUniqueObjectName(prefix);
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
    PROFILE(OgreWorld_GetMeshInformation);

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

RaycastResult* OgreWorld::Raycast(int x, int y)
{
    return Raycast(x, y, 0xffffffff);
}

RaycastResult* OgreWorld::Raycast(int x, int y, unsigned layerMask)
{
    PROFILE(OgreWorld_Raycast);
    
    result_.entity = 0;
    
    int width = renderer_->GetWindowWidth();
    int height = renderer_->GetWindowHeight();
    if ((!width) || (!height))
        return &result_; // Headless
    Ogre::Camera* camera = VerifyCurrentSceneCamera();
    if (!camera)
        return &result_;
    
    float screenx = x / (float)width;
    float screeny = y / (float)height;

    Ogre::Ray ray = camera->getCameraToViewportRay(screenx, screeny);
    rayQuery_->setRay(ray);
    Ogre::RaySceneQueryResult &results = rayQuery_->execute();

    Ogre::Real closest_distance = -1.0f;
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
    
        if (!entry.movable)
            continue;

        /// \todo Do we want results for invisible entities?
        if (!entry.movable->isVisible())
            continue;
        
        const Ogre::Any& any = entry.movable->getUserAny();
        if (any.isEmpty())
            continue;

        Entity *entity = 0;
        try
        {
            entity = Ogre::any_cast<Entity*>(any);
        }
        catch(Ogre::InvalidParametersException &/*e*/)
        {
            continue;
        }
        
        EC_Placeable* placeable = entity->GetComponent<EC_Placeable>().get();
        if (placeable)
        {
            if (!(placeable->selectionLayer.Get() & layerMask))
                continue;
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
                    if ((closest_distance < 0.0f) || (hit.second < closest_distance))
                    {
                        // this is the closest/best so far, save it
                        closest_distance = hit.second;

                        Ogre::Vector2 uv = FindUVs(ray, hit.second, vertices, texcoords, indices, j); 
                        Ogre::Vector3 point = ray.getPoint(closest_distance);

                        result_.entity = entity;
                        result_.pos = Vector3df(point.x, point.y, point.z);
                        result_.submesh = GetSubmeshFromIndexRange(j, submeshstartindex);
                        result_.u = uv.x;
                        result_.v = uv.y;
                    }
                }
            }
        }
        else
        {
            // Not an entity, fall back to just using the bounding box - ray intersection
            if ((closest_distance < 0.0f) || (entry.distance < closest_distance))
            {
                // this is the closest/best so far, save it
                closest_distance = entry.distance;

                Ogre::Vector3 point = ray.getPoint(closest_distance);

                result_.entity = entity;
                result_.pos = Vector3df(point.x, point.y, point.z);
                result_.submesh = 0;
                result_.u = 0.0f;
                result_.v = 0.0f;
            }
        }
    }

    return &result_;
}

QList<Entity*> OgreWorld::FrustumQuery(QRect &viewrect)
{
    PROFILE(OgreWorld_FrustumQuery);

    QList<Entity*>l;

    int width = renderer_->GetWindowWidth();
    int height = renderer_->GetWindowHeight();
    if ((!width) || (!height))
        return l; // Headless
    Ogre::Camera* camera = VerifyCurrentSceneCamera();
    if (!camera)
        return l;

    float w= (float)width;
    float h= (float)height;
    float left = (float)(viewrect.left()) / w, right = (float)(viewrect.right()) / w;
    float top = (float)(viewrect.top()) / h, bottom = (float)(viewrect.bottom()) / h;
    
    if(left > right) std::swap(left, right);
    if(top > bottom) std::swap(top, bottom);
    // don't do selection box is too small
    if((right - left) * (bottom-top) < 0.0001) return l;
    
    Ogre::PlaneBoundedVolumeList volumes;
    Ogre::PlaneBoundedVolume p = camera->getCameraToViewportBoxVolume(left, top, right, bottom, true);
    volumes.push_back(p);

    Ogre::PlaneBoundedVolumeListSceneQuery *query = sceneManager_->createPlaneBoundedVolumeQuery(volumes);
    assert(query);

    Ogre::SceneQueryResult results = query->execute();
    for(Ogre::SceneQueryResultMovableList::iterator iter = results.movables.begin(); iter != results.movables.end(); ++iter)
    {
        Ogre::MovableObject *m = *iter;
        const Ogre::Any& any = m->getUserAny();
        if (any.isEmpty())
            continue;
        
        Entity *entity = 0;
        try
        {
            entity = Ogre::any_cast<Entity*>(any);
        }
        catch(Ogre::InvalidParametersException &/*e*/)
        {
            continue;
        }
        if(entity)
            l << entity;
    }

    sceneManager_->destroyQuery(query);

    return l;
}

bool OgreWorld::IsEntityVisible(Entity* entity) const
{
    EC_Camera* cameraComponent = VerifyCurrentSceneCameraComponent();
    if (!cameraComponent)
        return false;
    
    return cameraComponent->IsEntityVisible(entity);
}

QList<Entity*> OgreWorld::GetVisibleEntities() const
{
    QList<Entity*> l;
    EC_Camera* cameraComponent = VerifyCurrentSceneCameraComponent();
    if (!cameraComponent)
        return QList<Entity*>();
    
    return cameraComponent->GetVisibleEntities();
}

void OgreWorld::StartViewTracking(Entity* entity)
{
    if (!entity)
        return;

    EntityPtr entityPtr = entity->shared_from_this();
    for (unsigned i = 0; i < visibilityTrackedEntities_.size(); ++i)
    {
        if (visibilityTrackedEntities_[i].lock() == entityPtr)
            return; // Already added
    }
    
    visibilityTrackedEntities_.push_back(entity->shared_from_this());
}

void OgreWorld::StopViewTracking(Entity* entity)
{
    if (!entity)
        return;
    
    EntityPtr entityPtr = entity->shared_from_this();
    for (unsigned i = 0; i < visibilityTrackedEntities_.size(); ++i)
    {
        if (visibilityTrackedEntities_[i].lock() == entityPtr)
        {
            visibilityTrackedEntities_.erase(visibilityTrackedEntities_.begin() + i);
            return;
        }
    }
}

void OgreWorld::OnUpdated(float timeStep)
{
    // Do nothing if visibility not being tracked for any entities
    if (visibilityTrackedEntities_.empty())
    {
        if (!lastVisibleEntities_.empty())
            lastVisibleEntities_.clear();
        if (!visibleEntities_.empty())
            visibleEntities_.clear();
        return;
    }
    
    // Update visible objects from the active camera
    lastVisibleEntities_ = visibleEntities_;
    EC_Camera* activeCamera = VerifyCurrentSceneCameraComponent();
    if (activeCamera)
        visibleEntities_ = activeCamera->GetVisibleEntityIDs();
    else
        visibleEntities_.clear();
    
    for (unsigned i = visibilityTrackedEntities_.size() - 1; i < visibilityTrackedEntities_.size(); --i)
    {
        // Check if the entity has expired; erase from list in that case
        if (visibilityTrackedEntities_[i].expired())
            visibilityTrackedEntities_.erase(visibilityTrackedEntities_.begin() + i);
        else
        {
            Entity* entity = visibilityTrackedEntities_[i].lock().get();
            entity_id_t id = entity->Id();
            
            // Check for change in visibility status
            bool last = lastVisibleEntities_.find(id) != lastVisibleEntities_.end();
            bool now = visibleEntities_.find(id) != visibleEntities_.end();
            
            if ((!last) && (now))
            {
                emit EntityEnterView(entity);
                entity->EmitEnterView(activeCamera);
            }
            else if ((last) && (!now))
            {
                emit EntityLeaveView(entity);
                entity->EmitLeaveView(activeCamera);
            }
        }
    }
}

void OgreWorld::SetupShadows()
{
    Ogre::SceneManager* sceneManager = sceneManager_;
    // Debug mode Ogre might assert due to illegal shadow camera AABB, with empty scene. Disable shadows in debug mode.
#ifdef _DEBUG
        sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
        return;
#else
    OgreRenderer::ShadowQuality quality = renderer_->GetShadowQuality();
    bool using_pssm = (quality == OgreRenderer::Shadows_High);
    bool soft_shadow = framework_->Config()->Get(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "soft shadow").toBool();
    
    //unsigned short shadowTextureSize = settings.value("depthmap_size", "1024").toInt();  */
    float shadowFarDist = 50;
    unsigned short shadowTextureSize = 2048;
    unsigned short shadowTextureCount = 1;
    
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
    
    if (quality == OgreRenderer::Shadows_Off)
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
        splitpoints.push_back(0.1f); // Default nearclip
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
        for(size_t i=0;i<shadowTextureCount;i++)
        {
            OgreRenderer::GaussianListener* gaussianListener = new OgreRenderer::GaussianListener(); 
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

Ogre::Camera* OgreWorld::VerifyCurrentSceneCamera() const
{
    EC_Camera* cameraComponent = VerifyCurrentSceneCameraComponent();
    return cameraComponent ? cameraComponent->GetCamera() : 0;
}

EC_Camera* OgreWorld::VerifyCurrentSceneCameraComponent() const
{
    EC_Camera* cameraComponent = checked_static_cast<EC_Camera*>(renderer_->GetActiveCamera());
    if (!cameraComponent)
        return 0;
    Entity* entity = cameraComponent->ParentEntity();
    if ((!entity) || (entity->ParentScene() != scene_.lock().get()))
        return 0;
    
    return cameraComponent;
}

bool OgreWorld::IsActive() const
{
    return VerifyCurrentSceneCamera() != 0;
}
