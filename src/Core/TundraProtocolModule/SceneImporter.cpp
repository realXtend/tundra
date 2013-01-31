// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneImporter.h"
#include "TundraLogicModule.h"

#include "Scene/Scene.h"
#include "Entity.h"
#include "SceneDesc.h"
#include "OgreMaterialUtils.h"
#include "CoreStringUtils.h"
#include "Math/float3.h"
#include "Math/Quat.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "EC_Name.h"
#include "Renderer.h"
#include "AssetAPI.h"
#include "LoggingFunctions.h"
#include "SceneAPI.h"
#include "OgreRenderingModule.h"
#include "CoreException.h"
#include "FileUtils.h"

#include <Ogre.h>

#include <QDomDocument>
#include <QFile>
#include <QDir>

#include "MemoryLeakCheck.h"

namespace TundraLogic
{

SceneImporter::SceneImporter(const ScenePtr &scene) :
    scene_(scene)
{
}

SceneImporter::~SceneImporter()
{
}

EntityPtr SceneImporter::ImportMesh(const QString &filename, const QString &in_asset_dir, const Transform &worldtransform,
    const QString &entity_prefab_xml, const QString &prefix, AttributeChange::Type change, bool inspect,
    const QString &meshName)
{
    if (!scene_)
    {
        LogError("Null scene for mesh import");
        return EntityPtr();
    }

    QString meshleafname = QFileInfo(filename).fileName();

    QStringList material_names;
    QString skeleton_name;
    if (inspect)
        if (!ParseMeshForMaterialsAndSkeleton(filename, material_names, skeleton_name))
            return EntityPtr();

    QSet<QString> material_names_set;
    for(uint i = 0; i < (uint)material_names.size(); ++i)
    {
        LogDebug("Material ref: " + material_names[i]);
        material_names_set.insert(material_names[i]);
    }

    LogDebug("Skeleton ref: " + skeleton_name);

    // Scan the asset dir for material files, because we don't actually know what material file the mesh refers to.
    QStringList material_files;
    if (inspect)
        material_files = OgreRenderer::FindMaterialFiles(in_asset_dir);

    // mesh copied, add mesh name inside the file
    if (!meshName.isEmpty() /*&& createMesh*/)
        meshleafname += '/' + meshName;

    // Create a new entity in any case, with a new ID
    EntityPtr newentity = scene_->CreateEntity(0, QStringList(), change, true);
    if (!newentity)
    {
        LogError("Could not create entity for mesh");
        return newentity;
    }
    
    // If the prefab contains valid data, instantiate the components from there
    QDomDocument prefab;
    prefab.setContent(entity_prefab_xml);
    QDomElement ent_elem = prefab.firstChildElement("entity");
    if (!ent_elem.isNull())
    {
        /// \todo this should possibly be a function in Scene
        QDomElement comp_elem = ent_elem.firstChildElement("component");
        while(!comp_elem.isNull())
        {
            QString type_name = comp_elem.attribute("type");
            QString name = comp_elem.attribute("name");
            ComponentPtr newcomp = newentity->GetOrCreateComponent(type_name, name);
            if (newcomp)
                // Trigger no signal yet when entity is in incoherent state
                newcomp->DeserializeFrom(comp_elem, AttributeChange::Disconnected);
            comp_elem = comp_elem.nextSiblingElement("component");
        }
    }

    // Fill the placeable attributes
    shared_ptr<EC_Placeable> placeablePtr = dynamic_pointer_cast<EC_Placeable>(newentity->GetOrCreateComponent(EC_Placeable::TypeNameStatic(), change));
    if (placeablePtr)
        placeablePtr->transform.Set(worldtransform, AttributeChange::Disconnected);
    else
        LogError("No EC_Placeable was created!");

    // Fill the mesh attributes
    AssetReferenceList materials;
    foreach(QString matName, material_names)
        materials.Append(AssetReference(prefix + matName + ".material"));

    shared_ptr<EC_Mesh> meshPtr = dynamic_pointer_cast<EC_Mesh>(newentity->GetOrCreateComponent(EC_Mesh::TypeNameStatic(), change));
    if (meshPtr)
    {
        meshPtr->meshRef.Set(AssetReference(prefix + meshleafname), AttributeChange::Disconnected);
        if (!skeleton_name.isEmpty())
            meshPtr->skeletonRef.Set(AssetReference(prefix + skeleton_name), AttributeChange::Disconnected);
        meshPtr->meshMaterial.Set(materials, AttributeChange::Disconnected);

        if (inspect)
            meshPtr->nodeTransformation.Set(Transform(float3(0,0,0), float3(90,0,180), float3(1,1,1)), AttributeChange::Disconnected);
        else
            meshPtr->nodeTransformation.Set(Transform(), AttributeChange::Disconnected);
    }
    else
        LogError("No EC_Mesh was created!");

    // Fill the name attributes
    shared_ptr<EC_Name> namePtr = dynamic_pointer_cast<EC_Name>(newentity->GetOrCreateComponent(EC_Name::TypeNameStatic(), change));
    if (namePtr)
        ///\todo Use name of scenedesc?
        namePtr->name.Set(meshleafname.replace(".mesh", ""), AttributeChange::Disconnected);
    else
        LogError("No EC_Name was created!");

    // All components have been loaded/modified. Trigger change for them now.
    scene_->EmitEntityCreated(newentity.get(), change);
    const Entity::ComponentMap &components = newentity->Components();
    for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
        i->second->ComponentChanged(change);

    return newentity;
}

QList<Entity *> SceneImporter::Import(const QString &filename, const QString &in_asset_dir, const Transform &worldtransform,
    const QString &prefix, AttributeChange::Type change, bool clearscene, bool replace)
{
    QList<Entity *> ret;
    if (!scene_)
    {
        LogError("SceneImporter::Import: Null scene for import");
        return ret;
    }

    try
    {
        if (clearscene)
            LogInfo("SceneImporter::Import: Importing scene from " + filename + " and clearing the old");
        else
            LogInfo("SceneImporter::Import: Importing scene from " + filename);

        QFile file(filename);
        if (!file.open(QFile::ReadOnly))
        {
            file.close();
            LogError("SceneImporter::Import: Failed to open file " + filename);
            return QList<Entity *>();
        }
        
        QDomDocument dotscene;
        if (!dotscene.setContent(&file))
        {
            file.close();
            LogError("SceneImporter::Import:Failed to parse XML content");
            return ret;
        }
        
        file.close();
        
        QDomElement scene_elem = dotscene.firstChildElement("scene");
        if (scene_elem.isNull())
        {
            LogError("SceneImporter::Import:No 'scene' element");
            return ret;
        }
        QDomElement nodes_elem = scene_elem.firstChildElement("nodes");
        if (nodes_elem.isNull())
        {
            LogError("SceneImporter::Import No 'nodes' element");
            return ret;
        }
        
        // We assume two cases:
        // - Blender exporter, up axis not specified; Ogre meshes will be exported as Y-up, but the scene is Z-up (!). Need to change the scene coordinate system.
        // - Up axis specified as Y: no conversion needs to be done
        bool flipyz = true;
        QString upaxis = scene_elem.attribute("upAxis");
        if (upaxis == "y")
            flipyz = false;
        
        if (clearscene)
            scene_->RemoveAllEntities(true, change);
        
        QDomElement node_elem = nodes_elem.firstChildElement("node");
        
        // First pass: get used assets
        LogDebug("SceneImporter::Import: Processing scene for assets");
        ProcessNodeForAssets(node_elem, in_asset_dir);
        
        // Write out the needed assets
        LogDebug("SceneImporter::Import: Saving needed assets");
        // By default, assume the material file is scenename.material if scene is scenename.scene.
        // However, if an external reference exists, use that.
        QString matfilename = filename;
        matfilename.replace(".scene", ".material");
        QDomElement externals_elem = scene_elem.firstChildElement("externals");
        if (!externals_elem.isNull())
        {
            QDomElement item_elem = externals_elem.firstChildElement("item");
            while(!item_elem.isNull())
            {
                if (item_elem.attribute("type") == "material")
                {
                    QDomElement file_elem = item_elem.firstChildElement("file");
                    if (!file_elem.isNull())
                    {
                        matfilename = in_asset_dir + '/' + file_elem.attribute("name");
                        break;
                    }
                }
                item_elem = item_elem.nextSiblingElement();
            }
        }
        
//        ProcessAssets(matfilename, in_asset_dir, out_asset_dir, localassets);
        
        // Second pass: build scene hierarchy and actually create entities. This assumes assets are available
        LogDebug("SceneImporter::Import: Creating entities");

        Quat rot = worldtransform.Orientation();
        ProcessNodeForCreation(ret, node_elem, worldtransform.pos, rot, worldtransform.scale, change, prefix, flipyz, replace);
    }
    catch(Exception& e)
    {
        LogError(QString("SceneImporter::Import: Exception while scene importing " + filename + ": ") + e.what());
        return QList<Entity *>();
    }
    
    LogDebug("Finished");

    return ret;
}

bool SceneImporter::ParseMeshForMaterialsAndSkeleton(const QString& meshname, QStringList & material_names, QString& skeleton_name) const
{
    material_names.clear();
    
    QFile mesh_in(meshname);
    if (!mesh_in.open(QFile::ReadOnly))
    {
        LogError("SceneImporter::ParseMeshForMaterialsAndSkeleton: Could not open input mesh file " + meshname);
        return false;
    }
    else
    {
        QByteArray mesh_bytes = mesh_in.readAll();
        mesh_in.close();
        OgreRenderer::RendererPtr renderer = scene_->GetFramework()->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
        if (!renderer)
        {
            LogError("SceneImporter::ParseMeshForMaterialsAndSkeleton: Renderer does not exist");
            return false;
        }
        
        std::string uniquename = renderer->GetUniqueObjectName("SceneImport_tempmesh");
        try
        {
            Ogre::MeshPtr tempmesh = Ogre::MeshManager::getSingleton().createManual(uniquename, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            if (tempmesh.isNull())
            {
                LogError("SceneImporter::ParseMeshForMaterialsAndSkeleton: failed to create temp mesh");
                return false;
            }
            
#include "DisableMemoryLeakCheck.h"
            Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)mesh_bytes.data(), mesh_bytes.size(), false));
#include "EnableMemoryLeakCheck.h"
            Ogre::MeshSerializer serializer;
            serializer.importMesh(stream, tempmesh.getPointer());
            
            for(uint i = 0; i < tempmesh->getNumSubMeshes(); ++i)
            {
                Ogre::SubMesh* submesh = tempmesh->getSubMesh(i);
                if (submesh)
                {
                    // Replace / with _ from material name
                    std::string submeshmat = submesh->getMaterialName();
                    ReplaceCharInplace(submeshmat, '/', '_');
                    
                    material_names.push_back(submeshmat.c_str());
                }
            }
            skeleton_name = tempmesh->getSkeletonName().c_str();
            
            // Now delete the mesh as we're done with inspecting it
            tempmesh.setNull();
            Ogre::MeshManager::getSingleton().remove(uniquename);
        }
        catch(...)
        {
            LogError("SceneImporter::ParseMeshForMaterialsAndSkeleton: Exception while inspecting mesh " + meshname);
            return false;
        }
    }
    
    return true;
}

SceneDesc SceneImporter::CreateSceneDescFromMesh(const QString &source) const
{
    SceneDesc sceneDesc;

    if (!source.endsWith(".mesh", Qt::CaseInsensitive))
    {
        LogError("SceneImporter::CreateSceneDescFromMesh: Unsupported file type for scene description creation: " + source);
        return sceneDesc;
    }

    sceneDesc.filename = source;

    // For files
    QStringList materialNames;
    QStringList skeletons;
    QString skeletonName;
    QString path;
    QString meshleafname;
    // For both URL and file
    QString meshEntityName;
    bool isUrl = source.startsWith("http://") || source.startsWith("https://");
    if (isUrl)
    {
        meshEntityName = source;
        meshEntityName = meshEntityName.split("/").last();
        meshEntityName = meshEntityName.split(".mesh").first();
    }
    else
    {
        path = QFileInfo(source).dir().path();
        meshleafname = QFileInfo(source).fileName();

        if (!ParseMeshForMaterialsAndSkeleton(source, materialNames, skeletonName))
            return sceneDesc;

        if (!skeletonName.isEmpty())
            skeletons << skeletonName;

        // Construct entity name from the mesh file name.
        int idx = meshleafname.lastIndexOf(".mesh");
        meshEntityName = meshleafname;
        meshEntityName.remove(idx, meshleafname.length() - idx);
    }

    EntityDesc entityDesc("", meshEntityName);

    ComponentDesc meshDesc;
    meshDesc.typeName = EC_Mesh::TypeNameStatic();

    ComponentDesc placeableDesc;
    placeableDesc.typeName = EC_Placeable::TypeNameStatic();

    ComponentDesc nameDesc;
    nameDesc.typeName = EC_Name::TypeNameStatic();

    if (isUrl)
    {
        // Create asset description
        AssetDesc ad;
        ad.source = source;
        ad.dataInMemory = false;
        ad.typeName = "mesh";
        ad.destinationName = "";
        sceneDesc.assets[qMakePair(ad.source, ad.subname)] = ad;
    }
    else
    {
        // Scan the asset dir for material files, because we don't actually know what material file the mesh refers to.
        QStringList meshFiles(QStringList() << source);
        QSet<QString> usedMaterials = materialNames.toSet();
        QStringList materialFiles = OgreRenderer::FindMaterialFiles(path);

        CreateAssetDescs(path, meshFiles, skeletons, materialFiles, usedMaterials, sceneDesc);

        // mesh copied, add mesh name inside the file
        //if (!meshName.empty())
        //    meshleafname += std::string("/") + meshName;
    }

    // Create attribute descriptions.
    SceneAPI *sceneAPI = scene_->GetFramework()->Scene(); ///\todo Replace with scene_->SceneAPI();

    // Fill the mesh attributes
    AssetReferenceList materials("OgreMaterial");
    foreach(QString matName, materialNames)
        materials.Append(AssetReference(path + "/" + matName + ".material"));

    /// \todo This creates dummy components, specifying a null scene during creation
    shared_ptr<EC_Mesh> mesh = sceneAPI->CreateComponent<EC_Mesh>(0);
    assert(mesh);
    if (mesh)
    {
        if (isUrl)
        {
            mesh->meshRef.Set(AssetReference(source), AttributeChange::Disconnected);
        }
        else
        {
            mesh->meshRef.Set(AssetReference(path + "/" + meshleafname), AttributeChange::Disconnected);
            mesh->meshMaterial.Set(materials, AttributeChange::Disconnected);
            if (!skeletonName.isEmpty())
                mesh->skeletonRef.Set(AssetReference(path + "/" + skeletonName), AttributeChange::Disconnected);
        }

        foreach(IAttribute *a, mesh->Attributes())
        {
            AttributeDesc attrDesc = { a->TypeName(), a->Name(), a->ToString().c_str() };
            meshDesc.attributes.append(attrDesc);
        }
    }

    shared_ptr<EC_Placeable> placeable = sceneAPI->CreateComponent<EC_Placeable>(0);
    assert(placeable);
    if (placeable)
        foreach(IAttribute *a, placeable->Attributes())
        {
            AttributeDesc attrDesc = { a->TypeName(), a->Name(), a->ToString().c_str() };
            placeableDesc.attributes.append(attrDesc);
        }

    shared_ptr<EC_Name> name = sceneAPI->CreateComponent<EC_Name>(0);
    assert(name);
    if (name)
    {
        name->name.Set(meshEntityName, AttributeChange::Disconnected);
        foreach(IAttribute *a, name->Attributes())
        {
            AttributeDesc attrDesc = { a->TypeName(), a->Name(), a->ToString().c_str() };
            nameDesc.attributes.append(attrDesc);
        }
    }

    entityDesc.components << meshDesc << placeableDesc << nameDesc;
    sceneDesc.entities << entityDesc;

    return sceneDesc;
}

SceneDesc SceneImporter::CreateSceneDescFromScene(const QString &filename)
{
    SceneDesc sceneDesc;

    if (!filename.endsWith(".scene", Qt::CaseInsensitive))
    {
        LogError("SceneImporter::CreateSceneDescFromScene: Unsupported file type for scene description creation: " + filename);
        return sceneDesc;
    }

    sceneDesc.filename = filename;

    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
    {
        file.close();
        LogError("SceneImporter::CreateSceneDescFromScene: Failed to open file " + filename);
        return sceneDesc;
    }

    QDomDocument dotscene;
    QString errorMsg;
    int errorLine, errorColumn;
    if (!dotscene.setContent(&file, false, &errorMsg, &errorLine, &errorColumn))
    {
        file.close();
        LogError(QString("SceneImporter::CreateSceneDescFromScene: Parsing Ogre .scene XML from %1 failed when loading Scene XML: %2 at line %3 column %4.").arg(filename).arg(errorMsg).arg(errorLine).arg(errorColumn));
        return sceneDesc;
    }
    
    file.close();
    
    QDomElement scene_elem = dotscene.firstChildElement("scene");
    if (scene_elem.isNull())
    {
        LogError("SceneImporter::CreateSceneDescFromScene: No 'scene' element.");
        return sceneDesc;
    }

    QDomElement nodes_elem = scene_elem.firstChildElement("nodes");
    if (nodes_elem.isNull())
    {
        LogError("SceneImporter::CreateSceneDescFromScene: No 'nodes' element.");
        return sceneDesc;
    }

    QString path = QFileInfo(filename).dir().path();

    /// @todo Do we want to read environment settings from the .scene? Implement!

    // By default, assume the material file is scenename.material if scene is scenename.scene.
    // However, if an external reference exists, use that.
    QString materialFileName = filename;
    materialFileName.replace(".scene", ".material");

    QDomElement externals_elem = scene_elem.firstChildElement("externals");
    if (!externals_elem.isNull())
    {
        QDomElement item_elem = externals_elem.firstChildElement("item");
        while(!item_elem.isNull())
        {
            if (item_elem.attribute("type") == "material")
            {
                QDomElement file_elem = item_elem.firstChildElement("file");
                if (!file_elem.isNull())
                {
                    QString extMatName = path + "/" + file_elem.attribute("name");
                    if (QFile::exists(extMatName))
                        materialFileName = extMatName;
                    break;
                }
            }
            item_elem = item_elem.nextSiblingElement();
        }
    }

    QStringList meshFiles;
    QStringList skeletons;
    QSet<QString> usedMaterials;
    QStringList materialFiles(materialFileName);

    Transform f;
    Quat rot = f.Orientation();

    QDomElement node_elem = nodes_elem.firstChildElement("node");
    while(!node_elem.isNull())
    {
        ProcessNodeForDesc(sceneDesc, node_elem, f.pos, rot, f.scale, path + "/"/*prefix*/, false/*flipyz*/, meshFiles, skeletons, usedMaterials);

        // Process siblings
        node_elem = node_elem.nextSiblingElement("node");
    }

    CreateAssetDescs(path, meshFiles, skeletons, materialFiles, usedMaterials, sceneDesc);

    return sceneDesc;
}

void SceneImporter::ProcessNodeForAssets(QDomElement node_elem, const QString& in_asset_dir)
{
    while(!node_elem.isNull())
    {
        // Process entity node, if any
        QDomElement entity_elem = node_elem.firstChildElement("entity");
        if (!entity_elem.isNull())
        {
            QString mesh_name = entity_elem.attribute("meshFile");
            // Store the original name. Later we fix duplicates.
            mesh_names_[mesh_name] = mesh_name;
            QDomElement subentities_elem = entity_elem.firstChildElement("subentities");
            if (!subentities_elem.isNull())
            {
                QDomElement subentity_elem = subentities_elem.firstChildElement("subentity");
                while(!subentity_elem.isNull())
                {
                    QString material_name = subentity_elem.attribute("materialName");
                    material_names_.insert(material_name);
                    subentity_elem = subentity_elem.nextSiblingElement("subentity");
                }
            }
            else
            {
                // If no subentity element, have to interrogate the mesh.
                QStringList material_names;
                QString skeleton_name;
                QString meshRef;

                AssetAPI::FileQueryResult result = scene_->GetFramework()->Asset()->ResolveLocalAssetPath(mesh_name, in_asset_dir, meshRef);
                if (result == AssetAPI::FileQueryLocalFileMissing)
                    LogWarning("Mesh file \"" + mesh_name + "\" cannot be found from path \"" + in_asset_dir + "\"!");

                ParseMeshForMaterialsAndSkeleton(meshRef, material_names, skeleton_name);
                for(uint i = 0; i < (uint)material_names.size(); ++i)
                    material_names_.insert(material_names[i]);
                mesh_default_materials_[mesh_name] = material_names;
            }
        }

        // Process child nodes
        QDomElement childnode_elem = node_elem.firstChildElement("node");
        if (!childnode_elem.isNull())
            ProcessNodeForAssets(childnode_elem, in_asset_dir);

        // Process siblings
        node_elem = node_elem.nextSiblingElement("node");
    }
}

void SceneImporter::ProcessNodeForCreation(QList<Entity* > &entities, QDomElement node_elem, float3 pos, Quat rot, float3 scale,
    AttributeChange::Type change, const QString &prefix, bool flipyz, bool replace)
{
    while(!node_elem.isNull())
    {
        QDomElement pos_elem = node_elem.firstChildElement("position");
        QDomElement rot_elem = node_elem.firstChildElement("rotation");
        QDomElement quat_elem = node_elem.firstChildElement("quaternion");
        QDomElement scale_elem = node_elem.firstChildElement("scale");
        float posx, posy, posz, rotx = 0.0f, roty = 0.0f, rotz = 0.0f, rotw = 1.0f, scalex, scaley, scalez;

        posx = ParseFloat(pos_elem.attribute("x"), 0.0f);
        posy = ParseFloat(pos_elem.attribute("y"), 0.0f);
        posz = ParseFloat(pos_elem.attribute("z"), 0.0f);

        if (!rot_elem.isNull())
        {
            rotx = ParseFloat(rot_elem.attribute("qx"), 0.0f);
            roty = ParseFloat(rot_elem.attribute("qy"), 0.0f);
            rotz = ParseFloat(rot_elem.attribute("qz"), 0.0f);
            rotw = ParseFloat(rot_elem.attribute("qw"), 1.0f);
        }
        if (!quat_elem.isNull())
        {
            rotx = ParseFloat(quat_elem.attribute("x"), 0.0f);
            roty = ParseFloat(quat_elem.attribute("y"), 0.0f);
            rotz = ParseFloat(quat_elem.attribute("z"), 0.0f);
            rotw = ParseFloat(quat_elem.attribute("w"), 1.0f);
        }

        scalex = ParseFloat(scale_elem.attribute("x"), 1.0f);
        scaley = ParseFloat(scale_elem.attribute("y"), 1.0f);
        scalez = ParseFloat(scale_elem.attribute("z"), 1.0f);

        float3 newpos(posx, posy, posz);
        Quat newrot(rotx, roty, rotz, rotw);
        float3 newscale(fabsf(scalex), fabsf(scaley), fabsf(scalez));

        // Transform by the parent transform
        newrot = rot * newrot;
        newscale = scale * newscale;
        newpos = rot * float3(scale * newpos);
        newpos += pos;

        // Process entity node, if any
        QDomElement entity_elem = node_elem.firstChildElement("entity");
        if (!entity_elem.isNull())
        {
            // Enforce uniqueness for node names, which may not be guaranteed by artists
            QString base_node_name = node_elem.attribute("name");
            if (base_node_name.isEmpty())
                base_node_name = "object";
            QString node_name = base_node_name;
            int append_num = 1;
            while(node_names_.find(node_name) != node_names_.end())
            {
                node_name = base_node_name + "_" + QString::number(append_num);
                ++append_num;
            }
            node_names_.insert(node_name);
            
            // Get mesh name from map
            QString orig_mesh_name = entity_elem.attribute("meshFile");
            QString mesh_name = mesh_names_[orig_mesh_name];
            
            bool cast_shadows = ::ParseBool(entity_elem.attribute("castShadows"));

            mesh_name = prefix + mesh_name;

            EntityPtr entity;
            bool new_entity = false;
            QString node_name_qstr = node_name;

            // Try to find existing entity by name
            if (replace)
                entity = scene_->GetEntityByName(node_name_qstr);

            if (!entity)
            {
                entity = scene_->CreateEntity(scene_->NextFreeId());
                new_entity = true;
            }
            else
            {
                LogInfo("Updating existing entity " + node_name);
            }

            shared_ptr<EC_Mesh> meshPtr;
            shared_ptr<EC_Name> namePtr;
            shared_ptr<EC_Placeable> placeablePtr;

            if (entity)
            {
                meshPtr = dynamic_pointer_cast<EC_Mesh>(entity->GetOrCreateComponent(EC_Mesh::TypeNameStatic(), change));
                namePtr = dynamic_pointer_cast<EC_Name>(entity->GetOrCreateComponent(EC_Name::TypeNameStatic(), change));
                placeablePtr = dynamic_pointer_cast<EC_Placeable>(entity->GetOrCreateComponent(EC_Placeable::TypeNameStatic(), change));
                assert(meshPtr && namePtr && placeablePtr);
                if (meshPtr && namePtr && placeablePtr)
                {
                    namePtr->name.Set(node_name_qstr, change);
                    
                    QVector<QString> materials;
                    QDomElement subentities_elem = entity_elem.firstChildElement("subentities");
                    if (!subentities_elem.isNull())
                    {
                        QDomElement subentity_elem = subentities_elem.firstChildElement("subentity");
                        while(!subentity_elem.isNull())
                        {
                            QString material_name = subentity_elem.attribute("materialName") + ".material";
                            material_name.replace('/', '_');

                            int index = subentity_elem.attribute("index").toInt();
                            material_name = prefix + material_name;
                            if (index >= materials.size())
                                materials.resize(index + 1);
                            materials[index] = material_name;

                            subentity_elem = subentity_elem.nextSiblingElement("subentity");
                        }
                    }
                    else
                    {
                        // If no subentity element, use the inspected material names we stored earlier
                        const QStringList& default_materials = mesh_default_materials_[orig_mesh_name];
                        materials.resize(default_materials.size());
                        for(uint i = 0; i < (uint)default_materials.size(); ++i)
                            materials[i] =  prefix + default_materials[i] + ".material";
                    }
                    
                    Transform entity_transform;
                    
                    /// \todo Allow any transformation of coordinate axes, not just fixed y/z flip
                    if (flipyz)
                    {
                        float3 rot_euler;
                        Quat adjustedrot(-newrot.x, newrot.z, newrot.y, newrot.w);
                        adjustedrot = Quat::FromEulerZYX(0, pi, 0) * adjustedrot;
                        entity_transform.SetPos(-newpos.x, newpos.z, newpos.y);
                        entity_transform.SetOrientation(adjustedrot);
                        entity_transform.SetScale(newscale.x, newscale.z, newscale.y);
                    }
                    else
                    {
                        entity_transform.SetPos(newpos);
                        entity_transform.SetOrientation(newrot);
                        entity_transform.SetScale(newscale);
                    }

                    placeablePtr->transform.Set(entity_transform, change);
                    meshPtr->meshRef.Set(AssetReference(mesh_name), change);

                    AssetReferenceList materialRefs;
                    foreach(QString material, materials)
                        materialRefs.Append(AssetReference(material));

                    meshPtr->meshMaterial.Set(materialRefs, change);
                    meshPtr->castShadows.Set(cast_shadows, change);

                    if (new_entity)
                        scene_->EmitEntityCreated(entity.get(), change);
                    placeablePtr->ComponentChanged(change);
                    meshPtr->ComponentChanged(change);
                    namePtr->ComponentChanged(change);

                    entities.append(entity.get());
                }
                else
                    LogError("Could not create mesh, placeable, name components");
            }
        }

        // Process child nodes
        QDomElement childnode_elem = node_elem.firstChildElement("node");
        if (!childnode_elem.isNull())
            ProcessNodeForCreation(entities, childnode_elem, newpos, newrot, newscale, change, prefix, flipyz, replace);

        // Process siblings
        node_elem = node_elem.nextSiblingElement("node");
    }
}

void SceneImporter::ProcessNodeForDesc(SceneDesc &desc, QDomElement nodeElement, float3 pos, Quat rot, float3 scale, const QString &prefix, bool flipyz, 
    QStringList &meshFiles, QStringList &skeletonFiles, QSet<QString> &usedMaterials, const QString &parentRef)
{
    AttributeChange::Type change = AttributeChange::Disconnected;
    if (!nodeElement.isNull())
    {
        QDomElement posElement = nodeElement.firstChildElement("position");
        QDomElement rotElement = nodeElement.firstChildElement("rotation");
        QDomElement quatElement = nodeElement.firstChildElement("quaternion");
        QDomElement scaleElement = nodeElement.firstChildElement("scale");
        float posX, posY, posZ, rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f, rotW = 1.0f, scaleX, scaleY, scaleZ;

        posX = ParseFloat(posElement.attribute("x"), 0.0f);
        posY = ParseFloat(posElement.attribute("y"), 0.0f);
        posZ = ParseFloat(posElement.attribute("z"), 0.0f);

        if (!rotElement.isNull())
        {
            rotX = ParseFloat(rotElement.attribute("qx"), 0.0f);
            rotY = ParseFloat(rotElement.attribute("qy"), 0.0f);
            rotZ = ParseFloat(rotElement.attribute("qz"), 0.0f);
            rotW = ParseFloat(rotElement.attribute("qw"), 1.0f);
        }
        if (!quatElement.isNull())
        {
            rotX = ParseFloat(quatElement.attribute("x"), 0.0f);
            rotY = ParseFloat(quatElement.attribute("y"), 0.0f);
            rotZ = ParseFloat(quatElement.attribute("z"), 0.0f);
            rotW = ParseFloat(quatElement.attribute("w"), 1.0f);
        }

        scaleX = ParseFloat(scaleElement.attribute("x"), 1.0f);
        scaleY = ParseFloat(scaleElement.attribute("y"), 1.0f);
        scaleZ = ParseFloat(scaleElement.attribute("z"), 1.0f);

        float3 newPos(posX, posY, posZ);
        Quat newRot(rotX, rotY, rotZ, rotW);
        float3 newScale(fabsf(scaleX), fabsf(scaleY), fabsf(scaleZ));

        // Transform by the parent transform
        newRot = rot * newRot;
        newScale = scale * newScale;
        newPos = rot * (scale * newPos);
        newPos += pos;

        // Process entity node, if any
        QDomElement entityElement = nodeElement.firstChildElement("entity");
        if (!entityElement.isNull())
        {
            EntityDesc entityDesc;
            ComponentDesc nameDesc;
            ComponentDesc placeableDesc;
            ComponentDesc meshDesc;

            nameDesc.typeName = EC_Name::TypeNameStatic();
            placeableDesc.typeName = EC_Placeable::TypeNameStatic();
            meshDesc.typeName = EC_Mesh::TypeNameStatic();

            // Enforce uniqueness for node names, which may not be guaranteed by artists
            QString baseNodeName = nodeElement.attribute("name");
            if (baseNodeName.isEmpty())
                baseNodeName = "object";
            QString nodeName = baseNodeName;
            int appendNum = 1;
            while(node_names_.contains(nodeName))
            {
                nodeName = baseNodeName + "_" + QString::number(appendNum);
                ++appendNum;
            }
            node_names_.insert(nodeName);
            
            // Get mesh name from map
            QString meshName = entityElement.attribute("meshFile");
            QString meshRef, skeletonRef;
            AssetAPI::FileQueryResult result = scene_->GetFramework()->Asset()->ResolveLocalAssetPath(meshName, prefix, meshRef);
            if (result == AssetAPI::FileQueryLocalFileMissing)
                LogWarning("Mesh file \"" + meshName + "\" cannot be found from path \"" + prefix + "\"!");

            meshFiles << meshRef;

            bool cast_shadows = ::ParseBool(entityElement.attribute("castShadows"));
            entityDesc.name = nodeName;

            SceneAPI &sceneAPI = *scene_->GetFramework()->Scene();
            shared_ptr<EC_Mesh> meshPtr = sceneAPI.CreateComponent<EC_Mesh>(0);
            shared_ptr<EC_Name> namePtr = sceneAPI.CreateComponent<EC_Name>(0);
            shared_ptr<EC_Placeable> placeablePtr = sceneAPI.CreateComponent<EC_Placeable>(0);

            if (meshPtr.get() && namePtr.get() && placeablePtr.get())
            {
                namePtr->name.Set(nodeName, change);
                if (!parentRef.isEmpty())
                    placeablePtr->parentRef.Set(EntityReference(parentRef), change);

                QVector<QString> materials;
                QDomElement subentitiesElement = entityElement.firstChildElement("subentities");
                if (!subentitiesElement.isNull())
                {
                    QDomElement subentityElement = subentitiesElement.firstChildElement("subentity");
                    while(!subentityElement.isNull())
                    {
                        QString materialName = subentityElement.attribute("materialName");
                        usedMaterials.insert(materialName);
                        materialName += ".material";
                        materialName.replace('/', '_');

                        int index = ParseInt(subentityElement.attribute("index"), 0);

                        materialName = prefix + materialName;
                        if (index >= materials.size())
                            materials.resize(index + 1);
                        materials[index] = materialName;

                        subentityElement = subentityElement.nextSiblingElement("subentity");
                    }
                }
                else // If no subentity element, use the inspected material names we stored earlier
                {
                    QStringList materialNames;
                    
                    ParseMeshForMaterialsAndSkeleton(meshRef, materialNames, skeletonRef);
                    materials.resize(materialNames.size());
                    for (uint i = 0; i < (uint)materialNames.size(); ++i)
                    {
                        usedMaterials.insert(materialNames[i]);
                        materials[i] = prefix + materialNames[i] + ".material";
                    }

                    if (!skeletonRef.isEmpty())
                        skeletonFiles << skeletonRef;
                }
                
                Transform entityTransform;
                
                if (flipyz)
                {
                    float3 eulerRot;
                    Quat adjustedRot(-newRot.x, newRot.y, newRot.z, newRot.w);
                    eulerRot = adjustedRot.ToEulerXZY();
                    entityTransform.SetPos(-newPos.x, newPos.z, newPos.y);
                    entityTransform.SetRotation(RadToDeg(eulerRot.x), RadToDeg(eulerRot.y), RadToDeg(eulerRot.z));
                    entityTransform.SetScale(newScale.x, newScale.z, newScale.y);
                }
                else
                {
                    float3 eulerRot = newRot.ToEulerXYZ();
                    entityTransform.SetPos(newPos.x, newPos.y, newPos.z);
                    entityTransform.SetRotation(RadToDeg(eulerRot.x), RadToDeg(eulerRot.y), RadToDeg(eulerRot.z));
                    entityTransform.SetScale(newScale.x, newScale.y, newScale.z);
                }

                placeablePtr->transform.Set(entityTransform, change);
                meshPtr->meshRef.Set(AssetReference(meshRef), change);

                AssetReferenceList materialRefs;
                foreach(QString material, materials)
                    materialRefs.Append(AssetReference(material));

                meshPtr->meshMaterial.Set(materialRefs, change);
                meshPtr->skeletonRef.Set(AssetReference(skeletonRef), change);
                meshPtr->castShadows.Set(cast_shadows, change);

                // Create attribute descriptions for each component
                foreach(IAttribute *a, namePtr->Attributes())
                {
                    AttributeDesc attrDesc = { a->TypeName(), a->Name(), a->ToString().c_str() };
                    nameDesc.attributes.append(attrDesc);
                }
                foreach(IAttribute *a, meshPtr->Attributes())
                {
                    AttributeDesc attrDesc = { a->TypeName(), a->Name(), a->ToString().c_str() };
                    meshDesc.attributes.append(attrDesc);
                }
                foreach(IAttribute *a, placeablePtr->Attributes())
                {
                    AttributeDesc attrDesc = { a->TypeName(), a->Name(), a->ToString().c_str() };
                    placeableDesc.attributes.append(attrDesc);
                }

                entityDesc.components << nameDesc << meshDesc << placeableDesc;
                desc.entities << entityDesc;
            }
            else
                LogError("SceneImporter::ProcessNodeForDesc: Could not create mesh, placeable, name components");
        }

        // Process child nodes
        QDomElement childNodeElement = nodeElement.firstChildElement("node");
        if (!childNodeElement.isNull())
            ProcessNodeForDesc(desc, childNodeElement, newPos, newRot, newScale, prefix, flipyz, meshFiles, skeletonFiles, usedMaterials, nodeElement.attribute("name"));
    }
}


void SceneImporter::CreateAssetDescs(const QString &path, const QStringList &meshFiles, const QStringList &skeletons,
    const QStringList &materialFiles, const QSet<QString> &usedMaterials, SceneDesc &desc) const
{
    foreach(QString filename, meshFiles)
    {
        AssetDesc ad;
        ad.source = filename;
        ad.dataInMemory = false;
        ad.typeName = "OgreMesh";
        ad.destinationName = QFileInfo(filename).fileName();//meshAssetDesc.source;
        desc.assets[qMakePair(ad.source, ad.subname)] = ad;
    }

    foreach(QString skeleton, skeletons)
    {
        AssetDesc ad;
        ad.source = path + "/" + skeleton; // This is already an absolute path. No need to use ResolveLocalAssetPath.
        ad.dataInMemory = false;
        ad.typeName = "OgreSkeleton";
        ad.destinationName = skeleton;
        desc.assets[qMakePair(ad.source, ad.subname)] = ad;
    }

    // Get all materials scripts from all material script files.
    std::set<OgreRenderer::MaterialInfo> allMaterials;
    foreach(QString filename, materialFiles)
    {
        std::set<OgreRenderer::MaterialInfo> mats = OgreRenderer::LoadAllMaterialsFromFile(filename);
        allMaterials.insert(mats.begin(), mats.end());
    }

    // Find the used materials and create material assets descs even if the files don't exist.
    foreach(QString matName, usedMaterials)
    {
        AssetDesc ad;
        ad.typeName = "OgreMaterial";
        ad.subname = matName;
        ad.dataInMemory = true;
        ad.destinationName = matName + ".material";

        foreach(OgreRenderer::MaterialInfo mat, allMaterials)
            if (mat.name == matName)
            {
                ad.source = mat.source;
                ad.data = mat.data.toAscii();
            }

            desc.assets[qMakePair(ad.source, ad.subname)] = ad;
    }

    // Process materials for textures.
    QSet<QString> all_textures;
    foreach(OgreRenderer::MaterialInfo matInfo, allMaterials)
        if (usedMaterials.find(matInfo.name) != usedMaterials.end())
            all_textures.unite(OgreRenderer::ProcessMaterialForTextures(matInfo.data));

    // Add texture asset descs.
    foreach(QString tex, all_textures)
    {
        AssetDesc ad;
        ad.typeName = "Texture";
        ad.dataInMemory = false;
        AssetAPI::FileQueryResult result = scene_->GetFramework()->Asset()->ResolveLocalAssetPath(tex, path, ad.source);
        if (result == AssetAPI::FileQueryLocalFileMissing)
            LogWarning("Texture file \"" + tex + "\" cannot be found from path \"" + path + "\"!");
        ad.destinationName = AssetAPI::ExtractFilenameFromAssetRef(tex); // The destination name must be local to the destination asset storage.
        desc.assets[qMakePair(ad.source, ad.subname)] = ad;
    }
}

}
