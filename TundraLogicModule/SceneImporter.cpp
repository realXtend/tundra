// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CoreStringUtils.h"
#include "RexNetworkUtils.h"
#include "DebugOperatorNew.h"
#include "SceneImporter.h"
#include "TundraLogicModule.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "EC_OgrePlaceable.h"
#include "EC_Mesh.h"

#include <Ogre.h>

#include <QDomDocument>
#include <QFile>

using namespace RexTypes;

namespace TundraLogic
{

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

SceneImporter::SceneImporter()
{
}

SceneImporter::~SceneImporter()
{
}

bool SceneImporter::Import(Scene::ScenePtr scene, const std::string& filename, std::string in_asset_dir, std::string out_asset_dir,
    AttributeChange::Type change, bool clearscene, bool localassets)
{
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
    ProcessNodeForAssets(node_elem);
    
    // Write out the needed assets
    ProcessAssets(filename, in_asset_dir, out_asset_dir, localassets);
    
    // Second pass: build scene hierarchy and actually create entities. This assumes assets are available
    ProcessNodeForCreation(scene, node_elem, Vector3df(0.0f, 0.0f, 0.0f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Vector3df(1.0f, 1.0f, 1.0f), change, localassets, flipyz);
    
    }
    catch (Exception& e)
    {
        TundraLogicModule::LogError(std::string("Exception while scene importing: ") + e.what());
        return false;
    }
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
            mesh_names_.insert(mesh_name);
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
    
    // Read the material file
    QFile matfile(matfilename.c_str());
    if (!matfile.open(QFile::ReadOnly))
        TundraLogicModule::LogError("Could not open material file. Skipping material & texture assets.");
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
                            if (material_names_.find(matname) == material_names_.end())
                                TundraLogicModule::LogWarning("Encountered unused material " + matname + " in the material file");
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
                                texture_names_.insert(texname);
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
    
    // Copy textures
    std::set<std::string>::iterator i = texture_names_.begin();
    while (i != texture_names_.end())
    {
        std::string texname = *i;
        QFile texture_in((in_asset_dir + texname).c_str());
        if (!texture_in.open(QFile::ReadOnly))
            TundraLogicModule::LogError("Could not open input texture file " + texname);
        else
        {
            QByteArray bytes = texture_in.readAll();
            texture_in.close();
            QFile texture_out((out_asset_dir + texname).c_str());
            if (!texture_out.open(QFile::WriteOnly))
                TundraLogicModule::LogError("Could not open output texture file " + texname);
            else
            {
                texture_out.write(bytes);
                texture_out.close();
            }
        }
        ++i;
    }
    
    // Copy meshes
    //! \todo check for binary duplicates
    i = mesh_names_.begin();
    while (i != mesh_names_.end())
    {
        std::string meshname = *i;
        QFile mesh_in((in_asset_dir + meshname).c_str());
        if (!mesh_in.open(QFile::ReadOnly))
            TundraLogicModule::LogError("Could not open input mesh file " + meshname);
        else
        {
            QByteArray bytes = mesh_in.readAll();
            mesh_in.close();
            QFile mesh_out((out_asset_dir + meshname).c_str());
            if (!mesh_out.open(QFile::WriteOnly))
                TundraLogicModule::LogError("Could not open output mesh file " + meshname);
            else
            {
                mesh_out.write(bytes);
                mesh_out.close();
            }
        }
        ++i;
    }
}

void SceneImporter::ProcessNodeForCreation(Scene::ScenePtr scene, QDomElement node_elem, Vector3df pos, Quaternion rot, Vector3df scale,
    AttributeChange::Type change, bool localassets, bool flipyz)
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
            QString mesh_name = entity_elem.attribute("meshFile");
            bool cast_shadows = ParseBool(entity_elem.attribute("castShadows").toStdString());
            
            if (localassets)
                mesh_name = "file://" + mesh_name;

            QStringList components;
            components.append(EC_Mesh::TypeNameStatic());
            components.append(OgreRenderer::EC_OgrePlaceable::TypeNameStatic());
            
            Scene::EntityPtr entity = scene->CreateEntity(scene->GetNextFreeId(), components);
            EC_Mesh* meshPtr = 0;
            OgreRenderer::EC_OgrePlaceable* placeablePtr = 0;
            if (entity)
            {
                meshPtr = entity->GetComponent<EC_Mesh>().get();
                placeablePtr = entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
                
                std::vector<QVariant> materials;
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
                    //! \todo this probably breaks due to the hardcoded adjustment of meshes from Ogre to Opensim orientation
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
                }

                placeablePtr->transform_.Set(entity_transform, change);

                meshPtr->meshResouceId_.Set(mesh_name, change);
                meshPtr->meshMaterial_.Set(materials, change);
                meshPtr->castShadows_.Set(cast_shadows, change);

                scene->EmitEntityCreated(entity, change);
                placeablePtr->ComponentChanged(change);
                meshPtr->ComponentChanged(change);
            }
            

        }
        
        // Process child nodes
        QDomElement childnode_elem = node_elem.firstChildElement("node");
        if (!childnode_elem.isNull())
            ProcessNodeForCreation(scene, childnode_elem, newpos, newrot, newscale, change, localassets, flipyz);
        
        // Process siblings
        node_elem = node_elem.nextSiblingElement("node");
    }
}

}

