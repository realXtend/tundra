// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_SceneImporter_h
#define incl_TundraLogicModule_SceneImporter_h

#include "SceneManager.h"
#include "IAttribute.h"

class QDomElement;

#include <map>

namespace Foundation
{
    class Framework;
}

namespace TundraLogic
{

class SceneImporter
{
public:
    SceneImporter(Foundation::Framework* framework);
    ~SceneImporter();
    
    //! Import a dotscene
    /*! \param scene Destination scene
        \param filename Input filename
        \param in_asset_dir Where to read input assets. Typically same as the input file path
        \param out_asset_dir Where to put resulting assets
        \param change What changetype to use in scene operations
        \param clearscene Whether to clear scene first. Default false
        \param localassets Whether to put file:// prefix into all asset references
        \param replace Whether to search for entities by name and replace just the visual components (placeable, mesh) if an existing entity is found.
               Default true. If this is false, all entities will be created as new
        \return true if successful
     */
    bool Import(Scene::ScenePtr scene, const std::string& filename, std::string in_asset_dir, std::string out_asset_dir, AttributeChange::Type change,
        bool clearscene = false, bool localassets = true, bool replace = true);
    
private:
    //! Process the asset references of a node, and its child nodes
    /*! \param node_elem Node element
     */
    void ProcessNodeForAssets(QDomElement node_elem);
    
    //! Process assets
    /*! Note: for materials, if scene is called scene.scene, then material file is assumed to be scene.material in the same dir
    /*! \param filename Scene filename
        \param in_asset_dir Where to read input assets. Typically same as the input file path
        \param out_asset_dir Where to put resulting assets
        \param localassets Whether to put file:// prefix into all asset references
     */
    void ProcessAssets(const std::string& filename, const std::string& in_asset_dir, const std::string& out_asset_dir, bool localassets);
    
    //! Process node and its child nodes for creation of entities & components. Done after asset pass
    /*! \param scene Destination scene
        \param node_elem Node element
        \param pos Current position
        \param rot Current rotation
        \param scale Current scale
        \param change What changetype to use in scene operations
        \param localassets Whether to put file:// prefix into all asset references
        \param flipyz Whether to switch y/z axes from Ogre to OpenSim convention
        \param replace Whether to replace contents of entities by name. If false, all entities will be created as new
     */
    void ProcessNodeForCreation(Scene::ScenePtr scene, QDomElement node_elem, Vector3df pos, Quaternion rot, Vector3df scale,
        AttributeChange::Type change, bool localassets, bool flipyz, bool replace);
    
    //! Materials encountered in scene
    std::set<std::string> material_names_;
    //! Textures encountered in scene
    std::set<std::string> texture_names_;
    //! Meshes encountered in scene
    /*! For supporting binary duplicate detection, this is a map which maps the original names to actual assets that will be stored.
     */
    std::map<std::string, std::string> mesh_names_;
    //! Nodes already created into the scene. Used for enforcing uniqueness (otherwise the name-based "update import" logic will fail)
    std::set<std::string> node_names_;
    
    //! Framework
    Foundation::Framework* framework_;
};

}

#endif
