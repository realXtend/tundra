// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CoreStringUtils.h"
#include "RexNetworkUtils.h"
#include "DebugOperatorNew.h"
#include "SceneImporter.h"
#include "TundraLogicModule.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "EC_Name.h"
#include "ServiceManager.h"
#include "Renderer.h"

#include <Ogre.h>

#include <QDomDocument>
#include <QFile>

using namespace RexTypes;

namespace fs = boost::filesystem;

namespace TundraLogic
{

struct MeshInfo
{
    std::string name_;
    unsigned filesize_;
};

bool ProcessBraces(const std::string& line, int& braceLevel)
{
    if (line == "{")
    {
        ++braceLevel;
        return true;
    } 
    else if (line == "}")
    {
        --braceLevel;
        return true;
    }
    else return false;
}

SceneImporter::SceneImporter(Foundation::Framework* framework) :
    framework_(framework)
{
}

SceneImporter::~SceneImporter()
{
}

Scene::EntityPtr SceneImporter::ImportMesh(Scene::ScenePtr scene, const std::string& meshname, std::string in_asset_dir, std::string out_asset_dir,
    Transform worldtransform, const std::string& entity_prefab_xml, AttributeChange::Type change, bool localassets)
{
    if (!scene)
    {
        TundraLogicModule::LogError("Null scene for mesh import");
        return Scene::EntityPtr();
    }
    
    std::string prefix;
    if (localassets)
        prefix = "file://";
    
    if (!in_asset_dir.empty())
    {
        char lastchar = in_asset_dir[in_asset_dir.length() - 1];
        if ((lastchar != '/') && (lastchar != '\\'))
            in_asset_dir += '/';
    }
    if (!out_asset_dir.empty())
    {
        char lastchar = out_asset_dir[out_asset_dir.length() - 1];
        if ((lastchar != '/') && (lastchar != '\\'))
            out_asset_dir += '/';
    }
    
    boost::filesystem::path path(meshname);
    std::string meshleafname = path.leaf();
    
    std::vector<std::string> material_names;
    std::set<std::string> material_names_set;
    std::string skeleton_name;
    
    QFile mesh_in((meshname).c_str());
    if (!mesh_in.open(QFile::ReadOnly))
    {
        TundraLogicModule::LogError("Could not open input mesh file " + meshname);
        return Scene::EntityPtr();
    }
    else
    {
        QByteArray mesh_bytes = mesh_in.readAll();
        mesh_in.close();
        // Need to actually parse the mesh to get skeleton link and materials used
        boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer)
        {
            TundraLogicModule::LogError("Renderer does not exist");
            return Scene::EntityPtr();
        }
        
        std::string uniquename = renderer->GetUniqueObjectName();
        try
        {
            Ogre::MeshPtr tempmesh = Ogre::MeshManager::getSingleton().createManual(uniquename, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            if (tempmesh.isNull())
            {
                TundraLogicModule::LogError("Failed to create temp mesh");
                return Scene::EntityPtr();
            }
            
            Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)mesh_bytes.data(), mesh_bytes.size(), false));
            Ogre::MeshSerializer serializer;
            serializer.importMesh(stream, tempmesh.getPointer());
            
            for (uint i = 0; i < tempmesh->getNumSubMeshes(); ++i)
            {
                Ogre::SubMesh* submesh = tempmesh->getSubMesh(i);
                if (submesh)
                {
                    material_names.push_back(submesh->getMaterialName());
                    material_names_set.insert(submesh->getMaterialName());
                }
            }
            skeleton_name = tempmesh->getSkeletonName();
            
            // Now delete the mesh as we're done with inspecting it
            tempmesh.setNull();
            Ogre::MeshManager::getSingleton().remove(uniquename);
        }
        catch (...)
        {
            TundraLogicModule::LogError("Exception while inspecting the mesh " + meshname);
            return Scene::EntityPtr();
        }
    }
    
    for (uint i = 0; i < material_names.size(); ++i)
        TundraLogicModule::LogInfo("Material ref: " + material_names[i]);
    TundraLogicModule::LogInfo("Skeleton ref: " + skeleton_name);
    
    // Scan the asset dir for material files, because we don't actually know what material file the mesh refers to.
    std::vector<std::string> material_files;
    fs::directory_iterator iter(in_asset_dir);
    fs::directory_iterator end_iter;
    for(; iter != end_iter; ++iter )
    {
        if (fs::is_regular_file(iter->status()))
        {
            std::string ext = iter->path().extension();
            boost::algorithm::to_lower(ext);
            if (ext == ".material")
            {
                TundraLogicModule::LogInfo("Material file: " + iter->path().string());
                material_files.push_back(iter->path().string());
            }
        }
    }
    
    // Process materials
    std::set<std::string> all_textures;
    for (uint i = 0; i < material_files.size(); ++i)
    {
        std::set<std::string> textures;
        textures = ProcessMaterialFile(material_files[i], material_names_set, out_asset_dir, localassets);
        all_textures.insert(textures.begin(), textures.end());
    }
    
    // Process textures
    ProcessTextures(all_textures, in_asset_dir, out_asset_dir);
    
    // Copy mesh and skeleton
    CopyAsset(meshleafname, in_asset_dir, out_asset_dir);
    if (!skeleton_name.empty())
        CopyAsset(skeleton_name, in_asset_dir, out_asset_dir);
    
    // Create a new entity in any case, with a new ID
    Scene::EntityPtr newentity = scene->CreateEntity(0, QStringList(), change, true);
    if (!newentity)
    {
        TundraLogicModule::LogError("Could not create entity for mesh");
        return newentity;
    }
    
    // If the prefab contains valid data, instantiate the components from there
    QDomDocument prefab;
    prefab.setContent(QString::fromStdString(entity_prefab_xml));
    QDomElement ent_elem = prefab.firstChildElement("entity");
    if (!ent_elem.isNull())
    {
        //! \todo this should possibly be a function in SceneManager
        QDomElement comp_elem = ent_elem.firstChildElement("component");
        while (!comp_elem.isNull())
        {
            QString type_name = comp_elem.attribute("type");
            QString name = comp_elem.attribute("name");
            ComponentPtr newcomp = newentity->GetOrCreateComponent(type_name, name);
            if (newcomp)
                // Trigger no signal yet when entity is in incoherent state
                newcomp->DeserializeFrom(comp_elem, AttributeChange::Disconnected);
            comp_elem = comp_elem.nextSiblingElement("component");
        }
        scene->EmitEntityCreated(newentity, change);
    }
    
    // Fill the placeable attributes
    EC_Placeable* placeablePtr = checked_static_cast<EC_Placeable*>(newentity->GetOrCreateComponent(EC_Placeable::TypeNameStatic(), change).get());
    if (placeablePtr)
        placeablePtr->transform.Set(worldtransform, AttributeChange::Disconnected);
    else
        TundraLogicModule::LogError("No EC_Placeable was created!");
    // Fill the mesh attributes
    QVector<QVariant> materials;
    for (uint i = 0; i < material_names.size(); ++i)
        materials.push_back(QString::fromStdString(prefix + material_names[i] + ".material"));
    EC_Mesh* meshPtr = checked_static_cast<EC_Mesh*>(newentity->GetOrCreateComponent(EC_Mesh::TypeNameStatic(), change).get());
    if (meshPtr)
    {
        meshPtr->meshResourceId.Set(QString::fromStdString(prefix + meshleafname), AttributeChange::Disconnected);
        if (!skeleton_name.empty())
            meshPtr->skeletonId.Set(QString::fromStdString(prefix + skeleton_name), AttributeChange::Disconnected);
        meshPtr->meshMaterial.Set(QList<QVariant>::fromVector(materials), AttributeChange::Disconnected);
        meshPtr->nodeTransformation.Set(Transform(Vector3df(0,0,0), Vector3df(90,0,180), Vector3df(1,1,1)), AttributeChange::Disconnected);
    }
    else
        TundraLogicModule::LogError("No EC_Mesh was created!");
    
    // All components have been loaded/modified. Trigger change for them now.
    const Scene::Entity::ComponentVector &components = newentity->GetComponentVector();
    for(uint i = 0; i < components.size(); ++i)
        components[i]->ComponentChanged(change);
    
    return newentity;
}

bool SceneImporter::Import(Scene::ScenePtr scene, const std::string& filename, std::string in_asset_dir, std::string out_asset_dir,
    AttributeChange::Type change, bool clearscene, bool localassets, bool replace)
{
    if (!scene)
    {
        TundraLogicModule::LogError("Null scene for import");
        return false;
    }
    
    try
    {
        if (clearscene)
            TundraLogicModule::LogInfo("Importing scene from " + filename + " and clearing the old");
        else
            TundraLogicModule::LogInfo("Importing scene from " + filename);
        
        // Create output asset path if does not exist
        if (boost::filesystem::exists(out_asset_dir) == false)
            boost::filesystem::create_directory(out_asset_dir);
        
        if (!in_asset_dir.empty())
        {
            char lastchar = in_asset_dir[in_asset_dir.length() - 1];
            if ((lastchar != '/') && (lastchar != '\\'))
                in_asset_dir += '/';
        }
        if (!out_asset_dir.empty())
        {
            char lastchar = out_asset_dir[out_asset_dir.length() - 1];
            if ((lastchar != '/') && (lastchar != '\\'))
                out_asset_dir += '/';
        }
        
        QFile file(filename.c_str());
        if (!file.open(QFile::ReadOnly))
        {
            file.close();
            TundraLogicModule::LogError("Failed to open file");
            return false;
        }
        
        QDomDocument dotscene;
        if (!dotscene.setContent(&file))
        {
            file.close();
            TundraLogicModule::LogError("Failed to parse XML content");
            return false;
        }
        
        file.close();
        
        QDomElement scene_elem = dotscene.firstChildElement("scene");
        if (scene_elem.isNull())
        {
            TundraLogicModule::LogError("No scene element");
            return false;
        }
        QDomElement nodes_elem = scene_elem.firstChildElement("nodes");
        if (nodes_elem.isNull())
        {
            TundraLogicModule::LogError("No nodes element");
            return false;
        }
        
        bool flipyz = false;
        QString upaxis = scene_elem.attribute("upAxis");
        if (upaxis == "y")
            flipyz = true;
        
        if (clearscene)
            scene->RemoveAllEntities(true, change);
        
        QDomElement node_elem = nodes_elem.firstChildElement("node");
        
        // First pass: get used assets
        TundraLogicModule::LogInfo("Processing scene for assets");
        ProcessNodeForAssets(node_elem);
        
        // Write out the needed assets
        TundraLogicModule::LogInfo("Saving needed assets");
        ProcessAssets(filename, in_asset_dir, out_asset_dir, localassets);
        
        // Second pass: build scene hierarchy and actually create entities. This assumes assets are available
        TundraLogicModule::LogInfo("Creating entities");
        ProcessNodeForCreation(scene, node_elem, Vector3df(0.0f, 0.0f, 0.0f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Vector3df(1.0f, 1.0f, 1.0f), change, localassets, flipyz, replace);
        
    }
    catch (Exception& e)
    {
        TundraLogicModule::LogError(std::string("Exception while scene importing: ") + e.what());
        return false;
    }
    
    TundraLogicModule::LogInfo("Finished");
    return true;
}

void SceneImporter::ProcessNodeForAssets(QDomElement node_elem)
{
    while (!node_elem.isNull())
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
                while (!subentity_elem.isNull())
                {
                    std::string material_name = subentity_elem.attribute("materialName").toStdString();
                    material_names_.insert(material_name);
                    subentity_elem = subentity_elem.nextSiblingElement("subentity");
                }
            }
        }
        
        // Process child nodes
        QDomElement childnode_elem = node_elem.firstChildElement("node");
        if (!childnode_elem.isNull())
            ProcessNodeForAssets(childnode_elem);
        
        // Process siblings
        node_elem = node_elem.nextSiblingElement("node");
    }
}

void SceneImporter::ProcessAssets(const std::string& filename, const std::string& in_asset_dir, const std::string& out_asset_dir, bool localassets)
{
    std::string matfilename = ReplaceSubstring(filename, ".scene", ".material");
    
    std::set<std::string> used_textures = ProcessMaterialFile(matfilename, material_names_, out_asset_dir, localassets);
    ProcessTextures(used_textures, in_asset_dir, out_asset_dir);
    
    // Copy meshes
    std::vector<MeshInfo> created_meshes;
    std::map<std::string, std::string>::iterator m = mesh_names_.begin();
    while (m != mesh_names_.end())
    {
        std::string meshname = m->first;
        QFile mesh_in((in_asset_dir + meshname).c_str());
        if (!mesh_in.open(QFile::ReadOnly))
            TundraLogicModule::LogError("Could not open input mesh file " + meshname);
        else
        {
            QByteArray bytes = mesh_in.readAll();
            mesh_in.close();
            
            // Loop through all already created meshes and check for duplicate
            bool duplicate_found = false;
            for (unsigned j = 0; j < created_meshes.size(); ++j)
            {
                if (created_meshes[j].filesize_ == bytes.size())
                {
                    QFile mesh_compare((out_asset_dir + created_meshes[j].name_).c_str());
                    if (!mesh_compare.open(QFile::ReadOnly))
                        continue;
                    QByteArray compare_bytes = mesh_compare.readAll();
                    mesh_compare.close();
                    if (compare_bytes.size() != bytes.size())
                        continue;
                    unsigned matches = 0;
                    for (unsigned k = 0; k < bytes.size(); ++k)
                    {
                        if (bytes[k] == compare_bytes[k])
                            matches++;
                    }
                    // If we match over 75% of the bytes at correct places, assume mesh is same
                    // (due to rounding errors in exporting, there may be meshes which differ slightly with their coordinates)
                    if (matches > (unsigned)(bytes.size() * 0.75f))
                    {
                        duplicate_found = true;
                        // Duplicate was found, adjust the map to refer to the original
                        m->second = created_meshes[j].name_;
                        break;
                    }
                }
            }
            
            if (!duplicate_found)
            {
                QFile mesh_out((out_asset_dir + meshname).c_str());
                if (!mesh_out.open(QFile::WriteOnly))
                    TundraLogicModule::LogError("Could not open output mesh file " + meshname);
                else
                {
                    mesh_out.write(bytes);
                    mesh_out.close();
                    
                    MeshInfo newinfo;
                    newinfo.name_ = meshname;
                    newinfo.filesize_ = bytes.size();
                    created_meshes.push_back(newinfo);
                }
            }
        }
        ++m;
    }
}

void SceneImporter::ProcessNodeForCreation(Scene::ScenePtr scene, QDomElement node_elem, Vector3df pos, Quaternion rot, Vector3df scale,
    AttributeChange::Type change, bool localassets, bool flipyz, bool replace)
{
    while (!node_elem.isNull())
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
            rotx = ParseString<float>(rot_elem.attribute("x").toStdString(), 0.0f);
            roty = ParseString<float>(rot_elem.attribute("y").toStdString(), 0.0f);
            rotz = ParseString<float>(rot_elem.attribute("z").toStdString(), 0.0f);
            rotw = ParseString<float>(rot_elem.attribute("w").toStdString(), 1.0f);
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
            while (node_names_.find(node_name) != node_names_.end())
            {
                node_name = base_node_name + "_" + ToString<int>(append_num);
                ++append_num;
            }
            node_names_.insert(node_name);
            
            // Get mesh name from map
            QString mesh_name = QString::fromStdString(mesh_names_[entity_elem.attribute("meshFile").toStdString()]);
            
            bool cast_shadows = ParseBool(entity_elem.attribute("castShadows").toStdString());
            
            if (localassets)
                mesh_name = "file://" + mesh_name;
            
            Scene::EntityPtr entity;
            bool new_entity = false;
            QString node_name_qstr = QString::fromStdString(node_name);
            
            // Try to find existing entity by name
            if (replace)
            {
                entity = scene->GetEntity(node_name_qstr);
            }
            if (!entity)
            {
                entity = scene->CreateEntity(scene->GetNextFreeId());
                new_entity = true;
            }
            else
            {
                TundraLogicModule::LogInfo("Updating existing entity " + node_name);
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
                    
                    QVector<QVariant> materials;
                    QDomElement subentities_elem = entity_elem.firstChildElement("subentities");
                    if (!subentities_elem.isNull())
                    {
                        QDomElement subentity_elem = subentities_elem.firstChildElement("subentity");
                        while (!subentity_elem.isNull())
                        {
                            QString material_name = subentity_elem.attribute("materialName") + ".material";
                            int index = ParseString<int>(subentity_elem.attribute("index").toStdString());
                            if (localassets)
                                material_name = "file://" + material_name;
                            if (index >= materials.size())
                                materials.resize(index + 1);
                            materials[index] = material_name;

                            subentity_elem = subentity_elem.nextSiblingElement("subentity");
                        }
                    }
                    
                    Transform entity_transform;
                    
                    if (!flipyz)
                    {
                        Vector3df rot_euler;
                        newrot.toEuler(rot_euler);
                        entity_transform.SetPos(newpos.x, newpos.y, newpos.z);
                        entity_transform.SetRot(rot_euler.x * RADTODEG, rot_euler.y * RADTODEG, rot_euler.z * RADTODEG);
                        entity_transform.SetScale(newscale.x, newscale.y, newscale.z);
                    }
                    else
                    {
                        //! \todo it's unpleasant having to do this kind of coordinate mutilations. Possibly move to native Ogre coordinate system?
                        Vector3df rot_euler;
                        Quaternion adjustedrot(-newrot.x, newrot.z, newrot.y, newrot.w);
                        adjustedrot.toEuler(rot_euler);
                        entity_transform.SetPos(-newpos.x, newpos.z, newpos.y);
                        entity_transform.SetRot(rot_euler.x * RADTODEG, rot_euler.y * RADTODEG, rot_euler.z * RADTODEG);
                        entity_transform.SetScale(newscale.x, newscale.y, newscale.z);
                        meshPtr->nodeTransformation.Set(Transform(Vector3df(0,0,0), Vector3df(90,0,180), Vector3df(1,1,1)), change);
                    }

                    placeablePtr->transform.Set(entity_transform, change);
                    meshPtr->meshResourceId.Set(mesh_name, change);
                    meshPtr->meshMaterial.Set(QList<QVariant>::fromVector(materials), change);
                    meshPtr->castShadows.Set(cast_shadows, change);

                    if (new_entity)
                        scene->EmitEntityCreated(entity, change);
                    placeablePtr->ComponentChanged(change);
                    meshPtr->ComponentChanged(change);
                    namePtr->ComponentChanged(change);
                }
                else
                    TundraLogicModule::LogError("Could not create mesh, placeable, name components");
            }
        }
        
        // Process child nodes
        QDomElement childnode_elem = node_elem.firstChildElement("node");
        if (!childnode_elem.isNull())
            ProcessNodeForCreation(scene, childnode_elem, newpos, newrot, newscale, change, localassets, flipyz, replace);
        
        // Process siblings
        node_elem = node_elem.nextSiblingElement("node");
    }
}

std::set<std::string> SceneImporter::ProcessMaterialFile(const std::string& matfilename, const std::set<std::string>& used_materials, const std::string& out_asset_dir, bool localassets)
{
    std::set<std::string> used_textures;
    
    // Read the material file
    QFile matfile(matfilename.c_str());
    if (!matfile.open(QFile::ReadOnly))
    {
        TundraLogicModule::LogError("Could not open material file " + matfilename);
        return used_textures;
    }
    else
    {
        QByteArray bytes = matfile.readAll();
        matfile.close();
        
        QFile matoutfile;
        
        if (bytes.size())
        {
            int num_materials = 0;
            int brace_level = 0;
            bool skip_until_next = false;
            int skip_brace_level = 0;
            Ogre::DataStreamPtr data = Ogre::DataStreamPtr(new Ogre::MemoryDataStream(bytes.data(), bytes.size()));
            
            while (!data->eof())
            {
                std::string line = data->getLine();
                
                // Skip empty lines & comments
                if ((line.length()) && (line.substr(0, 2) != "//"))
                {
                    // Process opening/closing braces
                    if (!ProcessBraces(line, brace_level))
                    {
                        // If not a brace and on level 0, it should be a new material
                        if ((brace_level == 0) && (line.substr(0, 8) == "material") && (line.length() > 8))
                        {
                            // Close the previous material file
                            if (matoutfile.isOpen())
                                matoutfile.close();
                            
                            std::string matname = line.substr(9);
                            if (used_materials.find(matname) == used_materials.end())
                                TundraLogicModule::LogDebug("Encountered unused material " + matname + " in the material file");
                            else
                            {
                                ++num_materials;
                                matoutfile.setFileName(QString::fromStdString(out_asset_dir + matname + ".material"));
                                if (!matoutfile.open(QFile::WriteOnly))
                                    TundraLogicModule::LogError("Could not open material file " + matname + ".material for writing");
                            }
                        }
                        else
                        {
                            // Check for textures
                            if ((line.substr(0, 8) == "texture ") && (line.length() > 8))
                            {
                                std::string texname = line.substr(8);
                                used_textures.insert(texname);
                                if (localassets)
                                    line = "texture file://" + texname;
                            }
                        }
                        // Write line to the modified copy
                        if ((!skip_until_next) && (matoutfile.isOpen()))
                        {
                            matoutfile.write(line.c_str());
                            matoutfile.write("\n");
                        }
                    }
                    else
                    {
                        // Write line to the modified copy
                        if ((!skip_until_next) && (matoutfile.isOpen()))
                        {
                            matoutfile.write(line.c_str());
                            matoutfile.write("\n");
                        }
                        if (brace_level <= skip_brace_level)
                            skip_until_next = false;
                    }
                }
            }
            
            matoutfile.close();
        }
    }
    
    return used_textures;
}

void SceneImporter::ProcessTextures(const std::set<std::string>& used_textures, const std::string& in_asset_dir, const std::string& out_asset_dir)
{
    // Copy textures
    std::set<std::string>::const_iterator i = used_textures.begin();
    while (i != used_textures.end())
    {
        std::string texname = *i;
        CopyAsset(texname, in_asset_dir, out_asset_dir);
        ++i;
    }
}

bool SceneImporter::CopyAsset(const std::string& name, const std::string& in_asset_dir, const std::string& out_asset_dir)
{
    QFile asset_in((in_asset_dir + name).c_str());
    if (!asset_in.open(QFile::ReadOnly))
    {
        TundraLogicModule::LogError("Could not open input asset file " + name);
        return false;
    }
    else
    {
        QByteArray bytes = asset_in.readAll();
        asset_in.close();
        
        QFile asset_out((out_asset_dir + name).c_str());
        if (!asset_out.open(QFile::WriteOnly))
        {
            TundraLogicModule::LogError("Could not open output asset file " + name);
            return false;
        }
        else
        {
            asset_out.write(bytes);
            asset_out.close();
        }
    }
    
    return true;
}

}

