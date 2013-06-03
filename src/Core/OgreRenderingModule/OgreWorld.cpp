// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "CoreException.h"

#define MATH_OGRE_INTEROP

#include "OgreWorld.h"
#include "Renderer.h"
#include "EC_Camera.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "OgreCompositionHandler.h"
#include "OgreShadowCameraSetupFocusedPSSM.h"
#include "OgreBulletCollisionsDebugLines.h"
#include "OgreMaterialAsset.h"
#include "OgreMeshAsset.h"

#include "OgreMeshAsset.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "Profiler.h"
#include "ConfigAPI.h"
#include "FrameAPI.h"
#include "AssetAPI.h"
#include "Transform.h"
#include "Math/float2.h"
#include "Math/float3x4.h"
#include "Geometry/AABB.h"
#include "Geometry/OBB.h"
#include "Geometry/Plane.h"
#include "Geometry/LineSegment.h"
#include "Math/float3.h"
#include "Geometry/Circle.h"
#include "Geometry/Sphere.h"

#include <Ogre.h>

#ifdef ANDROID
#include <OgreRTShaderSystem.h>
#include <OgreShaderGenerator.h>
#endif

#include "MemoryLeakCheck.h"

OgreWorld::OgreWorld(OgreRenderer::Renderer* renderer, ScenePtr scene) :
    framework_(scene->GetFramework()),
    renderer_(renderer),
    scene_(scene),
    sceneManager_(0),
    rayQuery_(0),
    debugLines_(0),
    debugLinesNoDepth_(0),
    drawDebugInstancing_(false)
{
    assert(renderer_->IsInitialized());
    sceneManager_ = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC, scene->Name().toStdString());
#ifdef ANDROID
    Ogre::RTShader::ShaderGenerator* shaderGenerator = renderer_->GetShaderGenerator();
    if (shaderGenerator)
        shaderGenerator->addSceneManager(sceneManager_);
#endif

    if (!framework_->IsHeadless())
    {
        rayQuery_ = sceneManager_->createRayQuery(Ogre::Ray());
        rayQuery_->setQueryTypeMask(Ogre::SceneManager::FX_TYPE_MASK | Ogre::SceneManager::ENTITY_TYPE_MASK);
        rayQuery_->setSortByDistance(true);

        // If fog is FOG_NONE, force it to some default ineffective settings, because otherwise SuperShader shows just white
        if (sceneManager_->getFogMode() == Ogre::FOG_NONE)
            SetDefaultSceneFog();
        // Set a default ambient color that matches the default ambient color of EC_EnvironmentLight, in case there is no environmentlight component.
        sceneManager_->setAmbientLight(DefaultSceneAmbientLightColor());

        SetupShadows();

#include "DisableMemoryLeakCheck.h"
        debugLines_ = new DebugLines("PhysicsDebug");
        debugLinesNoDepth_ = new DebugLines("PhysicsDebugNoDepth");
#include "EnableMemoryLeakCheck.h"
        sceneManager_->getRootSceneNode()->attachObject(debugLines_);
        sceneManager_->getRootSceneNode()->attachObject(debugLinesNoDepth_);
        debugLinesNoDepth_->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
    }

    connect(framework_->Frame(), SIGNAL(Updated(float)), this, SLOT(OnUpdated(float)));
}

OgreWorld::~OgreWorld()
{
    if (!intancingTargets_.isEmpty())
    {
        try
        {
            for (int i=0; i<intancingTargets_.size(); ++i)
            {
                InstancingTarget *intancingTarget = intancingTargets_[i];
                for (int k=0; k<intancingTarget->targets.size(); ++k)
                {
                    MeshInstanceTarget *meshTarget = intancingTarget->targets[k];
                    if (meshTarget && !meshTarget->instances.isEmpty())
                    {
                        // These should all be gone if DestroyInstances() were called by the creators correctly!
                        LogWarning(QString("OgreWorld: %1 instanced entities with submesh %2 are still loaded to the system for %3, destroying them now!")
                            .arg(meshTarget->instances.size()).arg(meshTarget->submesh).arg(intancingTarget->ref));
                        foreach(Ogre::InstancedEntity* instance, meshTarget->instances)
                            sceneManager_->destroyInstancedEntity(instance);
                        meshTarget->instances.clear();
                    }

                    // Destroy instance manager
                    sceneManager_->destroyInstanceManager(meshTarget->manager);
                    meshTarget->manager = 0;
                }
                SAFE_DELETE(intancingTargets_[i]);
            }
        }
        catch (Ogre::Exception &e)
        {
            LogError(QString("OgreWorld: Exception occurred while destroying instance managers: ") + e.what());
        }
        intancingTargets_.clear();
    }

    if (rayQuery_)
        sceneManager_->destroyQuery(rayQuery_);
    
    if (debugLines_)
    {
        sceneManager_->getRootSceneNode()->detachObject(debugLines_);
        SAFE_DELETE(debugLines_);
    }
    if (debugLinesNoDepth_)
    {
        sceneManager_->getRootSceneNode()->detachObject(debugLinesNoDepth_);
        SAFE_DELETE(debugLinesNoDepth_);
    }
    
    // Remove all compositors.
    /// \todo This does not work with a proper multiscene approach
    OgreCompositionHandler* comp = renderer_->CompositionHandler();
    if (comp)
        comp->RemoveAllCompositors();
    
#ifdef ANDROID
    Ogre::RTShader::ShaderGenerator* shaderGenerator = renderer_->GetShaderGenerator();
    if (shaderGenerator)
        shaderGenerator->removeSceneManager(sceneManager_);
#endif
    Ogre::Root::getSingleton().destroySceneManager(sceneManager_);
}

std::string OgreWorld::GenerateUniqueObjectName(const std::string &prefix)
{
    return renderer_->GetUniqueObjectName(prefix);
}

void OgreWorld::FlushDebugGeometry()
{
    if (debugLines_)
        debugLines_->draw();
    if (debugLinesNoDepth_)
        debugLinesNoDepth_->draw();
}

Color OgreWorld::DefaultSceneAmbientLightColor()
{
    return Color(0.364f, 0.364f, 0.364f, 1.f);
}

void OgreWorld::SetDefaultSceneFog()
{
    if (sceneManager_)
    {
        sceneManager_->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue::White, 0.001f, 2000.0f, 4000.0f);
        Renderer()->MainViewport()->setBackgroundColour(Color()); // Color default ctor == black
    }
}

Ogre::InstancedEntity *OgreWorld::CreateInstance(const QString &meshRef, const AssetReferenceList &materials)
{
    return CreateInstance(framework_->Asset()->GetAsset(meshRef), materials);
}

Ogre::InstancedEntity *OgreWorld::CreateInstance(const AssetPtr &meshAsset, const AssetReferenceList &materials)
{
    PROFILE(OgreWorld_CreateInstance);

    if (!sceneManager_ || !meshAsset.get())
        return 0;

    OgreMeshAsset *mesh = dynamic_cast<OgreMeshAsset*>(meshAsset.get());
    if (!mesh || !mesh->IsLoaded())
        return 0;

    int submeshCount = static_cast<int>(mesh->ogreMesh->getNumSubMeshes());

    // Verify materials have been loaded. Use 'AssetLoadError' from Tundras core
    // materials for empty refs (or instancing will crash later to null mat ptr).
    QStringList instanceMaterials;
    for (int i=0; i<submeshCount; ++i)
    {
        QString materialRef = i < materials.Size() ? materials[i].ref.trimmed() : "";
        if (!materialRef.isEmpty())
        {
            AssetPtr materialAsset = framework_->Asset()->GetAsset(materialRef);
            if (!materialAsset.get() || !materialAsset->IsLoaded())
            {
                LogError(QString("OgreWorld::CreateInstance: Cannot create instance for %1, material in index %2 is not loaded!").arg(mesh->Name()).arg(i));
                return 0;
            }
            OgreMaterialAsset *material = dynamic_cast<OgreMaterialAsset*>(materialAsset.get());
            if (!material)
            {
                LogError(QString("OgreWorld::CreateInstance: Cannot create instance for %1, material in index %2 is not type of OgreMaterial type!").arg(mesh->Name()).arg(i));
                return 0;
            }

            // This function will clone the current material if needed for instancing use.
            // If it returns empty string it means something went wrong so bail out.
            materialRef = PrepareInstancingMaterial(material);
            if (materialRef.isEmpty())
                return 0;
        }
        else
            materialRef = "Tundra/Instancing/HWBasic/Empty";

        instanceMaterials << materialRef;
    }

    // Get instance manager and create the new instances, parenting submesh instances to the main instance.
    Ogre::InstancedEntity *mainInstance = 0;
    QList<Ogre::InstancedEntity*> rollback;

    try
    {
        for (int i=0; i<submeshCount; ++i)
        {
            MeshInstanceTarget *target = GetOrCreateInstanceMeshTarget(mesh->OgreMeshName(), i);
            Ogre::InstancedEntity *instance = target->CreateInstance(instanceMaterials[i], mainInstance);

            if (drawDebugInstancing_)
                target->manager->setSetting(Ogre::InstanceManager::SHOW_BOUNDINGBOX, drawDebugInstancing_, instanceMaterials[i].toStdString());

            // Main instance that is being returned.
            if (!mainInstance)
                mainInstance = instance;

            // If exception happens we can destroy this rollback history.
            rollback << instance;
        }
    }
    catch (Exception &e)
    {
        LogError(QString("OgreWorld::CreateInstance: Exception occurred while creating instances: %1 for %2").arg(e.what()).arg(mesh->Name()));
        DestroyInstances(rollback);
        mainInstance = 0;
    }
    catch (Ogre::Exception &e)
    {
        LogError(QString("OgreWorld::CreateInstance: Exception occurred while creating instances: %1 for %2").arg(e.what()).arg(mesh->Name()));
        DestroyInstances(rollback);
        mainInstance = 0;
    }

    return mainInstance;
}

void OgreWorld::DestroyInstances(Ogre::InstancedEntity* instance)
{
    return DestroyInstances(QList<Ogre::InstancedEntity*>() << instance);
}

void OgreWorld::DestroyInstances(const QList<Ogre::InstancedEntity*> instances)
{
    PROFILE(OgreWorld_DestroyInstances);

    /** @todo Optimize this, its gonna be slow when huge amount of instances.
        Though we cant expect EC_Mesh etc. to tell us the meshRef (to slim down these iterations)
        so now we have to brute force it. iTarget and kTarget is my best attempt to reduce extra loops. */

    int iTarget = -1; int kTarget = -1;
    for (int i=0; i<intancingTargets_.size(); ++i)
    {
        InstancingTarget *intancingTarget = intancingTargets_[i];
        for (int k=0; k<intancingTarget->targets.size(); ++k)
        {
            MeshInstanceTarget *meshTarget = intancingTarget->targets[k];
            if (meshTarget->ForgetInstances(instances))
            {
                iTarget = i;
                kTarget = k;
                break;
            }
        }
        if (iTarget > -1)
            break;
    }

    // Internal state has now forgotten these raw ptrs. Destroy the instances.
    foreach(Ogre::InstancedEntity *instance, instances)
    {
        if (instance)
            sceneManager_->destroyInstancedEntity(instance); /// @todo Check if this can throw!
        instance = 0;
    }

    // Now destroy the manager owning the instances if its instance count is empty.
    if (iTarget > -1)
    {
        MeshInstanceTarget *meshTarget = intancingTargets_[iTarget]->targets[kTarget];
        if (meshTarget->instances.isEmpty())
        {
            if (meshTarget->manager)
                sceneManager_->destroyInstanceManager(meshTarget->manager);
            meshTarget->manager = 0;

            SAFE_DELETE(intancingTargets_[iTarget]->targets[kTarget]);
            intancingTargets_[iTarget]->targets.removeAt(kTarget);
        }
    }
}

MeshInstanceTarget *OgreWorld::GetOrCreateInstanceMeshTarget(const QString &meshRef, int submesh)
{
    PROFILE(OgreWorld_GetOrCreateInstanceMeshTarget);

    InstancingTarget *intancingTarget = 0;
    for (int i=0; i<intancingTargets_.size(); ++i)
    {
        InstancingTarget *iTarget = intancingTargets_[i];
        if (iTarget->ref.compare(meshRef, Qt::CaseSensitive) == 0)
        {
            // Target for this mesh found.
            intancingTarget = iTarget;
            for (int k=0; k<iTarget->targets.size(); ++k)
            {
                MeshInstanceTarget *mTarget = iTarget->targets[k];
                if (mTarget->submesh == submesh)
                {
                    // Target for this submesh found.
                    return mTarget;
                }
            }
            break;
        }
        if (intancingTarget)
            break;
    }

    // No target for the mesh ref exists.
    if (!intancingTarget)
        intancingTarget = new InstancingTarget(meshRef);

    // Check any existing mesh targets within this mesh ref if static instancing is enabled.
    bool isStatic = false;
    if (intancingTarget->targets.size() > 0)
    {
        MeshInstanceTarget *existingTarget = intancingTarget->targets.first();
        if (existingTarget && existingTarget->isStatic)
            isStatic = true;
    }

    uint batchSize = MeshInstanceCount(meshRef);

    // Create target. @note createInstanceManager() will throw if manager with name already exists.
    MeshInstanceTarget *meshTarget = new MeshInstanceTarget(submesh, isStatic);
    meshTarget->manager = sceneManager_->createInstanceManager(QString("InstanceManager_%1_%2").arg(meshRef).arg(submesh).toStdString(),
        meshRef.toStdString(), Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
        Ogre::InstanceManager::HWInstancingBasic, static_cast<size_t>(batchSize), Ogre::IM_USEALL, submesh);

    intancingTarget->targets << meshTarget;
    intancingTargets_ << intancingTarget;
    return meshTarget;
}

uint OgreWorld::MeshInstanceCount(const QString &meshRef)
{
    if (scene_.expired())
        return 64;

    uint count = 0;
    Entity::ComponentVector components = scene_.lock()->Components(EC_Mesh::TypeIdStatic());
    for (Entity::ComponentVector::const_iterator iter = components.begin(); iter != components.end(); ++iter)
    {
        EC_Mesh *mesh = dynamic_cast<EC_Mesh*>((*iter).get());
        if (!mesh)
            continue;
        // Check self resolved/sanitated mesh ref. If we were to use mesh->OgreEntity()->getMesh()->getName()
        // we would not get true count as OgreEntity might be null for most things when this is first called!
        QString ref = mesh->meshRef.Get().ref.trimmed();
        if (ref.isEmpty())
            continue;
        ref = AssetAPI::SanitateAssetRef(framework_->Asset()->ResolveAssetRef("", ref));
        if (meshRef.compare(ref, Qt::CaseInsensitive) == 0)
            count++;
    }
    /// @todo Someone who understands ~optimal values of batch instance count better revisit this logic.
    /// This is just an naive implementation that makes sure we don't jam all our instances into a single batch.
    uint batchSize = (count >= 16384 ? 4096 : (count >= 8192 ? 2048 : (count >= 2048 ? 1024 : (count >= 1024 ? 512 : (count >= 512 ? 256 : (count > 0 ? count : 64))))));
    LogDebug(QString("OgreWorld::MeshInstanceCount: Found %1 instances of %2 in the active scene. Returning batch size of %3.").arg(count).arg(meshRef).arg(batchSize));
    return batchSize;
}

QString OgreWorld::PrepareInstancingMaterial(OgreMaterialAsset *material)
{
    static QString InstancingHWBasic_VS = "Tundra/Instancing/HWBasicVS";
    static QString InstancingHWBasic_PS = "Tundra/Instancing/HWBasicPS";

    QString VS = material->VertexShader(0, 0).trimmed();
    QString PS = material->PixelShader(0, 0).trimmed();
    if (VS.isEmpty() || VS != InstancingHWBasic_VS || PS.isEmpty() || PS != InstancingHWBasic_PS)
    {
        // We cannot modify the original material as it might be used in non-instanced meshes too.
        QString cloneRef = material->Name().replace(".material", "_Cloned_InstancingHWBasic.material", Qt::CaseInsensitive);
        AssetPtr clone = framework_->Asset()->GetAsset(cloneRef);
        if (!clone.get())
        {
            // Clone needs to be created
            clone = material->Clone(cloneRef);
            OgreMaterialAsset *clonedMaterial = dynamic_cast<OgreMaterialAsset*>(clone.get());
            if (clonedMaterial)
            {
                bool logVS = false; bool logPS = false;
                if (VS != InstancingHWBasic_VS)
                {
                    if (!clonedMaterial->SetVertexShader(0, 0, InstancingHWBasic_VS))
                    {
                        LogError(QString("OgreWorld::CreateInstance: Failed to clone material '%1' in submesh %2 with instancing shaders!").arg(material->Name()));
                        return "";
                    }
                    else if (!VS.isEmpty())
                        logVS = true;
                }
                if (PS != InstancingHWBasic_PS)
                {
                    if (!clonedMaterial->SetPixelShader(0, 0, InstancingHWBasic_PS))
                    {
                        LogError(QString("OgreWorld::CreateInstance: Failed to clone material '%1' in submesh %2 with instancing shaders!").arg(material->Name()));
                        return "";
                    }
                    else if (!PS.isEmpty())
                        logPS = true;
                }

                // Unify warning prints to keep it clean as possible. Logging these warnings are either way useful 
                // for content authors when we are replacing existing shaders for instancing to work.
                if (logVS && logPS)
                    LogWarning(QString("OgreWorld::CreateInstance: Replaced existing vertex '%1' and fragment program '%2' in instancing material clone of '%3'. ").arg(VS).arg(PS).arg(material->Name()) +
                               QString("Use '%1' and '%2' in your material to get rid of this warning!").arg(InstancingHWBasic_VS).arg(InstancingHWBasic_PS));
                if (logVS && !logPS)
                    LogWarning(QString("OgreWorld::CreateInstance: Replaced existing vertex program '%1' in instancing material clone of '%2'. ").arg(VS).arg(material->Name()) +
                               QString("Use '%1' in your material to get rid of this warning!").arg(InstancingHWBasic_VS));
                if (!logVS && logPS)
                    LogWarning(QString("OgreWorld::CreateInstance: Replaced existing fragment program '%1' in instancing material clone of '%2'. ").arg(PS).arg(material->Name()) +
                               QString("Use '%1' in your material to get rid of this warning!").arg(InstancingHWBasic_PS));

                return clonedMaterial->ogreAssetName;
            }
            else
            {
                LogError(QString("OgreWorld::CreateInstance: Failed to clone material '%1' with instancing shaders!").arg(material->Name()));
                return "";
            }
        }
        else
        {
            OgreMaterialAsset *clonedMaterial = dynamic_cast<OgreMaterialAsset*>(clone.get());
            return (clonedMaterial != 0 ? clonedMaterial->ogreAssetName : "");
        }
    }

    return material->ogreAssetName;
}

RaycastResult* OgreWorld::Raycast(int x, int y)
{
    return Raycast(x, y, 0xffffffff);
}

RaycastResult* OgreWorld::Raycast(int x, int y, unsigned layerMask)
{
    PROFILE(OgreWorld_Raycast);
    
    result_.entity = 0;
    result_.component = 0;
    
    int width = renderer_->WindowWidth();
    int height = renderer_->WindowHeight();
    if (!width || !height)
        return &result_; // Headless
    Ogre::Camera* camera = VerifyCurrentSceneCamera();
    if (!camera)
        return &result_;
    
    float screenx = x / (float)width;
    float screeny = y / (float)height;

    Ogre::Ray ray = camera->getCameraToViewportRay(screenx, screeny);
    rayQuery_->setRay(ray);
    
    return RaycastInternal(layerMask);
}

RaycastResult* OgreWorld::Raycast(const Ray& ray, unsigned layerMask)
{
    result_.entity = 0;
    result_.component = 0;

    if (!rayQuery_)
        return &result_;
    rayQuery_->setRay(Ogre::Ray(ray.pos, ray.dir));
    return RaycastInternal(layerMask);
}

RaycastResult* OgreWorld::RaycastInternal(unsigned layerMask)
{
    result_.entity = 0;
    result_.component = 0;
    
    Ray ray = rayQuery_->getRay();
    
    Ogre::RaySceneQueryResult &results = rayQuery_->execute();
    float closestDistance = -1.0f;
    
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
        IComponent *component = 0;
        try
        {
            component = Ogre::any_cast<IComponent*>(any);
            entity = component ? component->ParentEntity() : 0;
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
        
        // If this MovableObject's bounding box is further away than our current best result, skip the detailed (e.g. triangle-level) test, as this object possibly can't be closer.
        if (closestDistance >= 0.0f && entry.distance > closestDistance)
            continue;

        Ogre::Entity* meshEntity = dynamic_cast<Ogre::Entity*>(entry.movable);
        if (meshEntity)
        {
            RayQueryResult r;
            bool hit;

            if (meshEntity->hasSkeleton())
                hit = EC_Mesh::Raycast(meshEntity, ray, &r.t, &r.submeshIndex, &r.triangleIndex, &r.pos, &r.normal, &r.uv);
            else
            {
                Ogre::SceneNode *node = meshEntity->getParentSceneNode();
                if (!node)
                {
                    LogError("EC_Mesh::Raycast called for a mesh entity that is not attached to a scene node. Returning no result.");
                    return &result_;
                }

                assume(!float3(node->_getDerivedScale()).IsZero());
                float3x4 localToWorld = float3x4::FromTRS(node->_getDerivedPosition(), node->_getDerivedOrientation(), node->_getDerivedScale());
                assume(localToWorld.IsColOrthogonal());
                float3x4 worldToLocal = localToWorld.Inverted();

                EC_Mesh *mesh = entity->GetComponent<EC_Mesh>().get();
                shared_ptr<OgreMeshAsset> ogreMeshAsset = mesh ? mesh->MeshAsset() : shared_ptr<OgreMeshAsset>();
                if (ogreMeshAsset)
                {
                    Ray localRay = worldToLocal * ray;
                    float oldLength = localRay.dir.Normalize();
                    if (oldLength == 0)
                        continue;
                    r = ogreMeshAsset->Raycast(localRay);
                    hit = r.t < std::numeric_limits<float>::infinity();
                    r.pos = localToWorld.MulPos(r.pos);
                    r.normal = localToWorld.MulDir(r.normal);
                    r.t = r.pos.Distance(ray.pos); ///\todo Can optimize out a sqrt.
                }
                else
                {
                    // No mesh asset available, probably hit terrain? EC_Mesh::Raycast still applicable.
                    hit = EC_Mesh::Raycast(meshEntity, ray, &r.t, &r.submeshIndex, &r.triangleIndex, &r.pos, &r.normal, &r.uv);
                }
            }

            if (hit && (closestDistance < 0.0f || r.t < closestDistance))
            {
                closestDistance = r.t;
                result_.entity = entity;
                result_.component = component;
                result_.pos = r.pos;
                result_.normal = r.normal;
                result_.submesh = r.submeshIndex;
                result_.index = r.triangleIndex;
                result_.u = r.uv.x;
                result_.v = r.uv.y;
            }
        }
        else
        {
            Ogre::BillboardSet *bbs = dynamic_cast<Ogre::BillboardSet*>(entry.movable);
            if (bbs)
            {
                float3x4 camWorldTransform = float4x4(renderer_->MainOgreCamera()->getParentSceneNode()->_getFullTransform()).Float3x4Part();

                float3 billboardFrontDir = camWorldTransform.Col(2).Normalized(); // The -direction this camera views in world space. (In Ogre, like common in OpenGL, cameras view towards -Z in their local space).
                float3 cameraUpDir = camWorldTransform.Col(1).Normalized(); // The direction of the up vector of the camera in world space.
                float3 cameraRightDir = camWorldTransform.Col(0).Normalized(); // The direction of the right vector of the camera in world space.

                Ogre::Matrix4 w_;
                bbs->getWorldTransforms(&w_);
                float3x4 world = float4x4(w_).Float3x4Part(); // The world transform of the whole billboard set.

                // Test a precise hit to each individual billboard in turn, and output the index of hit to the closest billboard in the set.
                for(int i = 0; i < bbs->getNumBillboards(); ++i)
                {
                    Ogre::Billboard *b = bbs->getBillboard(i);
                    float3 worldPos = world.MulPos(b->getPosition()); // A point on this billboard in world space. (@todo assuming this is the center point, but Ogre can use others!)
                    Plane billboardPlane(worldPos, billboardFrontDir); // The plane of this billboard in world space.
                    float d;
                    bool success = billboardPlane.Intersects(ray, &d);
                    if (!success || (closestDistance > 0.0f && d >= closestDistance))
                        continue;

                    float3 intersectionPoint = ray.GetPoint(d); // The point where the ray intersects the plane of the billboard.

                    // Compute the 3D world space -> local normalized 2D (x,y) coordinate frame mapping for this billboard.
                    float w = (b->hasOwnDimensions() ? b->getOwnWidth() : bbs->getDefaultWidth()) * world.Col(0).Length();
                    float h = (b->hasOwnDimensions() ? b->getOwnHeight() : bbs->getDefaultHeight()) * world.Col(1).Length();
                    float3x3 m(w*0.5f*cameraRightDir, h*0.5f*cameraUpDir, billboardFrontDir);
                    success = m.InverseColOrthogonal();
                    assume(success);

                    // Compute the 2D coordinates of the ray hit on the billboard plane.
                    const float3 hit = m * (intersectionPoint - worldPos);

                    /* Test code: To visualize the borders of the billboards, do this:
                    float3 tl = worldPos - w*0.5f*cameraRightDir + h*0.5f*cameraUpDir;
                    float3 tr = worldPos + w*0.5f*cameraRightDir + h*0.5f*cameraUpDir;
                    float3 bl = worldPos - w*0.5f*cameraRightDir - h*0.5f*cameraUpDir;
                    float3 br = worldPos + w*0.5f*cameraRightDir - h*0.5f*cameraUpDir;

                    DebugDrawLineSegment(LineSegment(tl, tr), 1,1,0);
                    DebugDrawLineSegment(LineSegment(tr, br), 1,0,0);
                    DebugDrawLineSegment(LineSegment(br, bl), 1,0,0);
                    DebugDrawLineSegment(LineSegment(bl, tl), 1,0,0); */

                    // The visible range of the billboard is normalized to [-1,1] in x & y. See if the hit is inside the billboard.
                    if (hit.x >= -1.f && hit.x <= 1.f && hit.y >= -1.f && hit.y <= 1.f)
                    {
                        closestDistance = d;
                        result_.entity = entity;
                        result_.component = component;
                        result_.pos = intersectionPoint;
                        result_.normal = billboardFrontDir;
                        result_.submesh = i; // Store in the 'submesh' index the index of the individual billboard we hit.
                        result_.index = (unsigned int)-1; // Not applicable for billboards.
                        result_.u = (hit.x + 1.f) * 0.5f;
                        result_.v = (hit.y + 1.f) * 0.5f;
                    }
                }
            }
            else
            {
                // Not a mesh entity, fall back to just using the bounding box - ray intersection
                if (closestDistance < 0.0f || entry.distance < closestDistance)
                {
                    closestDistance = entry.distance;
                    result_.entity = entity;
                    result_.component = component;
                    result_.pos = ray.GetPoint(closestDistance);
                    result_.normal = -ray.dir;
                    result_.submesh = 0;
                    result_.index = 0;
                    result_.u = 0.0f;
                    result_.v = 0.0f;
                }
            }
        }
    }

    return &result_;
}

QList<Entity*> OgreWorld::FrustumQuery(QRect &viewrect) const
{
    PROFILE(OgreWorld_FrustumQuery);

    QList<Entity*>l;

    int width = renderer_->WindowWidth();
    int height = renderer_->WindowHeight();
    if (!width || !height)
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
            IComponent *component = Ogre::any_cast<IComponent*>(any);
            entity = component ? component->ParentEntity() : 0;
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
    if (cameraComponent)
        return cameraComponent->IsEntityVisible(entity);
    return false;
}

QList<Entity*> OgreWorld::VisibleEntities() const
{
    EC_Camera* cameraComponent = VerifyCurrentSceneCameraComponent();
    if (cameraComponent)
        return cameraComponent->VisibleEntities();
    return QList<Entity*>();
}

void OgreWorld::StartViewTracking(Entity* entity)
{
    if (!entity)
    {
        LogError("OgreWorld::StartViewTracking: null entity passed!");
        return;
    }

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
    {
        LogError("OgreWorld::StopViewTracking: null entity passed!");
        return;
    }

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
    PROFILE(OgreWorld_OnUpdated);
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
        visibleEntities_ = activeCamera->VisibleEntityIDs();
    else
        visibleEntities_.clear();
    
    for (size_t i = visibilityTrackedEntities_.size() - 1; i < visibilityTrackedEntities_.size(); --i)
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
            if (!last && now)
            {
                emit EntityEnterView(entity);
                entity->EmitEnterView(activeCamera);
            }
            else if (last && !now)
            {
                emit EntityLeaveView(entity);
                entity->EmitLeaveView(activeCamera);
            }
        }
    }
}

void OgreWorld::SetupShadows()
{
    OgreRenderer::Renderer::ShadowQualitySetting shadowQuality = renderer_->ShadowQuality();
    if (shadowQuality == OgreRenderer::Renderer::Shadows_Off)
    {
        sceneManager_->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
        return;
    }
    
    bool pssmEnabled = (shadowQuality == OgreRenderer::Renderer::Shadows_High);    
    unsigned short shadowTextureFSAA = static_cast<unsigned short>(framework_->Config()->Get(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "shadow texture antialias", 0).toInt());
    unsigned short shadowTextureSize = static_cast<unsigned short>(framework_->Config()->Get(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "shadow texture size", 2048).toInt());
    unsigned short shadowTextureCount = 1;
    float shadowFarDist = 50;

    if (pssmEnabled)
    {
        shadowTextureSize = static_cast<unsigned short>(framework_->Config()->Get(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "shadow texture size pssm", 1024).toInt());
        shadowTextureCount = 3;
    }
    
    LogDebug(QString("[OgreWorld]: Shadow quality       : %1").arg((shadowQuality == OgreRenderer::Renderer::Shadows_Low ? "Low" : "High")));
    LogDebug(QString("[OgreWorld]: Shadow texture size  : %1x%2").arg(shadowTextureSize).arg(shadowTextureSize));
    LogDebug(QString("[OgreWorld]: Shadow texture count : %1").arg(shadowTextureCount));
    LogDebug(QString("[OgreWorld]: Shadow texture FSAA  : %1").arg(shadowTextureFSAA));

    /** "rex/ShadowCaster" is the default material to use for shadow buffer rendering pass, can be overridden in user materials.
        @note We use the same single material (vertex program) for each object, so we're relying on that we use Ogre software skinning. 
        Hardware skinning would require us to do different vertex programs for skinned/nonskinned geometry. */
    sceneManager_->setShadowColour(Ogre::ColourValue(0.6f, 0.6f, 0.6f));
    sceneManager_->setShadowTextureCountPerLightType(Ogre::Light::LT_DIRECTIONAL, shadowTextureCount);
    sceneManager_->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
    sceneManager_->setShadowTextureCasterMaterial("rex/ShadowCaster");
    sceneManager_->setShadowTextureSelfShadow(true);
    sceneManager_->setShadowTextureFSAA(shadowTextureFSAA);

    /** On DirectX/Windows PF_FLOAT32_R format produces a blue tinted shadow. This occurs 
        at least when the basic "rex/ShadowCaster" is enabled (default for materials). */
#ifdef DIRECTX_ENABLED
    sceneManager_->setShadowTextureSettings(shadowTextureSize, shadowTextureCount, Ogre::PF_R8G8B8A8);
#else
    sceneManager_->setShadowTextureSettings(shadowTextureSize, shadowTextureCount, Ogre::PF_FLOAT32_R);
#endif

    Ogre::ShadowCameraSetupPtr shadowCameraSetup;
    if (pssmEnabled)
    {
        /** The splitpoints are hardcoded also to the shaders. 
            If you modify these, also change them to the shaders. */
        OgreShadowCameraSetupFocusedPSSM::SplitPointList splitpoints;
        splitpoints.push_back(0.1f); // Default nearclip
        splitpoints.push_back(3.5);
        splitpoints.push_back(11);
        splitpoints.push_back(shadowFarDist);
        
#include "DisableMemoryLeakCheck.h"
        OgreShadowCameraSetupFocusedPSSM* pssmSetup = new OgreShadowCameraSetupFocusedPSSM();
#include "EnableMemoryLeakCheck.h"
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
    
    sceneManager_->setShadowCameraSetup(shadowCameraSetup);
    sceneManager_->setShadowFarDistance(shadowFarDist);
    sceneManager_->setShadowCasterRenderBackFaces(false); // If set to true, problems with objects that clip into the ground
    
    // Debug overlays for shadow textures
    /*
    if(renderer_.expired())
        return;
    Ogre::TexturePtr shadowTex;
    Ogre::String str("shadowDebug");
    Ogre::Overlay* debugOverlay = Ogre::OverlayManager::getSingleton().getByName(str);
    if (!debugOverlay)
        debugOverlay= Ogre::OverlayManager::getSingleton().create(str);
    for(int i = 0; i<shadowTextureCount;i++)
    {
            shadowTex = sceneManager_->getShadowTexture(i);

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
    debugOverlay->show();
    */

    bool softShadowEnabled = framework_->Config()->Get(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_RENDERING, "soft shadow", false).toBool();
    if (softShadowEnabled)
    {
        for(size_t i=0;i<shadowTextureCount;i++)
        {
            ::GaussianListener* gaussianListener = new GaussianListener();
            Ogre::TexturePtr shadowTex = sceneManager_->getShadowTexture(0);
            Ogre::RenderTarget* shadowRtt = shadowTex->getBuffer()->getRenderTarget();
            Ogre::Viewport* vp = shadowRtt->getViewport(0);
            Ogre::CompositorInstance *instance = Ogre::CompositorManager::getSingleton().addCompositor(vp, "Gaussian Blur");
            Ogre::CompositorManager::getSingleton().setCompositorEnabled(vp, "Gaussian Blur", true);
            instance->addListener(gaussianListener);
            gaussianListener->notifyViewportSize(vp->getActualWidth(), vp->getActualHeight());
            gaussianListeners_.push_back(gaussianListener);
        }
    }
}

Ogre::Camera* OgreWorld::VerifyCurrentSceneCamera() const
{
    EC_Camera* cameraComponent = VerifyCurrentSceneCameraComponent();
    return cameraComponent ? cameraComponent->GetCamera() : 0;
}

EC_Camera* OgreWorld::VerifyCurrentSceneCameraComponent() const
{
    if (!renderer_)
        return 0;
    Entity *mainCamera = renderer_->MainCamera();
    if (!mainCamera)
        return 0;
    EC_Camera *cameraComponent = dynamic_cast<EC_Camera*>(mainCamera->GetComponent<EC_Camera>().get());
    if (!cameraComponent)
        return 0;
    Entity* entity = cameraComponent->ParentEntity();
    if (!entity || entity->ParentScene() != scene_.lock().get())
        return 0;
    
    return cameraComponent;
}

bool OgreWorld::IsActive() const
{
    return VerifyCurrentSceneCamera() != 0;
}

bool OgreWorld::IsDebugInstancingEnabled() const
{
    return drawDebugInstancing_;
}

bool OgreWorld::IsInstancingStatic(const QString &meshRef)
{
    QString ref = AssetAPI::SanitateAssetRef(framework_->Asset()->ResolveAssetRef("", meshRef));
    for (int i=0; i<intancingTargets_.size(); ++i)
    {
        InstancingTarget *intancingTarget = intancingTargets_[i];
        if (intancingTarget->ref.compare(ref, Qt::CaseSensitive) == 0)
        {
            // Static instancing is set for all targets with mesh ref or none of them
            // so we can just return the first ones static property.
            for (int k=0; k<intancingTarget->targets.size(); ++k)
            {
                MeshInstanceTarget *meshTarget = intancingTarget->targets[k];
                if (meshTarget)
                    return meshTarget->isStatic;
            }
        }
    }
    return false;
}

bool OgreWorld::SetInstancingStatic(const QString &meshRef, bool _static)
{
    QString ref = AssetAPI::SanitateAssetRef(framework_->Asset()->ResolveAssetRef("", meshRef));
    for (int i=0; i<intancingTargets_.size(); ++i)
    {
        InstancingTarget *intancingTarget = intancingTargets_[i];
        if (intancingTarget->ref.compare(ref, Qt::CaseSensitive) == 0)
        {
            for (int k=0; k<intancingTarget->targets.size(); ++k)
            {
                MeshInstanceTarget *meshTarget = intancingTarget->targets[k];
                if (meshTarget)
                    meshTarget->SetBatchesStatic(_static);
            }
            return true;
        }
    }
    return false;
}

void OgreWorld::SetDebugInstancingEnabled(bool enabled)
{
    drawDebugInstancing_ = enabled;
    
    for (int i=0; i<intancingTargets_.size(); ++i)
    {
        InstancingTarget *intancingTarget = intancingTargets_[i];
        for (int k=0; k<intancingTarget->targets.size(); ++k)
        {
            MeshInstanceTarget *meshTarget = intancingTarget->targets[k];
            if (meshTarget->manager)
                meshTarget->manager->setSetting(Ogre::InstanceManager::SHOW_BOUNDINGBOX, drawDebugInstancing_);
        }
    }
}

void OgreWorld::DebugDrawAABB(const AABB &aabb, const Color &clr, bool depthTest)
{
    for(int i = 0; i < 12; ++i)
        DebugDrawLineSegment(aabb.Edge(i), clr, depthTest);
}

void OgreWorld::DebugDrawOBB(const OBB &obb, const Color &clr, bool depthTest)
{
    for(int i = 0; i < 12; ++i)
        DebugDrawLineSegment(obb.Edge(i), clr, depthTest);
}

void OgreWorld::DebugDrawLineSegment(const LineSegment &l, const Color &clr, bool depthTest)
{
    if (depthTest)
    {
        if (debugLines_)
            debugLines_->addLine(l.a, l.b, clr);
    }
    else
    {
        if (debugLinesNoDepth_)
            debugLinesNoDepth_->addLine(l.a, l.b, clr);
    }
}

void OgreWorld::DebugDrawLine(const float3& start, const float3& end, const Color &clr, bool depthTest)
{
    if (depthTest)
    {
        if (debugLines_)
            debugLines_->addLine(start, end, clr);
    }
    else
    {
        if (debugLinesNoDepth_)
            debugLinesNoDepth_->addLine(start, end, clr);
    }
}

void OgreWorld::DebugDrawPlane(const Plane &plane, const Color &clr, const float3 &refPoint, float uSpacing, float vSpacing, 
                               int uSegments, int vSegments, bool depthTest)
{
    float U0 = -uSegments * uSpacing / 2.f;
    float V0 = -vSegments * vSpacing / 2.f;

    float U1 = uSegments * uSpacing / 2.f;
    float V1 = vSegments * vSpacing / 2.f;

    for(int y = 0; y < vSegments; ++y)
        for(int x = 0; x < uSegments; ++x)
        {
            float u = U0 + x * uSpacing;
            float v = V0 + y * vSpacing;
            DebugDrawLine(plane.Point(U0, v, refPoint), plane.Point(U1, v, refPoint), clr, depthTest);
            DebugDrawLine(plane.Point(u, V0, refPoint), plane.Point(u, V1, refPoint), clr, depthTest);
        }
}

void OgreWorld::DebugDrawTransform(const Transform &t, float axisLength, float boxSize, const Color &clr, bool depthTest)
{
    DebugDrawFloat3x4(t.ToFloat3x4(), axisLength, boxSize, clr, depthTest);
}

void OgreWorld::DebugDrawFloat3x4(const float3x4 &t, float axisLength, float boxSize, const Color &clr, bool depthTest)
{
    AABB aabb(float3::FromScalar(-boxSize/2.f), float3::FromScalar(boxSize/2.f));
    OBB obb = aabb.Transform(t);
    DebugDrawOBB(obb, clr);
    DebugDrawLineSegment(LineSegment(t.TranslatePart(), t.TranslatePart() + axisLength * t.Col(0)), 1, 0, 0, depthTest);
    DebugDrawLineSegment(LineSegment(t.TranslatePart(), t.TranslatePart() + axisLength * t.Col(1)), 0, 1, 0, depthTest);
    DebugDrawLineSegment(LineSegment(t.TranslatePart(), t.TranslatePart() + axisLength * t.Col(2)), 0, 0, 1, depthTest);
}

void OgreWorld::DebugDrawCircle(const Circle &c, int numSubdivisions, const Color &clr, bool depthTest)
{
    float3 p = c.GetPoint(0);
    for(int i = 1; i <= numSubdivisions; ++i)
    {
        float3 p2 = c.GetPoint(i * 2.f * 3.14f / numSubdivisions);
        DebugDrawLineSegment(LineSegment(p, p2), clr, depthTest);
        p = p2;
    }
}

void OgreWorld::DebugDrawAxes(const float3x4 &t, bool depthTest)
{
    float3 translate, scale;
    Quat rotate;
    t.Decompose(translate, rotate, scale);
    
    DebugDrawLine(translate, translate + rotate * float3(scale.x, 0.f, 0.f), 1, 0, 0, depthTest);
    DebugDrawLine(translate, translate + rotate * float3(0., scale.y, 0.f), 0, 1, 0, depthTest);
    DebugDrawLine(translate, translate + rotate * float3(0.f, 0.f, scale.z), 0, 0, 1, depthTest);
}

void OgreWorld::DebugDrawSphere(const float3& center, float radius, int vertices, const Color &clr, bool depthTest)
{
    if (vertices <= 0)
        return;
    
    std::vector<float3> positions(vertices);

    Sphere sphere(center, radius);
    int actualVertices = sphere.Triangulate(&positions[0], 0, 0, vertices, true);
    for (int i = 0; i < actualVertices; i += 3)
    {
        DebugDrawLine(positions[i], positions[i + 1], clr, depthTest);
        DebugDrawLine(positions[i + 1], positions[i + 2], clr, depthTest);
        DebugDrawLine(positions[i + 2], positions[i], clr, depthTest);
    }
}

void OgreWorld::DebugDrawLight(const float3x4 &t, int lightType, float range, float spotAngle, const Color &clr, bool depthTest)
{
    float3 translate, scale;
    Quat rotate;
    t.Decompose(translate, rotate, scale);
    float3 lightDirection = rotate * float3(0.0f, 0.0f, 1.0f);
    switch (lightType)
    {
        // Point
    case 0:
        DebugDrawCircle(Circle(translate, float3(1.f, 0.f, 0.f), range), 8, clr, depthTest);
        DebugDrawCircle(Circle(translate, float3(0.f, 1.f, 0.f), range), 8, clr, depthTest);
        DebugDrawCircle(Circle(translate, float3(0.f, 0.f, 1.f), range), 8, clr, depthTest);
        break;
        
        // Spot
    case 1:
        {
            float3 endPoint = translate + range * lightDirection;
            float coneRadius = range * sinf(DegToRad(spotAngle));
            Circle spotCircle(endPoint, -lightDirection, coneRadius);
            
            DebugDrawCircle(Circle(endPoint, -lightDirection, coneRadius), 8, clr, depthTest);
            for (int i = 1; i <= 8; ++i)
                DebugDrawLine(translate, spotCircle.GetPoint(i * 2.f * 3.14f / 8), clr, depthTest);
        }
        break;
        
        // Directional
    case 2:
        {
            const float cDirLightRange = 10.f;
            float3 endPoint = translate + cDirLightRange * lightDirection;
            float3 offset = rotate * float3(1.f, 0.f, 0.f);
            DebugDrawLine(translate, endPoint, clr, depthTest);
            DebugDrawLine(translate + offset, endPoint + offset, clr, depthTest);
            DebugDrawLine(translate - offset, endPoint - offset, clr, depthTest);
        }
        break;
    }
}

void OgreWorld::DebugDrawCamera(const float3x4 &t, float size, const Color &clr, bool depthTest)
{
    AABB aabb(float3(-size/2.f, -size/2.f, -size), float3(size/2.f, size/2.f, size));
    OBB obb = aabb.Transform(t);
    DebugDrawOBB(obb, clr, depthTest);
    
    float3 translate(0, 0, -size * 1.25f);
    AABB aabb2(translate + float3::FromScalar(-size/4.f), translate + float3::FromScalar(size/4.f));
    OBB obb2 = aabb2.Transform(t);
    DebugDrawOBB(obb2, clr, depthTest);
}

void OgreWorld::DebugDrawSoundSource(const float3 &soundPos, float soundInnerRadius, float soundOuterRadius, const Color &clr, bool depthTest)
{
    // Draw three concentric diamonds as a visual cue
    for(int i = 2; i < 5; ++i)
        DebugDrawSphere(soundPos, i/3.f, 24, i==2?1:0, i==3?1:0, i==4?1:0, depthTest);

    DebugDrawSphere(soundPos, soundInnerRadius, 24*3*3*3, 1, 0, 0, depthTest);
    DebugDrawSphere(soundPos, soundOuterRadius, 24*3*3*3, 0, 1, 0, depthTest);
}

/// MeshInstanceTarget

MeshInstanceTarget::MeshInstanceTarget(int _submesh, bool _static) :
    submesh(_submesh),
    manager(0),
    isStatic(_static),
    optimizationTimer_(new QTimer())
{
    optimizationTimer_->setSingleShot(true);
    connect(optimizationTimer_, SIGNAL(timeout()), SLOT(OptimizeBatches()));
}

MeshInstanceTarget::~MeshInstanceTarget()
{
    SAFE_DELETE(optimizationTimer_);
    manager = 0;
}

Ogre::InstancedEntity *MeshInstanceTarget::CreateInstance(const QString &material, Ogre::InstancedEntity *parent)
{
    PROFILE(OgreWorld_MeshInstanceTarget_CreateInstance);

    if (!manager)
        throw ::Exception("Instance manager is null, cannot create a new instance.");

    /// @todo Check if this can throw!
    Ogre::InstancedEntity *instance = manager->createInstancedEntity(material.toStdString());
    if (parent)
        parent->shareTransformWith(instance);

    instances << instance;

    InvokeOptimizations();
    return instance;
}

bool MeshInstanceTarget::ForgetInstances(Ogre::InstancedEntity* instance)
{
    return ForgetInstances(QList<Ogre::InstancedEntity*>() << instance);
}

bool MeshInstanceTarget::ForgetInstances(QList<Ogre::InstancedEntity*> _instances)
{
    PROFILE(OgreWorld_MeshInstanceTarget_ForgetInstances);

    bool found = false;
    foreach(Ogre::InstancedEntity *instance, _instances)
    {
        if (instances.contains(instance))
        {
            instances.removeAll(instance);
            found = true;
        }
    }
    if (found)
        InvokeOptimizations();
    return found;
}

void MeshInstanceTarget::InvokeOptimizations(int optimizeAfterMsec)
{
    if (!manager || instances.size() == 0)
        return;

    /** This timer waits for 5 seconds of rest before running optimizations to the managers batches.
        We don't want to be running these eg. after each instance creation as it would produce lots of resize/allocations
        in the target batches. It's best to do them after a big number or creations/deletions. */
    if (optimizationTimer_->isActive())
        optimizationTimer_->stop();
    optimizationTimer_->start(optimizeAfterMsec);
}

void MeshInstanceTarget::OptimizeBatches()
{
    PROFILE(OgreWorld_MeshInstanceTarget_OptimizeBatches);
    if (manager)
    {
        manager->cleanupEmptyBatches();
        manager->defragmentBatches(true);
        manager->setBatchesAsStaticAndUpdate(isStatic);
    }
}

void MeshInstanceTarget::SetBatchesStatic(bool _static)
{
    PROFILE(OgreWorld_MeshInstanceTarget_SetBatchesStatic);
    if (isStatic == _static)
        return;

    isStatic = _static;
    if (manager && !optimizationTimer_->isActive())
        manager->setBatchesAsStaticAndUpdate(isStatic);
}

/// InstancingTarget

InstancingTarget::InstancingTarget(const QString &meshRef) :
    ref(meshRef)
{
}

InstancingTarget::~InstancingTarget()
{
    foreach(MeshInstanceTarget *target, targets)
        if (target) SAFE_DELETE(target);
    targets.clear();
}
