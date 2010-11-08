// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneParser.h"
#include "WorldBuildingModule.h"
#include "SceneManager.h"

#include "EC_DynamicComponent.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "EC_OpenSimPrim.h"
#include "EC_Highlight.h"

#include "OgreVector3.h"
#include "OgreSceneNode.h"
#include "OgreEntity.h"

#include <QFile>
#include <QDomDocument>
#include <QDomElement>

namespace WorldBuilding
{
    SceneParser::SceneParser(QObject *parent, Foundation::Framework *framework) :
        QObject(parent),
        framework_(framework),
        export_component_name_("RexPrimExportData")
    {
        mesh_ref_set = 0;
        animation_ref_set = 0;
        material_ref_set = 0;
        particle_ref_set = 0;
        sound_ref_set = 0;
    }

    SceneParser::~SceneParser()
    {
    }

    void SceneParser::ExportToFile(const QString &filename, QList<Scene::Entity *> entities)
    {
        WorldBuildingModule::LogDebug(QString("SceneParser: Adding export data component to %1 entities").arg(entities.count()).toStdString().c_str());

        foreach(Scene::Entity *entity, entities)
            AddExportData(entity);
        ExportXml(filename, entities);
        RemoveExportData(entities);
    }

    QByteArray SceneParser::ExportToByteArray(QList<Scene::Entity *> entities)
    {
        WorldBuildingModule::LogDebug(QString("SceneParser: Adding export data component to %1 entities").arg(entities.count()).toStdString().c_str());

        QByteArray return_array;
        foreach(Scene::Entity *entity, entities)
            AddExportData(entity);
        return_array = ExportXml("", entities);
        
        return return_array;
    }

    QByteArray SceneParser::ExportSceneXml()
    {
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return QByteArray();

        QDomDocument scene_doc("Scene");
        QDomElement scene_elem = scene_doc.createElement("scene");

        QList<Scene::Entity*> entities;
        Scene::SceneManager::iterator iter = scene->begin();
        Scene::SceneManager::iterator end = scene->end();

        QStringList non_wanted;
        non_wanted << "EC_Terrain" << "EC_WaterPlane" << "EC_EnvironmentLight";
        
        uint comp_count = 0;
        while (iter != end)
        {
            Scene::Entity *entity = iter->second.get();
            if (entity)
            {
                // Components or inspection
                const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
                if (components.empty())
                {
                    WorldBuildingModule::LogInfo("> Skipping entity: no components");
                    ++iter;
                    continue;
                }

                // Public voice channel
                if (entity->HasComponent("EC_VoiceChannel") && components.size() == 1)
                {
                    WorldBuildingModule::LogInfo("> Skipping entity: Public voip channel");
                    ++iter;
                    continue;
                }

                // Non-wanted entities
                foreach(QString dont_want, non_wanted)
                {
                    if (entity->HasComponent(dont_want))
                    {
                        WorldBuildingModule::LogInfo("> Skipping entity: Has unwanted components for export");
                        ++iter;
                        continue;
                    }
                }

                if (AddExportData(entity))
                    entities.append(entity);
                else
                {
                    WorldBuildingModule::LogInfo("> Skipping entity: No prim/mesh/placeable component(s)");
                    ++iter;
                    continue;
                }

                int writable_comps = 0;
                for(uint i = 0; i < components.size(); ++i)
                {
                    // Exclude serializable components that would cause dublicated data to server opensim server database, modrex in particular
                    if (components[i]->TypeName() == "EC_Mesh" || components[i]->TypeName() == "EC_Placeable" ||
                        components[i]->TypeName() == "EC_AnimationController")                        
                        continue;
                    if (components[i]->IsSerializable())
                        writable_comps++;
                }
                if (writable_comps == 0)
                {
                    WorldBuildingModule::LogInfo("> Skipping entity: No serializable component");
                    ++iter;
                    continue;
                }

                QDomElement entity_elem = scene_doc.createElement("entity");
                QString id_str = QString::number(entity->GetId());
                entity_elem.setAttribute("id", id_str);
                

                for(uint i = 0; i < components.size(); ++i)
                {
                    // Exclude serializable components that would cause dublicated data to server opensim server database, modrex in particular
                    if (components[i]->TypeName() == "EC_Mesh" || components[i]->TypeName() == "EC_Placeable" ||
                        components[i]->TypeName() == "EC_AnimationController")                        
                        continue;
                    if (components[i]->IsSerializable())
                    {
                        components[i]->SerializeTo(scene_doc, entity_elem);
                        comp_count++;
                    }
                }
                scene_elem.appendChild(entity_elem);
            }
            ++iter;
        }
        WorldBuildingModule::LogInfo(QString("Completed exporting %1 entities with %2 components").arg( QString::number(entities.count()), QString::number(comp_count)).toStdString().c_str());

        scene_doc.appendChild(scene_elem);
        RemoveExportData(entities);
        return scene_doc.toByteArray();;
    }

    QByteArray SceneParser::ExportXml(const QString &filename, const QList<Scene::Entity *> entity_list)
    {
        QDomDocument scene_doc("Scene");
        QDomElement scene_elem = scene_doc.createElement("scene");
        
        QList<Scene::Entity*>::const_iterator iter = entity_list.begin();
        QList<Scene::Entity*>::const_iterator end = entity_list.end();

        while (iter != end)
        {
            Scene::Entity *entity = *iter;
            if (entity)
            {
                QDomElement entity_elem = scene_doc.createElement("entity");
                QString id_str = QString::number(entity->GetId());
                entity_elem.setAttribute("id", id_str);

                const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
                for(uint i = 0; i < components.size(); ++i)
                {
                    // Exclude serializable components that would cause dublicated data to server opensim server database, modrex in particular
                    if (components[i]->TypeName() == "EC_Mesh" || components[i]->TypeName() == "EC_Placeable" ||
                        components[i]->TypeName() == "EC_AnimationController")
                        continue;
                    if (components[i]->IsSerializable())
                        components[i]->SerializeTo(scene_doc, entity_elem);
                }
                scene_elem.appendChild(entity_elem);
            }
            ++iter;
        }
        
        scene_doc.appendChild(scene_elem);
        
        QByteArray bytes = scene_doc.toByteArray();
        if (!filename.isEmpty())
        {
            QFile export_file(filename);
            if (export_file.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                WorldBuildingModule::LogDebug(QString("Storing scene to file: " + filename).toStdString().c_str());
                export_file.resize(0); // erases old data if present
                export_file.write(bytes);
                export_file.close();
            }
        }
        return bytes;
    }

    QByteArray SceneParser::ParseAndAdjust(const QByteArray &content, const Vector3df &avatar_position)
    {
        QDomDocument document;
        if (!document.setContent(content, true))
        {
            WorldBuildingModule::LogDebug("Failed to read xml content for adjusting");
            return QByteArray();
        }

        // Get root of the document
        QDomElement main_element = document.documentElement();
        QDomNode node = main_element.firstChild();

        // Iterate whole document, this will fill our affected_list_
        affected_list_.clear();
        while (!node.isNull())
        {
            QDomElement node_element = node.toElement();
            if (!node_element.isNull())
                ProcessElement(node_element, avatar_position);
            node = node.nextSibling();
        }

        // Calculate center point of the set
        f32 x = 0.0f;
        f32 y = 0.0f; 
        f32 z = 0.0f;
        int main_node_count = 0;
        foreach(AdjustData adjust, affected_list_.values())
        {
            if (adjust.is_child)
                continue;
            x += adjust.original_pos.x;
            y += adjust.original_pos.y;
            z += adjust.original_pos.z;
            main_node_count++;
        }
        if (main_node_count > 1)
        {
            x /= main_node_count;
            y /= main_node_count;
            z /= main_node_count;
        }
        Vector3df set_center(x, y, z);

        WorldBuildingModule::LogDebug(QString("Parser: Object set center position %1 %2 %3").arg(QString::number(x), QString::number(y), QString::number(z)).toStdString().c_str());
        WorldBuildingModule::LogDebug(QString("Parser: Adjusting position for %1 nodes").arg(affected_list_.count()).toStdString().c_str());

        foreach(AdjustData adjust, affected_list_.values())
        {
            // Adjust main entity positions
            if (!adjust.is_child)
            {
                adjust.offset = adjust.original_pos - set_center;                
                Vector3df main_end_pos = avatar_position + adjust.offset;
               
                QStringList main_position;
                main_position << QString::number(main_end_pos.x) << QString::number(main_end_pos.y) << QString::number(main_end_pos.z);
                adjust.node_position.setNodeValue(main_position.join(","));

                WorldBuildingModule::LogDebug(QString("Node adjusted to %1 %2 %3").arg(QString::number(main_end_pos.x), QString::number(main_end_pos.y), QString::number(main_end_pos.z)).toStdString().c_str());
            }

            // Adjust child entity positions
            foreach(AdjustData *child_adj, adjust.children)
            {
                Vector3df parent = adjust.original_pos;
                Vector3df child = child_adj->original_pos;
                Vector3df offset = child - parent;
                Vector3df end_pos = avatar_position + adjust.offset;
                end_pos = end_pos + offset;

                QStringList child_position;
                child_position << QString::number(end_pos.x) << QString::number(end_pos.y) << QString::number(end_pos.z);
                child_adj->node_position.setNodeValue(child_position.join(","));

                WorldBuildingModule::LogDebug(QString(">> Child Node adjusted to %1 %2 %3").arg(QString::number(end_pos.x), QString::number(end_pos.y), QString::number(end_pos.z)).toStdString().c_str());
            }
        }

        return document.toByteArray();
    }

    void SceneParser::ProcessElement(QDomElement node_element, Vector3df adjust_pos)
    {
        if (node_element.isNull())
            return;

        QDomNamedNodeMap attributes = node_element.attributes();
        if (!attributes.isEmpty())
        {
            QString id = attributes.namedItem("id").nodeValue();
            QString type = attributes.namedItem("type").nodeValue();
            QString name = attributes.namedItem("name").nodeValue();
            QString value = attributes.namedItem("value").nodeValue();
            if (!id.isEmpty())
            {
                AdjustData adj_ent;
                adj_ent.id = id.toUInt();
                adj_ent.parent = 0;
                adj_ent.is_child = false;

                affected_list_[adj_ent.id] = adj_ent;
                processing_ent_id_ = adj_ent.id;

                WorldBuildingModule::LogDebug(QString("Parser: Processing entity %1").arg(id).toStdString().c_str());
            }
            else if (!type.isEmpty() && type != "qvariant")
            {
            }
            else if (!name.isEmpty())
            {
                if (name == "Position" && node_element.parentNode().attributes().namedItem("name").nodeValue() == export_component_name_)
                {
                    if (affected_list_.contains(processing_ent_id_))
                    {
                        AdjustData *adj = &affected_list_[processing_ent_id_];
                        adj->SetPosition(attributes.namedItem("value"));
                        WorldBuildingModule::LogDebug(">> Original position stored");
                    }
                }
                else if (name == "Parent" && node_element.parentNode().attributes().namedItem("name").nodeValue() == export_component_name_)
                {
                    if (affected_list_.contains(processing_ent_id_))
                    {
                        entity_id_t parent = value.toUInt();
                        AdjustData *adj = &affected_list_[processing_ent_id_];
                        adj->parent = parent;
                        if (affected_list_.contains(parent))
                        {
                            AdjustData *adj_parent = &affected_list_[parent];
                            adj_parent->children.append(&affected_list_[processing_ent_id_]);
                            adj->is_child = true;
                            WorldBuildingModule::LogDebug(">> Parent entity found");
                        }
                    }
                }
            }

            QDomNodeList children = node_element.childNodes();
            for (uint i=0; i< children.length(); i++)
                ProcessElement(children.at(i).toElement(), adjust_pos);
        }
    }

    bool SceneParser::AddExportData(Scene::Entity *entity)
    {
        EC_Placeable *placeable = entity->GetComponent<EC_Placeable>().get();
        EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
        if (!entity || !placeable || !prim)
            return false;

        // Create export data component
        AttributeChange::Type ct = AttributeChange::Disconnected;
        ComponentPtr comp = entity->GetOrCreateComponent(EC_DynamicComponent::TypeNameStatic(), export_component_name_, ct, false);
        EC_DynamicComponent *export_data = dynamic_cast<EC_DynamicComponent*>(comp.get());
        if (!export_data)
        {
            WorldBuildingModule::LogError("Could not create export data dynamic component, aborting adding export data.");
            return false;
        }

        // Once more...
        export_data->SetUpdateMode(ct);
        export_data->SetNetworkSyncEnabled(false);

        // Update placeables transform value for export
        Ogre::Vector3 pos = placeable->GetLinkSceneNode()->_getDerivedPosition();
        Ogre::Vector3 scale = placeable->GetSceneNode()->_getDerivedScale();
        Ogre::Quaternion orientation = placeable->GetLinkSceneNode()->_getDerivedOrientation();

        // Add attributes
        QStringList attributes; 
        attributes << "Position" << "Scale" << "Orientation" << "Parent" << "MeshRef" << "CollisionMeshRef" << "SkeletonRef" << "Materials" << "MaterialTypes" << "ParticleRef"
                   << "DrawType" << "DrawDistance" << "ScaleToPrim" << "CastShadows" << "LightCreatesShadows" << "SoundID" 
                   << "SoundVolume" << "SoundRadius" << "LOD" << "Visible" << "AnimationName" << "AnimationRate" << "ServerScriptClass";        
        foreach(QString attr_name, attributes)
        {
            if (export_data->GetAttribute(attr_name).isNull())
                export_data->AddQVariantAttribute(attr_name, ct);
        }

        // 3D data
        export_data->SetAttribute("Position", QString("%1,%2,%3").arg(QString::number(pos.x), QString::number(pos.y), QString::number(pos.z)), ct);
        export_data->SetAttribute("Scale", QString("%1,%2,%3").arg(QString::number(scale.x), QString::number(scale.y), QString::number(scale.z)), ct);
        export_data->SetAttribute("Orientation", QString("%1,%2,%3,%4").arg(QString::number(orientation.w), QString::number(orientation.x), QString::number(orientation.y), QString::number(orientation.z)), ct);
        
        // Parent
        QString parent = "0";
        ComponentPtr parent_comp = placeable->GetParent();
        if (parent_comp)
        {
            Scene::Entity *parent_ent = parent_comp->GetParentEntity();
            if (parent_ent)
                parent = QString::number(parent_ent->GetId());
        }
        export_data->SetAttribute("Parent", parent, ct);

        // Mesh
        QString zero_uuid = "00000000-0000-0000-0000-000000000000";
        if (prim->getMeshID() != zero_uuid)
        {
            export_data->SetAttribute("MeshRef", prim->getMeshID(), ct);
            if (mesh_ref_set)
                mesh_ref_set->insert(prim->getMeshID());
        }
        else
            export_data->SetAttribute("MeshRef", "", ct);

        // Collision mesh
        if (prim->getCollisionMeshID() != zero_uuid)
            export_data->SetAttribute("CollisionMeshRef", prim->getCollisionMeshID(), ct);
        else
            export_data->SetAttribute("CollisionMeshRef", "", ct);

        // Animation
        if (prim->getAnimationPackageID() != zero_uuid)
        {
            export_data->SetAttribute("SkeletonRef", prim->getAnimationPackageID(), ct);
            if (animation_ref_set)
                animation_ref_set->insert(prim->getAnimationPackageID());
        }
        else
            export_data->SetAttribute("SkeletonRef", "", ct);

        // Particle script
        if (prim->getParticleScriptID() != zero_uuid)
        {
            export_data->SetAttribute("ParticleRef", prim->getParticleScriptID(), ct);
            if (particle_ref_set)
                particle_ref_set->insert(prim->getParticleScriptID());
        }
        else
            export_data->SetAttribute("ParticleRef", "", ct);

        // Drawing
        export_data->SetAttribute("DrawType", prim->getDrawType(), ct);
        export_data->SetAttribute("DrawDistance", prim->getDrawDistance(), ct);
        export_data->SetAttribute("ScaleToPrim", prim->getScaleToPrim(), ct);
        export_data->SetAttribute("CastShadows", prim->getCastShadows(), ct);
        export_data->SetAttribute("LightCreatesShadows", prim->getLightCreatesShadows(), ct);
        export_data->SetAttribute("LOD", prim->getLOD(), ct);
        export_data->SetAttribute("Visible", prim->getIsVisible(), ct);

        // Sound
        if (prim->getSoundID() != zero_uuid)
        {
            export_data->SetAttribute("SoundID", prim->getSoundID(), ct);
            if (sound_ref_set)
                sound_ref_set->insert(prim->getSoundID());
        }
        else
            export_data->SetAttribute("SoundID", "", ct);
        export_data->SetAttribute("SoundVolume", prim->getSoundVolume(), ct);
        export_data->SetAttribute("SoundRadius", prim->getSoundRadius(), ct);
        
        // Animation        
        export_data->SetAttribute("AnimationName", prim->getAnimationName(), ct);
        export_data->SetAttribute("AnimationRate", prim->getAnimationRate(), ct);

        // Script
        export_data->SetAttribute("ServerScriptClass", prim->getServerScriptClass(), ct);

        // Materials
        QStringList materials;
        QStringList material_types;
        MaterialMap mats = prim->Materials;
        MaterialMap::const_iterator iter = mats.begin();
        while (iter != mats.end())
        {
            QString mat_ref(iter->second.asset_id.c_str());
            if (mat_ref != zero_uuid)
            {
                uint mat_type = iter->second.Type;
                // Many worlds seem to give 4 here, i assume parsing error somewhere down the line
                // and will make it a material id
                if (mat_type == 4) 
                    mat_type = 45;
                materials.append(mat_ref);
                material_types.append(QString::number(mat_type));
                if (material_ref_set)
                    material_ref_set->insert(mat_ref, mat_type);
            }
            ++iter;
        }
        if (materials.count() == 0)
        {
            export_data->SetAttribute("Materials", "", ct);
            export_data->SetAttribute("MaterialTypes", "", ct);
        }
        else
        {
            export_data->SetAttribute("Materials", materials.join(";"), ct);
            export_data->SetAttribute("MaterialTypes", material_types.join(";"), ct);
        }

        return true;
    }

    void SceneParser::RemoveExportData(QList<Scene::Entity *> entities)
    {
        foreach(Scene::Entity *entity, entities)
        {
            ComponentPtr export_component = entity->GetComponent(EC_DynamicComponent::TypeNameStatic(), export_component_name_);
            if (export_component.get())
                entity->RemoveComponent(export_component, AttributeChange::Disconnected);
        }
    }

    QList<Scene::Entity*> SceneParser::GetAllPlaceableChildren(EC_Placeable *parent)
    {
        QList<Scene::Entity*> return_list;
        Scene::ScenePtr scene_ptr = framework_->GetDefaultWorldScene();
        if (scene_ptr && parent)
        {
            Scene::SceneManager::EntityMap::iterator iter = scene_ptr->begin();
            Scene::SceneManager::EntityMap::iterator end = scene_ptr->end();
            while (iter != end)
            {
                Scene::EntityPtr child_entity = iter->second;
                EC_Placeable *placeable = child_entity->GetComponent<EC_Placeable>().get();
                if (placeable)
                    if (placeable->GetParent().get() ==  parent)
                        return_list.append(child_entity.get());
                ++iter;
            }
        }
        return return_list;
    }
}