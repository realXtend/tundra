// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_SceneImporter_h
#define incl_TundraLogicModule_SceneImporter_h

#if defined (_WINDOWS)
#if defined(TUNDRALOGIC_MODULE_EXPORTS)
#define TUNDRALOGIC_MODULE_API __declspec(dllexport)
#else
#define TUNDRALOGIC_MODULE_API __declspec(dllimport)
#endif
#else
#define TUNDRALOGIC_MODULE_API
#endif

#include "SceneManager.h"
#include "IAttribute.h"
#include "Transform.h"

class QDomElement;

#include <map>

namespace Foundation
{
    class Framework;
}

namespace TundraLogic
{

//! Importer tool for OGRE .scene and .mesh files
class TUNDRALOGIC_MODULE_API SceneImporter
{
public:
    //! Constructs the importer.
    /*! \param scene Destination scene
    */
    explicit SceneImporter(Foundation::Framework* framework);
    
    //! Destroyes the importer.
    ~SceneImporter();
    
    //! Import a single mesh. Scans the mesh for needed skeleton & materials
    /*! \param scene Destination scene
        \param meshname Filename of mesh
        \param in_asset_dir Where to read input assets. Typically same as the input file path
        \param out_asset_dir Where to put resulting assets
        \param worldtransform Transform to use for the entity's placeable. You can use Transform's default ctor if you don't want to spesify custom Transform.
        \param entity_prefab_xml Prefab data (entity & components in xml serialized format) to use for the entity
        \param change What changetype to use in scene operations
        \param localassets Whether to put file:// prefix into all asset references
        \return Entity pointer if successful (null if failed)
     */
    Scene::EntityPtr SceneImporter::ImportMesh(Scene::ScenePtr scene, const std::string& meshname, std::string in_asset_dir, std::string out_asset_dir,
        const Transform &worldtransform, const std::string& entity_prefab_xml, AttributeChange::Type change, bool localassets);
    
    //! Import a dotscene
    /*! \param scene Destination scene
        \param filename Input filename
        \param in_asset_dir Where to read input assets. Typically same as the input file path
        \param out_asset_dir Where to put resulting assets
        \param worldtransform Transform to use for the entity's placeable
        \param change What changetype to use in scene operations
        \param clearscene Whether to clear scene first. Default false
        \param localassets Whether to put file:// prefix into all asset references
        \param replace Whether to search for entities by name and replace just the visual components (placeable, mesh) if an existing entity is found.
               Default true. If this is false, all entities will be created as new
        \return true if successful
     */
    bool Import(Scene::ScenePtr scene, const std::string& filename, std::string in_asset_dir, std::string out_asset_dir, const Transform &worldtransform,
        AttributeChange::Type change, bool clearscene = false, bool localassets = true, bool replace = true);
    
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
    
    //! Process a material file, searching for used materials and writing them to separate files if found, and also recording used textures
    /*! \param matfilename Material file name, including full path
        \param used_materials Set of materials that are needed. Any materials in the file that are not listed will be skipped
        \param out_asset_dir Directory to write output materials to
        \param localassets Whether to put file:// prefix into all asset references
        \return Set of used textures
     */
    std::set<std::string> ProcessMaterialFile(const std::string& matfilename, const std::set<std::string>& used_materials, const std::string& out_asset_dir, bool localassets);
    
    //! Copy textures to destination asset directory
    /*! \param used_textures Set of texture names
        \param in_asset_dir Directory to copy from
        \param out_asset_dir Directory to copy to
     */
    void ProcessTextures(const std::set<std::string>& used_textures, const std::string& in_asset_dir, const std::string& out_asset_dir);
    
    //! Copy an asset file from source to destination asset directory.
    /*! \param name Filename
        \param in_asset_dir Directory to copy from
        \param out_asset_dir Directory to copy to
        \return true if successful
     */
    bool CopyAsset(const std::string& name, const std::string& in_asset_dir, const std::string& out_asset_dir);
    
    //! Materials encountered in scene
    std::set<std::string> material_names_;
    //! Meshes encountered in scene
    /*! For supporting binary duplicate detection, this is a map which maps the original names to actual assets that will be stored.
     */
    std::map<std::string, std::string> mesh_names_;
    //! Nodes already created into the scene. Used for name-based "update import" logic
    std::set<std::string> node_names_;
    
    //! Framework
    Foundation::Framework* framework_;
};

}

#endif
