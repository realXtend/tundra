// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_SceneImporter_h
#define incl_TundraLogicModule_SceneImporter_h

#include "SceneManager.h"
#include "AttributeInterface.h"

class QDomElement;

#include <map>

namespace TundraLogic
{

class SceneImporter
{
public:
    SceneImporter();
    ~SceneImporter();
    
    //! Import a scene
    /*! \param scene Destination scene
        \param filename Input filename
        \param in_asset_dir Where to read input assets. Typically same as the input file path
        \param out_asset_dir Where to put resulting assets
        \param change What changetype to use in scene operations
        \param clearscene Whether to clear scene first
        \param localassets Whether to put file:// prefix into all asset references
        \return true if successful
     */
    bool Import(Scene::ScenePtr scene, const std::string& filename, std::string in_asset_dir, std::string out_asset_dir, AttributeChange::Type change,
        bool clearscene = false, bool localassets = true);
    
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
     */
    void ProcessNodeForCreation(Scene::ScenePtr scene, QDomElement node_elem, Vector3df pos, Quaternion rot, Vector3df scale,
        AttributeChange::Type change, bool localassets, bool flipyz);
        
private:
    //! Materials encountered in scene
    std::set<std::string> material_names_;
    //! Textures encountered in scene
    std::set<std::string> texture_names_;
    //! Meshes encountered in scene
    std::set<std::string> mesh_names_;
};

}

#endif
