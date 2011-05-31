// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "SceneImporter.h"
#include "TundraLogicModule.h"

#include "Scene.h"
#include "Entity.h"
#include "SceneDesc.h"
#include "OgreMaterialUtils.h"
#include "CoreStringUtils.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "EC_Name.h"
#include "Renderer.h"
#include "AssetAPI.h"
#include "LoggingFunctions.h"
#include "SceneAPI.h"
#include "OgreRenderingModule.h"
#include "CoreException.h"

#include <boost/filesystem.hpp>
#include <Ogre.h>

#include <QDomDocument>
#include <QFile>
#include <QDir>

namespace TundraLogic
{

SceneImporter::SceneImporter(const ScenePtr &scene) :
    scene_(scene)
{
}

SceneImporter::~SceneImporter()
{
}

EntityPtr SceneImporter::ImportMesh(const std::string& filename, std::string in_asset_dir, const Transform &worldtransform,
    const std::string& entity_prefab_xml, const QString &prefix, AttributeChange::Type change, bool inspect,
    const std::string &meshName)
{
    if (!scene_)
    {
        LogError("Null scene for mesh import");
        return EntityPtr();
    }

    boost::filesystem::path path(filename);
    std::string meshleafname = path.leaf();

    QStringList material_names;
    QString skeleton_name;
    if (inspect)
        if (!ParseMeshForMaterialsAndSkeleton(filename.c_str(), material_names, skeleton_name))
            return EntityPtr();

    QSet<QString> material_names_set;
    for(uint i = 0; i < (uint)material_names.size(); ++i)
    {
        LogDebug("Material ref: " + material_names[i].toStdString());
        material_names_set.insert(material_names[i]);
    }

    LogDebug("Skeleton ref: " + skeleton_name.toStdString());

    // Scan the asset dir for material files, because we don't actually know what material file the mesh refers to.
    QStringList material_files;
    if (inspect)
        material_files = GetMaterialFiles(in_asset_dir);

    // mesh copied, add mesh name inside the file
    if (!meshName.empty() /*&& createMesh*/)
        meshleafname += std::string("/") + meshName;

    // Create a new entity in any case, with a new ID
    EntityPtr newentity = scene_->CreateEntity(0, QStringList(), change, true);
    if (!newentity)
    {
        LogError("Could not create entity for mesh");
        return newentity;
    }
    
    // If the prefab contains valid data, instantiate the components from there
    QDomDocument prefab;
    prefab.setContent(QString::fromStdString(entity_prefab_xml));
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
    EC_Placeable* placeablePtr = checked_static_cast<EC_Placeable*>(newentity->GetOrCreateComponent(EC_Placeable::TypeNameStatic(), change).get());
    if (placeablePtr)
        placeablePtr->transform.Set(worldtransform, AttributeChange::Disconnected);
    else
        LogError("No EC_Placeable was created!");

    // Fill the mesh attributes
    AssetReferenceList materials;
    foreach(QString matName, material_names)
        materials.Append(AssetReference(prefix + matName + ".material"));

    EC_Mesh* meshPtr = checked_static_cast<EC_Mesh*>(newentity->GetOrCreateComponent(EC_Mesh::TypeNameStatic(), change).get());
    if (meshPtr)
    {
        meshPtr->meshRef.Set(AssetReference(prefix + QString(meshleafname.c_str())), AttributeChange::Disconnected);
        if (!skeleton_name.isEmpty())
            meshPtr->skeletonRef.Set(AssetReference(prefix + skeleton_name), AttributeChange::Disconnected);
        meshPtr->meshMaterial.Set(materials, AttributeChange::Disconnected);

        if (inspect)
            meshPtr->nodeTransformation.Set(Transform(Vector3df(0,0,0), Vector3df(90,0,180), Vector3df(1,1,1)), AttributeChange::Disconnected);
        else
            meshPtr->nodeTransformation.Set(Transform(), AttributeChange::Disconnected);
    }
    else
        LogError("No EC_Mesh was created!");

    // Fill the name attributes
    EC_Name * namePtr = checked_static_cast<EC_Name *>(newentity->GetOrCreateComponent(EC_Name::TypeNameStatic(), change).get());
    if (namePtr)
        ///\todo Use name of scenedesc?
        namePtr->name.Set(QString(meshleafname.c_str()).replace(".mesh", ""), AttributeChange::Disconnected);
    else
        LogError("No EC_Name was created!");

    // All components have been loaded/modified. Trigger change for them now.
    foreach(ComponentPtr c, newentity->Components())
        c->ComponentChanged(change);

    scene_->EmitEntityCreated(newentity, change);

    return newentity;
}

QList<Entity *> SceneImporter::Import(const std::string& filename, std::string in_asset_dir, const Transform &worldtransform,
    const QString &prefix, AttributeChange::Type change, bool clearscene, bool replace)
{
    QList<Entity *> ret;
    if (!scene_)
    {
        LogError("Null scene for import");
        return ret;
    }

    try
    {
        if (clearscene)
            LogInfo("Importing scene from " + filename + " and clearing the old");
        else
            LogInfo("Importing scene from " + filename);

        QFile file(filename.c_str());
        if (!file.open(QFile::ReadOnly))
        {
            file.close();
            LogError("Failed to open file");
            return QList<Entity *>();
        }
        
        QDomDocument dotscene;
        if (!dotscene.setContent(&file))
        {
            file.close();
            LogError("Failed to parse XML content");
            return ret;
        }
        
        file.close();
        
        QDomElement scene_elem = dotscene.firstChildElement("scene");
        if (scene_elem.isNull())
        {
            LogError("No scene element");
            return ret;
        }
        QDomElement nodes_elem = scene_elem.firstChildElement("nodes");
        if (nodes_elem.isNull())
        {
            LogError("No nodes element");
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
        LogInfo("Processing scene for assets");
        ProcessNodeForAssets(node_elem, in_asset_dir);
        
        // Write out the needed assets
        LogInfo("Saving needed assets");
        // By default, assume the material file is scenename.material if scene is scenename.scene.
        // However, if an external reference exists, use that.
        std::string matfilename = ReplaceSubstring(filename, ".scene", ".material");
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
                        matfilename = in_asset_dir + QString(QDir::separator()).toStdString() + file_elem.attribute("name").toStdString();
                        break;
                    }
                }
                item_elem = item_elem.nextSiblingElement();
            }
        }
        
//        ProcessAssets(matfilename, in_asset_dir, out_asset_dir, localassets);
        
        // Second pass: build scene hierarchy and actually create entities. This assumes assets are available
        LogInfo("Creating entities");

        Quaternion rot(DEGTORAD * worldtransform.rot.x, DEGTORAD * worldtransform.rot.y,
            DEGTORAD * worldtransform.rot.z);
        ProcessNodeForCreation(ret, node_elem, worldtransform.pos, rot, worldtransform.scale, change, prefix, flipyz, replace);
    }
    catch(Exception& e)
    {
        LogError(std::string("Exception while scene importing: ") + e.what());
        return QList<Entity *>();
    }
    
    LogInfo("Finished");

    return ret;
}

bool SceneImporter::ParseMeshForMaterialsAndSkeleton(const QString& meshname, QStringList & material_names, QString& skeleton_name) const
{
    material_names.clear();
    
    QFile mesh_in(meshname);
    if (!mesh_in.open(QFile::ReadOnly))
    {
        LogError("Could not open input mesh file " + meshname.toStdString());
        return false;
    }
    else
    {
        QByteArray mesh_bytes = mesh_in.readAll();
        mesh_in.close();
        OgreRenderer::RendererPtr renderer = scene_->GetFramework()->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
        if (!renderer)
        {
            LogError("Renderer does not exist");
            return false;
        }
        
        std::string uniquename = renderer->GetUniqueObjectName("SceneImport_tempmesh");
        try
        {
            Ogre::MeshPtr tempmesh = Ogre::MeshManager::getSingleton().createManual(uniquename, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            if (tempmesh.isNull())
            {
                LogError("Failed to create temp mesh");
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
            LogError("Exception while inspecting mesh " + meshname.toStdString());
            return false;
        }
    }
    
    return true;
}

SceneDesc SceneImporter::GetSceneDescForMesh(const QString &filename) const
{
    SceneDesc sceneDesc;

    if (!filename.endsWith(".mesh", Qt::CaseInsensitive))
    {
        LogError("Unsupported file type for scene description creation: " + filename.toStdString());
        return sceneDesc;
    }

    sceneDesc.filename = filename;

    QString path(boost::filesystem::path(filename.toStdString()).branch_path().string().c_str());
    QString meshleafname = boost::filesystem::path(filename.toStdString()).leaf().c_str();

    QStringList materialNames;
    QStringList skeletons;
    QString skeletonName;
    if (!ParseMeshForMaterialsAndSkeleton(filename, materialNames, skeletonName))
        return sceneDesc;

    if (!skeletonName.isEmpty())
        skeletons << skeletonName;

    // Construct entity name from the mesh file name.
    int idx = meshleafname.lastIndexOf(".mesh");
    QString meshEntityName = meshleafname;
    meshEntityName.remove(idx, meshleafname.length() - idx);

    EntityDesc entityDesc("", meshEntityName);
    ComponentDesc meshDesc = { EC_Mesh::TypeNameStatic() };
    ComponentDesc placeableDesc = { EC_Placeable::TypeNameStatic() };
    ComponentDesc nameDesc = { EC_Name::TypeNameStatic() };

    // Scan the asset dir for material files, because we don't actually know what material file the mesh refers to.
    QStringList meshFiles(QStringList() << filename);
    QSet<QString> usedMaterials = materialNames.toSet();
    QStringList materialFiles = GetMaterialFiles(path.toStdString());

    CreateAssetDescs(path, meshFiles, skeletons, materialFiles, usedMaterials, sceneDesc);

    // mesh copied, add mesh name inside the file
    //if (!meshName.empty())
    //    meshleafname += std::string("/") + meshName;

    // Create attribute descriptions.
    SceneAPI *sceneAPI = scene_->GetFramework()->Scene(); ///\todo Replace with scene_->SceneAPI();

    // Fill the mesh attributes
    AssetReferenceList materials;
    foreach(QString matName, materialNames)
        materials.Append(AssetReference(path + "/" + matName + ".material"));

    // Mesh  
    
    /// \todo This creates dummy components, specifying a null scene during creation
    
    boost::shared_ptr<EC_Mesh> mesh = sceneAPI->CreateComponent<EC_Mesh>(0);
    if (mesh)
    {
        mesh->meshRef.Set(AssetReference(path + "/" + meshleafname), AttributeChange::Disconnected);
        mesh->meshMaterial.Set(materials, AttributeChange::Disconnected);
        if (!skeletonName.isEmpty())
            mesh->skeletonRef.Set(AssetReference(path + "/" + skeletonName), AttributeChange::Disconnected);

        foreach(IAttribute *a, mesh->Attributes())
        {
            AttributeDesc attrDesc = { a->TypeName(), a->Name(), a->ToString().c_str() };
            meshDesc.attributes.append(attrDesc);
        }
    }

    boost::shared_ptr<EC_Placeable> placeable = sceneAPI->CreateComponent<EC_Placeable>(0);
    if (placeable)
        foreach(IAttribute *a, placeable->Attributes())
        {
            AttributeDesc attrDesc = { a->TypeName(), a->Name(), a->ToString().c_str() };
            placeableDesc.attributes.append(attrDesc);
        }

    // Name
    boost::shared_ptr<EC_Name> name = sceneAPI->CreateComponent<EC_Name>(0);
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

///\todo Bad code duplication. Remove the SceneImporter::GetSceneDescForMesh(QUrl) function altogether to avoid excessive code copy-paste. Have SceneImporter::GetSceneDescForMesh(QString) do the job of both. -jj.
///\todo Delete this function.
SceneDesc SceneImporter::GetSceneDescForMesh(const QUrl &meshUrl) const
{
    SceneDesc sceneDesc;

    if (!meshUrl.toString().endsWith(".mesh", Qt::CaseInsensitive))
    {
        LogError("Unsupported file type for scene description creation: " + meshUrl.toString().toStdString());
        return sceneDesc;
    }

    sceneDesc.filename = meshUrl.toString();

    // Construct entity name from the mesh file name.
    QString meshEntityName = meshUrl.toString();
    meshEntityName = meshEntityName.split("/").last();
    meshEntityName = meshEntityName.split(".mesh").first();

    EntityDesc entityDesc("", meshEntityName);
    ComponentDesc meshDesc = { EC_Mesh::TypeNameStatic() };
    ComponentDesc placeableDesc = { EC_Placeable::TypeNameStatic() };
    ComponentDesc nameDesc = { EC_Name::TypeNameStatic() };

    // Create asset description
    AssetDesc ad;
    ad.source = meshUrl.toString();
    ad.dataInMemory = false;
    ad.typeName = "mesh";
    ad.destinationName = "";
    sceneDesc.assets[qMakePair(ad.source, ad.subname)] = ad;
    
    // Create attribute descriptions
    SceneAPI *sceneAPI = scene_->GetFramework()->Scene();

    // Mesh
    boost::shared_ptr<EC_Mesh> mesh = sceneAPI->CreateComponent<EC_Mesh>(0);
    if (mesh)
    {
        mesh->meshRef.Set(AssetReference(meshUrl.toString()), AttributeChange::Disconnected);
        foreach(IAttribute *a, mesh->Attributes())
        {
            AttributeDesc attrDesc = { a->TypeName(), a->Name(), a->ToString().c_str() };
            meshDesc.attributes.append(attrDesc);
        }
    }

    // Placeable
    boost::shared_ptr<EC_Placeable> placeable = sceneAPI->CreateComponent<EC_Placeable>(0);
    if (placeable)
    {
        foreach(IAttribute *a, placeable->Attributes())
        {
            AttributeDesc attrDesc = { a->TypeName(), a->Name(), a->ToString().c_str() };
            placeableDesc.attributes.append(attrDesc);
        }
    }

    // Name
    boost::shared_ptr<EC_Name> name = sceneAPI->CreateComponent<EC_Name>(0);
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

SceneDesc SceneImporter::GetSceneDescForScene(const QString &filename)
{
    SceneDesc sceneDesc;

    if (!filename.endsWith(".scene", Qt::CaseInsensitive))
    {
        LogError("Unsupported file type for scene description creation: " + filename.toStdString());
        return sceneDesc;
    }

    sceneDesc.filename = filename;

    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
    {
        file.close();
        LogError("Failed to open file " + filename.toStdString());
        return sceneDesc;
    }

    QDomDocument dotscene;
    if (!dotscene.setContent(&file))
    {
        file.close();
        LogError("Failed to parse XML content");
        return sceneDesc;
    }
    
    file.close();
    
    QDomElement scene_elem = dotscene.firstChildElement("scene");
    if (scene_elem.isNull())
    {
        LogError("No scene element");
        return sceneDesc;
    }

    QDomElement nodes_elem = scene_elem.firstChildElement("nodes");
    if (nodes_elem.isNull())
    {
        LogError("No nodes element");
        return sceneDesc;
    }

    QString path(boost::filesystem::path(filename.toStdString()).branch_path().string().c_str());

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
    QStringList materialFiles(QStringList() << materialFileName);

//    Transform f;
//    Quaternion rot(DEGTORAD * f.rot.x, DEGTORAD * f.rot.y, DEGTORAD * f.rot.z);

    QDomElement node_elem = nodes_elem.firstChildElement("node");
//    ProcessNodeForDesc(sceneDesc, node_elem, f.position, rot, f.scale, path + "/"/*prefix*/, true/*flipyz*/);
    while(!node_elem.isNull())
    {
        // Process entity node, if any
        QDomElement entity_elem = node_elem.firstChildElement("entity");
        if (!entity_elem.isNull())
        {
            // Entity desc
            EntityDesc entityDesc;
            entityDesc.name = entity_elem.attribute("name");

            // Store the original name. Later we fix duplicates.
            QString mesh_name = entity_elem.attribute("meshFile");
            ComponentDesc compDesc;
            compDesc.typeName = EC_Mesh::TypeNameStatic();

            meshFiles << path + "/" + mesh_name; // TODO: This is a hardcoded assumption that the mesh_name ref was a local file. Might not hold. -jj.
            //AssetAPI::ResolveLocalAssetPath();

            /// Create dummy placeable desc.
            ComponentPtr placeable = scene_->GetFramework()->Scene()->CreateComponent<EC_Placeable>(0);
            if (placeable)
            {
                ComponentDesc placeableDesc;
                placeableDesc.typeName = EC_Placeable::TypeNameStatic();

                foreach(IAttribute *a, placeable->Attributes())
                {
                    AttributeDesc attrDesc = { a->TypeName(), a->Name(), a->ToString().c_str() };
                    placeableDesc.attributes.append(attrDesc);
                }
            }

            // Attribute desc for mesh asset reference.
            AttributeDesc meshAttrDesc = { "assetreference", "mesh", mesh_name };
            compDesc.attributes.append(meshAttrDesc);

            //mesh_names_[mesh_name] = mesh_name;
            QDomElement subentities_elem = entity_elem.firstChildElement("subentities");
            if (!subentities_elem.isNull())
            {
                QDomElement subentity_elem = subentities_elem.firstChildElement("subentity");
                while(!subentity_elem.isNull())
                {
                    QString material_name = subentity_elem.attribute("materialName");
                    usedMaterials.insert(material_name);

                    // Attribute desc for material asset reference.
                    AttributeDesc matAttrDesc = { "assetreference", "material", material_name };
                    compDesc.attributes.append(matAttrDesc);
                    //material_names_.insert(material_name);

                    subentity_elem = subentity_elem.nextSiblingElement("subentity");
                }
            }
            else
            {
                // If no subentity element, have to interrogate the mesh.
                QStringList material_names;
//                QSet<QString> material_names_set;
                QString skeleton_name;
                // TODO: This is a hardcoded assumption that the mesh_name ref was a local file. Might not hold. -jj.
                //AssetAPI::ResolveLocalAssetPath();
                ParseMeshForMaterialsAndSkeleton(path + "/" + mesh_name, material_names, skeleton_name);
                for(uint i = 0; i < (uint)material_names.size(); ++i)
                {
                    usedMaterials.insert(material_names[i]);
                    material_names_.insert(material_names[i].toStdString());
                }

                mesh_default_materials_[mesh_name] = material_names;

                if (!skeleton_name.isEmpty())
                    skeletons << skeleton_name;
            }

            entityDesc.components.append(compDesc);
            sceneDesc.entities.append(entityDesc);
        }

        // Process siblings
        node_elem = node_elem.nextSiblingElement("node");
    }

    CreateAssetDescs(path, meshFiles, skeletons, materialFiles, usedMaterials, sceneDesc);

//    foreach(AssetDesc ad, sceneDesc.assets)
//        RewriteAssetRef(sceneDesc.filename, ad.source);

    return sceneDesc;
}

QSet<QString> SceneImporter::ProcessMaterialFileForTextures(const QString& matfilename, const QSet<QString>& used_materials) const
{
    QSet<QString> used_textures;
    
    bool known_material = false;
    
    // Read the material file
    QFile matfile(matfilename);
    if (!matfile.open(QFile::ReadOnly))
    {
        LogError("Could not open material file " + matfilename.toStdString());
        return used_textures;
    }
    else
    {
        QByteArray bytes = matfile.readAll();
        matfile.close();

        if (bytes.size())
        {
            int num_materials = 0;
            int brace_level = 0;
            bool skip_until_next = false;
            int skip_brace_level = 0;
#include "DisableMemoryLeakCheck.h"
            Ogre::DataStreamPtr data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(bytes.data(), bytes.size()));
#include "EnableMemoryLeakCheck.h"
            
            while(!data->eof())
            {
                std::string line = data->getLine();
                
                // Skip empty lines & comments
                if ((line.length()) && (line.substr(0, 2) != "//"))
                {
                    // Process opening/closing braces
                    if (!OgreRenderer::ProcessBraces(line, brace_level))
                    {
                        // If not a brace and on level 0, it should be a new material
                        if ((brace_level == 0) && (line.substr(0, 8) == "material") && (line.length() > 8))
                        {
                            std::string matname = line.substr(9);
                            ReplaceCharInplace(matname, '/', '_');
                            line = "material " + matname;
                            if (used_materials.find(matname.c_str()) == used_materials.end())
                            {
                                known_material = false;
                            }
                            else
                            {
                                known_material = true;
                                ++num_materials;
                            }
                        }
                        else
                        {
                            // Check for textures
                            if (known_material)
                                if ((line.substr(0, 8) == "texture ") && (line.length() > 8))
                                    used_textures.insert(line.substr(8).c_str());
                        }
                    }
                    else
                    {
                        if (brace_level <= skip_brace_level)
                            skip_until_next = false;
                    }
                }
            }
        }
    }
    
    return used_textures;
}

QSet<QString> SceneImporter::ProcessMaterialForTextures(const QString &material) const
{
    QSet<QString> textures;
    QStringList lines = material.split("\n");
    for(int i = 0; i < lines.size(); ++i)
    {
        int idx = lines[i].indexOf("texture ");
        if (idx != -1)
            textures.insert(lines[i].mid(idx + 8).trimmed());
    }

    return textures;
}

QString SceneImporter::LoadSingleMaterialFromFile(const QString &filename, const QString &materialName) const
{
    QString material;

    bool right_material = false;

    // Read the material file
    QFile matfile(filename);
    if (!matfile.open(QFile::ReadOnly))
    {
        LogError("Could not open material file " + filename.toStdString());
        return material;
    }
    else
    {
        QByteArray bytes = matfile.readAll();
        matfile.close();
        if (bytes.size() == 0)
        {
            LogError("Empty material file: " + filename.toStdString());
            return material;
        }

        int brace_level = 0;
        bool skip_until_next = false;
        int skip_brace_level = 0;

#include "DisableMemoryLeakCheck.h"
        Ogre::DataStreamPtr data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(bytes.data(), bytes.size()));
#include "EnableMemoryLeakCheck.h"
        while(!data->eof())
        {
            std::string line = data->getLine();

            // Skip empty lines & comments
            if ((line.length()) && (line.substr(0, 2) != "//"))
            {
                // Process opening/closing braces
                if (!OgreRenderer::ProcessBraces(line, brace_level))
                {
                    // If not a brace and on level 0, it should be a new material
                    if ((brace_level == 0) && (line.substr(0, 8) == "material") && (line.length() > 8))
                    {
                        std::string matname = line.substr(9);
                        ReplaceCharInplace(matname, '/', '_');
                        line = "material " + matname;
                        if (matname.c_str() == materialName)
                            right_material = true;
                        else
                            right_material = false;
                    }

                    // Write line to the modified copy
                    if (!skip_until_next && right_material)
                    {
                        // Add indentation.
                        for(int i =0; i < brace_level; ++i)
                            material.append("    ");

                        material.append(line.c_str());
                        material.append("\n");
                    }
                }
                else
                {
                    // Write line to the modified copy
                    if (!skip_until_next && right_material)
                    {
                        // Add indentation.
                        int i = 0;
                        if (line.find("{") != std::string::npos)
                            ++i;
                        for(; i < brace_level; ++i)
                            material.append("    ");

                        material.append(line.c_str());
                        material.append("\n");
                    }

                    if (brace_level <= skip_brace_level)
                    {
                        skip_until_next = false;
                        ///\todo return material; here?
                    }
                }
            }
        }
    }

    return material;
}

MaterialInfoList SceneImporter::LoadAllMaterialsFromFile(const QString &filename) const
{
    MaterialInfoList materials;

    // Read the material file
    QFile matfile(filename);
    if (!matfile.open(QFile::ReadOnly))
    {
        LogError("Could not open material file " + filename.toStdString());
        return materials;
    }
    else
    {
        QByteArray bytes = matfile.readAll();
        matfile.close();
        if (bytes.size() == 0)
        {
            LogError("Empty material file: " + filename.toStdString());
            return materials;
        }

        int brace_level = 0;
        int skip_brace_level = 0;

        MaterialInfo material;
        material.source = filename;

#include "DisableMemoryLeakCheck.h"
        Ogre::DataStreamPtr data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(bytes.data(), bytes.size()));
#include "EnableMemoryLeakCheck.h"
        while(!data->eof())
        {
            std::string line = data->getLine();

            // Skip empty lines & comments
            if ((line.length()) && (line.substr(0, 2) != "//"))
            {
                // Process opening/closing braces
                if (!OgreRenderer::ProcessBraces(line, brace_level))
                {
                    // If not a brace and on level 0, it should be a new material
                    if ((brace_level == 0) && (line.substr(0, 8) == "material") && (line.length() > 8))
                    {
                        QString matname = line.substr(9).c_str();
                        matname.replace('/', '_');
                        material.name = matname.trimmed();
                        material.data.clear();
                    }

                    // Add indentation.
                    for(int i =0; i < brace_level; ++i)
                        material.data.append("    ");

                    material.data.append(line.c_str());
                    material.data.append("\n");
                }
                else
                {
                    // Add indentation.
                    int i = 0;
                    if (line.find("{") != std::string::npos)
                        ++i;
                    for(; i < brace_level; ++i)
                        material.data.append("    ");

                    material.data.append(line.c_str());
                    material.data.append("\n");

                    if (brace_level <= skip_brace_level)
                        materials.insert(material);
                }
            }
        }
    }

    return materials;
}

QStringList SceneImporter::GetMaterialFiles(const std::string &dir) const
{
    QStringList files;

    boost::filesystem::recursive_directory_iterator iter(dir), end_iter;
    for(; iter != end_iter; ++iter)
        if (boost::filesystem::is_regular_file(iter->status()))
        {
            QString ext(iter->path().extension().c_str());
            if (ext.contains(".material", Qt::CaseInsensitive))
                files.push_back(iter->path().string().c_str());
        }

    return files;
}

void SceneImporter::ProcessNodeForAssets(QDomElement node_elem, const std::string& in_asset_dir)
{
    while(!node_elem.isNull())
    {
        // Process entity node, if any
        QDomElement entity_elem = node_elem.firstChildElement("entity");
        if (!entity_elem.isNull())
        {
            std::string mesh_name = entity_elem.attribute("meshFile").toStdString();
            // Store the original name. Later we fix duplicates.
            mesh_names_[mesh_name] = mesh_name;
            QDomElement subentities_elem = entity_elem.firstChildElement("subentities");
            if (!subentities_elem.isNull())
            {
                QDomElement subentity_elem = subentities_elem.firstChildElement("subentity");
                while(!subentity_elem.isNull())
                {
                    std::string material_name = subentity_elem.attribute("materialName").toStdString();
                    material_names_.insert(material_name);
                    subentity_elem = subentity_elem.nextSiblingElement("subentity");
                }
            }
            else
            {
                // If no subentity element, have to interrogate the mesh.
                QStringList material_names;
                QString skeleton_name;
                ParseMeshForMaterialsAndSkeleton(QString::fromStdString(in_asset_dir + "/" + mesh_name), material_names, skeleton_name);
                for(uint i = 0; i < (uint)material_names.size(); ++i)
                    material_names_.insert(material_names[i].toStdString());
                mesh_default_materials_[mesh_name.c_str()] = material_names;
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

void SceneImporter::ProcessNodeForCreation(QList<Entity* > &entities, QDomElement node_elem, Vector3df pos, Quaternion rot, Vector3df scale,
    AttributeChange::Type change, const QString &prefix, bool flipyz, bool replace)
{
    while(!node_elem.isNull())
    {
        QDomElement pos_elem = node_elem.firstChildElement("position");
        QDomElement rot_elem = node_elem.firstChildElement("rotation");
        QDomElement quat_elem = node_elem.firstChildElement("quaternion");
        QDomElement scale_elem = node_elem.firstChildElement("scale");
        float posx, posy, posz, rotx = 0.0f, roty = 0.0f, rotz = 0.0f, rotw = 1.0f, scalex, scaley, scalez;

        posx = ParseString<float>(pos_elem.attribute("x").toStdString(), 0.0f);
        posy = ParseString<float>(pos_elem.attribute("y").toStdString(), 0.0f);
        posz = ParseString<float>(pos_elem.attribute("z").toStdString(), 0.0f);

        if (!rot_elem.isNull())
        {
            rotx = ParseString<float>(rot_elem.attribute("qx").toStdString(), 0.0f);
            roty = ParseString<float>(rot_elem.attribute("qy").toStdString(), 0.0f);
            rotz = ParseString<float>(rot_elem.attribute("qz").toStdString(), 0.0f);
            rotw = ParseString<float>(rot_elem.attribute("qw").toStdString(), 1.0f);
        }
        if (!quat_elem.isNull())
        {
            rotx = ParseString<float>(quat_elem.attribute("x").toStdString(), 0.0f);
            roty = ParseString<float>(quat_elem.attribute("y").toStdString(), 0.0f);
            rotz = ParseString<float>(quat_elem.attribute("z").toStdString(), 0.0f);
            rotw = ParseString<float>(quat_elem.attribute("w").toStdString(), 1.0f);
        }

        scalex = ParseString<float>(scale_elem.attribute("x").toStdString(), 1.0f);
        scaley = ParseString<float>(scale_elem.attribute("y").toStdString(), 1.0f);
        scalez = ParseString<float>(scale_elem.attribute("z").toStdString(), 1.0f);

        Vector3df newpos(posx, posy, posz);
        Quaternion newrot(rotx, roty, rotz, rotw);
        Vector3df newscale(fabsf(scalex), fabsf(scaley), fabsf(scalez));

        // Transform by the parent transform
        newrot = rot * newrot;
        newscale = scale * newscale;
        newpos = rot * (scale * newpos);
        newpos += pos;

        // Process entity node, if any
        QDomElement entity_elem = node_elem.firstChildElement("entity");
        if (!entity_elem.isNull())
        {
            // Enforce uniqueness for node names, which may not be guaranteed by artists
            std::string base_node_name = node_elem.attribute("name").toStdString();
            if (base_node_name.empty())
                base_node_name = "object";
            std::string node_name = base_node_name;
            int append_num = 1;
            while(node_names_.find(node_name) != node_names_.end())
            {
                node_name = base_node_name + "_" + ToString<int>(append_num);
                ++append_num;
            }
            node_names_.insert(node_name);
            
            // Get mesh name from map
            std::string orig_mesh_name = entity_elem.attribute("meshFile").toStdString();
            QString mesh_name = QString::fromStdString(mesh_names_[orig_mesh_name]);
            
            bool cast_shadows = ::ParseBool(entity_elem.attribute("castShadows").toStdString());

            mesh_name = prefix + mesh_name;

            EntityPtr entity;
            bool new_entity = false;
            QString node_name_qstr = QString::fromStdString(node_name);

            // Try to find existing entity by name
            if (replace)
                entity = scene_->GetEntityByName(node_name_qstr);

            if (!entity)
            {
                entity = scene_->CreateEntity(scene_->GetNextFreeId());
                new_entity = true;
            }
            else
            {
                LogInfo("Updating existing entity " + node_name);
            }

            EC_Mesh* meshPtr = 0;
            EC_Name* namePtr = 0;
            EC_Placeable* placeablePtr = 0;

            if (entity)
            {
                meshPtr = checked_static_cast<EC_Mesh*>(entity->GetOrCreateComponent(EC_Mesh::TypeNameStatic(), change).get());
                namePtr = checked_static_cast<EC_Name*>(entity->GetOrCreateComponent(EC_Name::TypeNameStatic(), change).get());
                placeablePtr = checked_static_cast<EC_Placeable*>(entity->GetOrCreateComponent(EC_Placeable::TypeNameStatic(), change).get());
                
                if ((meshPtr) && (namePtr) && (placeablePtr))
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
                            
                            int index = ParseString<int>(subentity_elem.attribute("index").toStdString());
                            
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
                        const QStringList& default_materials = mesh_default_materials_[orig_mesh_name.c_str()];
                        materials.resize(default_materials.size());
                        for(uint i = 0; i < (uint)default_materials.size(); ++i)
                            materials[i] =  prefix + default_materials[i] + ".material";
                    }
                    
                    Transform entity_transform;
                    
                    /// Todo: allow any transformation of coordinate axes, not just fixed y/z flip
                    if (flipyz)
                    {
                        Vector3df rot_euler;
                        Quaternion adjustedrot(-newrot.x, newrot.z, newrot.y, newrot.w);
                        adjustedrot = Quaternion(0, PI, 0) * adjustedrot;
                        adjustedrot.toEuler(rot_euler);
                        entity_transform.SetPos(-newpos.x, newpos.z, newpos.y);
                        entity_transform.SetRot(rot_euler.x * RADTODEG, rot_euler.y * RADTODEG, rot_euler.z * RADTODEG);
                        entity_transform.SetScale(newscale.x, newscale.z, newscale.y);
                    }
                    else
                    {
                        Vector3df rot_euler;
                        newrot.toEuler(rot_euler);
                        entity_transform.SetPos(newpos.x, newpos.y, newpos.z);
                        entity_transform.SetRot(rot_euler.x * RADTODEG, rot_euler.y * RADTODEG, rot_euler.z * RADTODEG);
                        entity_transform.SetScale(newscale.x, newscale.y, newscale.z);
                    }
                    
                    
                    placeablePtr->transform.Set(entity_transform, change);
                    meshPtr->meshRef.Set(AssetReference(mesh_name), change);

                    AssetReferenceList materialRefs;
                    foreach(QString material, materials)
                        materialRefs.Append(AssetReference(material));

                    meshPtr->meshMaterial.Set(materialRefs, change);
                    meshPtr->castShadows.Set(cast_shadows, change);

                    if (new_entity)
                        scene_->EmitEntityCreated(entity, change);
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

/*
void SceneImporter::ProcessNodeForDesc(SceneDesc &desc, QDomElement node_elem, Vector3df pos, Quaternion rot, Vector3df scale,
    const QString &prefix, bool flipyz)
{
    AttributeChange::Type change = AttributeChange::Disconnected;
    while(!node_elem.isNull())
    {
        QDomElement pos_elem = node_elem.firstChildElement("position");
        QDomElement rot_elem = node_elem.firstChildElement("rotation");
        QDomElement quat_elem = node_elem.firstChildElement("quaternion");
        QDomElement scale_elem = node_elem.firstChildElement("scale");
        float posx, posy, posz, rotx = 0.0f, roty = 0.0f, rotz = 0.0f, rotw = 1.0f, scalex, scaley, scalez;

        posx = ParseString<float>(pos_elem.attribute("x").toStdString(), 0.0f);
        posy = ParseString<float>(pos_elem.attribute("y").toStdString(), 0.0f);
        posz = ParseString<float>(pos_elem.attribute("z").toStdString(), 0.0f);

        if (!rot_elem.isNull())
        {
            rotx = ParseString<float>(rot_elem.attribute("qx").toStdString(), 0.0f);
            roty = ParseString<float>(rot_elem.attribute("qy").toStdString(), 0.0f);
            rotz = ParseString<float>(rot_elem.attribute("qz").toStdString(), 0.0f);
            rotw = ParseString<float>(rot_elem.attribute("qw").toStdString(), 1.0f);
        }
        if (!quat_elem.isNull())
        {
            rotx = ParseString<float>(quat_elem.attribute("x").toStdString(), 0.0f);
            roty = ParseString<float>(quat_elem.attribute("y").toStdString(), 0.0f);
            rotz = ParseString<float>(quat_elem.attribute("z").toStdString(), 0.0f);
            rotw = ParseString<float>(quat_elem.attribute("w").toStdString(), 1.0f);
        }

        scalex = ParseString<float>(scale_elem.attribute("x").toStdString(), 1.0f);
        scaley = ParseString<float>(scale_elem.attribute("y").toStdString(), 1.0f);
        scalez = ParseString<float>(scale_elem.attribute("z").toStdString(), 1.0f);

        Vector3df newpos(posx, posy, posz);
        Quaternion newrot(rotx, roty, rotz, rotw);
        Vector3df newscale(fabsf(scalex), fabsf(scaley), fabsf(scalez));

        // Transform by the parent transform
        newrot = rot * newrot;
        newscale = scale * newscale;
        newpos = rot * (scale * newpos);
        newpos += pos;

        // Process entity node, if any
        QDomElement entity_elem = node_elem.firstChildElement("entity");
        if (!entity_elem.isNull())
        {
            EntityDesc ed;
            ComponentDesc meshDesc = { EC_Mesh::TypeNameStatic() };
            ComponentDesc placeableDesc = { EC_Placeable::TypeNameStatic() };
            ComponentDesc nameDesc = { EC_Name::TypeNameStatic() };

            // Enforce uniqueness for node names, which may not be guaranteed by artists
            std::string base_node_name = node_elem.attribute("name").toStdString();
            if (base_node_name.empty())
                base_node_name = "object";
            std::string node_name = base_node_name;
            int append_num = 1;
            while(node_names_.find(node_name) != node_names_.end())
            {
                node_name = base_node_name + "_" + ToString<int>(append_num);
                ++append_num;
            }
            node_names_.insert(node_name);
            
            // Get mesh name from map
            std::string orig_mesh_name = entity_elem.attribute("meshFile").toStdString();
            QString mesh_name = QString::fromStdString(mesh_names_[orig_mesh_name]);
            
            bool cast_shadows = ::ParseBool(entity_elem.attribute("castShadows").toStdString());

            mesh_name = prefix + mesh_name;

            QString node_name_qstr = QString::fromStdString(node_name);

            ed.name = node_name_qstr;
            // Try to find existing entity by name
            //if (replace)
                //entity = scene_->GetEntity(node_name_qstr);
            //if (!entity)
            //{
            //    entity = scene_->CreateEntity(scene_->GetNextFreeId());
            //    new_entity = true;
            //}
            //else
            //{
            //    LogInfo("Updating existing entity " + node_name);
            //}

            ComponentManagerPtr mgr = scene_->GetFramework()->GetComponentManager();

            ComponentPtr meshComp = mgr->CreateComponent(EC_Mesh::TypeNameStatic());
            ComponentPtr nameComp = mgr->CreateComponent(EC_Mesh::TypeNameStatic());
            ComponentPtr placeableComp = mgr->CreateComponent(EC_Mesh::TypeNameStatic());

            EC_Mesh* meshPtr = checked_static_cast<EC_Mesh*>(meshComp.get());
            EC_Name* namePtr = checked_static_cast<EC_Name*>(nameComp.get());
            EC_Placeable* placeablePtr = checked_static_cast<EC_Placeable*>(placeableComp.get());
            if ((meshPtr) && (namePtr) && (placeablePtr))
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
                        
                        int index = ParseString<int>(subentity_elem.attribute("index").toStdString());
                        
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
                    const QStringList& default_materials = mesh_default_materials_[orig_mesh_name.c_str()];
                    materials.resize(default_materials.size());
                    for(uint i = 0; i < default_materials.size(); ++i)
                        materials[i] =  prefix + default_materials[i] + ".material";
                }
                
                Transform entity_transform;
                
                if (flipyz)
                {
                    Vector3df rot_euler;
                    Quaternion adjustedrot(-newrot.x, newrot.z, newrot.y, newrot.w);
                    adjustedrot.toEuler(rot_euler);
                    entity_transform.SetPos(-newpos.x, newpos.z, newpos.y);
                    entity_transform.SetRot(rot_euler.x * RADTODEG, rot_euler.y * RADTODEG, rot_euler.z * RADTODEG);
                    entity_transform.SetScale(newscale.x, newscale.z, newscale.y);
                }
                else
                {
                    Vector3df rot_euler;
                    newrot.toEuler(rot_euler);
                    entity_transform.SetPos(newpos.x, newpos.y, newpos.z);
                    entity_transform.SetRot(rot_euler.x * RADTODEG, rot_euler.y * RADTODEG, rot_euler.z * RADTODEG);
                    entity_transform.SetScale(newscale.x, newscale.y, newscale.z);
                }
                
                placeablePtr->transform.Set(entity_transform, change);
                meshPtr->meshRef.Set(AssetReference(mesh_name), change);

                AssetReferenceList materialRefs;
                foreach(QString material, materials)
                    materialRefs.Append(AssetReference(material));

                meshPtr->meshMaterial.Set(materialRefs, change);
                meshPtr->castShadows.Set(cast_shadows, change);

                placeablePtr->ComponentChanged(change);
                meshPtr->ComponentChanged(change);
                namePtr->ComponentChanged(change);

                // Create attribute descriptions for each component
                foreach(IAttribute *a, namePtr->Attributes())
                {
                    AttributeDesc attrDesc = { a->TypeName(), a->GetNameString().c_str(), a->ToString().c_str() };
                    nameDesc.attributes.append(attrDesc);
                }
                foreach(IAttribute *a, meshPtr->Attributes())
                {
                    AttributeDesc attrDesc = { a->TypeName(), a->GetNameString().c_str(), a->ToString().c_str() };
                    meshDesc.attributes.append(attrDesc);
                }
                foreach(IAttribute *a, placeablePtr->Attributes())
                {
                    AttributeDesc attrDesc = { a->TypeName(), a->GetNameString().c_str(), a->ToString().c_str() };
                    placeableDesc.attributes.append(attrDesc);
                }

                ed.components << nameDesc << meshDesc << placeableDesc;
                desc.entities << ed;
            }
            else
                LogError("Could not create mesh, placeable, name components");
        }

        // Process child nodes
        QDomElement childnode_elem = node_elem.firstChildElement("node");
        if (!childnode_elem.isNull())
            ProcessNodeForDesc(desc, childnode_elem, newpos, newrot, newscale, prefix, flipyz);

        // Process siblings
        node_elem = node_elem.nextSiblingElement("node");
    }
}
*/

void SceneImporter::CreateAssetDescs(const QString &path, const QStringList &meshFiles, const QStringList &skeletons,
    const QStringList &materialFiles, const QSet<QString> &usedMaterials, SceneDesc &desc) const
{
    foreach(QString filename, meshFiles)
    {
        AssetDesc ad;
        ad.source = filename;
        ad.dataInMemory = false;
        ad.typeName = "mesh";
        ad.destinationName = boost::filesystem::path(filename.toStdString()).leaf().c_str();//meshAssetDesc.source;
        desc.assets[qMakePair(ad.source, ad.subname)] = ad;
    }

    foreach(QString skeleton, skeletons)
    {
        AssetDesc ad;
        ad.source = path + "/" + skeleton; // This is already an absolute path. No need to use ResolveLocalAssetPath.
        ad.dataInMemory = false;
        ad.typeName = "skeleton";
        ad.destinationName = skeleton;
        desc.assets[qMakePair(ad.source, ad.subname)] = ad;
    }

    // Get all materials scripts from all material script files.
    MaterialInfoList allMaterials;
    foreach(QString filename, materialFiles)
    {
        MaterialInfoList mats = LoadAllMaterialsFromFile(filename);
        allMaterials.insert(mats.begin(), mats.end());
    }

    // Find the used materials and create material assets descs even if the files don't exist.
    foreach(QString matName, usedMaterials)
    {
        AssetDesc ad;
        ad.typeName = "material";
        ad.subname = matName;
        ad.dataInMemory = true;
        ad.destinationName = matName + ".material";

        foreach(MaterialInfo mat, allMaterials)
            if (mat.name == matName)
            {
                ad.source = mat.source;
                ad.data = mat.data.toAscii();
            }

            desc.assets[qMakePair(ad.source, ad.subname)] = ad;
    }

    // Process materials for textures.
    QSet<QString> all_textures;
    foreach(MaterialInfo matInfo, allMaterials)
        if (usedMaterials.find(matInfo.name) != usedMaterials.end())
            all_textures.unite(ProcessMaterialForTextures(matInfo.data));

    // Add texture asset descs.
    foreach(QString tex, all_textures)
    {
        AssetDesc ad;
        ad.typeName = "texture";
        ad.dataInMemory = false;
        AssetAPI::FileQueryResult result = scene_->GetFramework()->Asset()->ResolveLocalAssetPath(tex, path, ad.source);
        if (result == AssetAPI::FileQueryLocalFileMissing)
            LogWarning("Texture file \"" + tex.toStdString() + "\" cannot be found from path \"" + path.toStdString() + "\"!");
        ad.destinationName = AssetAPI::ExtractFilenameFromAssetRef(tex); // The destination name must be local to the destination asset storage.
        desc.assets[qMakePair(ad.source, ad.subname)] = ad;
    }
}

/*
void SceneImporter::RewriteAssetRef(const QString &sceneFileName, QString &ref) const
{
    QString basePath(boost::filesystem::path(sceneFileName.toStdString()).branch_path().string().c_str());
    QString outFilePath;
    AssetAPI::FileQueryResult res = scene_->GetFramework()->Asset()->ResolveLocalAssetPath(ref, basePath, outFilePath);
    if (res == AssetAPI::FileQueryLocalFileFound)
        ref = outFilePath;
}
*/

}

