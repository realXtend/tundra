// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraProtocolModuleApi.h"
#include "SceneFwd.h"
#include "AttributeChangeType.h"

class Transform;
class Quat;
class float3;

class QDomElement;

namespace TundraLogic
{

/// Importer tool for OGRE .scene and .mesh files.
/** You can use SceneImporter to directly create and instantiate content from OGRE .mesh and .scene files,
    or to create scene descriptions for the aforementioned file formats. The scene descriptions can be modified
    and then instantiated using Scene::CreateContentFromSceneDesc(). */
class TUNDRAPROTOCOL_MODULE_API SceneImporter
{
public:
    /// Constructs the importer.
    /** @param scene Destination scene. */
    explicit SceneImporter(const ScenePtr &scene);

    /// Destroys the importer.
    ~SceneImporter();

    /// Imports a single mesh. Scans the mesh for needed skeleton & materials.
    /** @param filename Filename of mesh
        @param inAssetDir Where to read input assets. Typically same as the input file path
        @param outAssetDir Where to put resulting assets
        @param worldTransform Transform to use for the entity's placeable.
            You can use Transform's default ctor if you don't want to spesify custom Transform.
        @param entityPrefabXml Prefab data (entity & components in xml serialized format) to use for the entity
        @param prefix Asset storage prefix to be added to the asset references.
        @param change What changetype to use in scene operations
        @param inspect Load and inspect mesh for materials and skeleton
        @param meshName Name of mesh inside the file
        @return Entity pointer if successful, null if failed. */
    EntityPtr ImportMesh(const QString &filename, const QString &inAssetDir, const Transform &worldTransform,
        const QString &entityPrefabXml, const QString &prefix, AttributeChange::Type change, bool inspect = true,
        const QString &meshName = "");

    /// Imports a dotscene.
    /** @param filename Input filename
        @param inAssetDir Where to read input assets. Typically same as the input file path
        @param worldTransform Transform to use for the entity's placeable
        @param prefix Asset storage prefix to be added to the asset references.
        @param change What changetype to use in scene operations
        @param clearscene Whether to clear scene first. Default false
        @param replace Whether to search for entities by name and replace just the visual components (placeable, mesh) if an existing entity is found.
               Default true. If this is false, all entities will be created as new
        @return List of created entities, or an empty list if import failed. */
    QList<Entity *> Import(const QString& filename, const QString &inAssetDir, const Transform &worldTransform,
        const QString &prefix, AttributeChange::Type change, bool clearscene = false, bool replace = true);

    /// Parse a mesh for materials & skeleton ref
    /** @param meshName Full path & filename of mesh
        @param materialNames Return vector for material names
        @param skeletonName Return value for skeleton ref */
    bool ParseMeshForMaterialsAndSkeleton(const QString& meshName, QStringList& materialNames, QString& skeletonName) const;

    /// Inspects OGRE .mesh file and returns a scene description structure of the contents of the file.
    /** @param source Mesh source, can be filename or an URL.
        @note For URLs, this will not dowload the binary data but only adds an entity with the URL mesh ref in it.*/
    SceneDesc CreateSceneDescFromMesh(const QString &source) const;

    /// Inspects OGRE .scene file and returns a scene description structure of the contents of the file.
    /** @param filename Filename.
        @note Currently produces malformed/incomplete SceneDesc structure. Do not use.
        @todo Re-implement! */
    SceneDesc CreateSceneDescFromScene(const QString &filename);

private:
    /// Process the asset references of a node, and its child nodes
    /** @param nodeElem Node element. */
    void ProcessNodeForAssets(QDomElement nodeElem, const QString& inAssetDir);

    /// Process node and its child nodes for creation of entities & components. Done after asset pass
    /** @param [out] entities List of created entities
        @param nodeElem Node element
        @param pos Current position
        @param rot Current rotation
        @param scale Current scale
        @param change What changetype to use in scene operations
        @param flipyz Whether to switch y/z axes from Ogre to OpenSim convention
        @param prefix 
        @param replace Whether to replace contents of entities by name. If false, all entities will be created as new. */
    void ProcessNodeForCreation(QList<Entity *> &entities, QDomElement nodeElem, float3 pos, Quat rot, float3 scale,
        AttributeChange::Type change, const QString &prefix, bool flipyz, bool replace);

    /// Process node and its child nodes for creation of scene description.
    /// @todo Implement and use in CreateSceneDescFromScene
    void ProcessNodeForDesc(SceneDesc &desc, QDomElement nodeElement, float3 pos, Quat rot, float3 scale, const QString &prefix, 
        bool flipYz, QStringList &meshFile, QStringList &skeletonFile, QSet<QString> &usedMaterials, const QString &parentRef = "");

    /// Creates asset descriptions from the provided lists of OGRE resource filenames.
    void CreateAssetDescs(const QString &path, const QStringList &meshFiles, const QStringList &skeletons,
        const QStringList &materialFiles, const QSet<QString> &usedMaterials, SceneDesc &desc) const;

    QMap<QString, QStringList> mesh_default_materials_; ///< Materials read from meshes, in case of no subentity elements
    QSet<QString> material_names_; /// Materials encountered in scene
    QSet<QString> node_names_; ///< Nodes already created into the scene. Used for name-based "update import" logic
    ScenePtr scene_; ///< Destination scene.

    /// Meshes encountered in scene
    /** For supporting binary duplicate detection, this is a map which maps the original names to actual assets that will be stored. */
    QMap<QString, QString> mesh_names_;
};

}
