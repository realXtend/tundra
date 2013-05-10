// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "EnvironmentModuleApi.h"
#include "IComponent.h"
#include "Math/float3.h"
#include "Transform.h"
#include "AssetReference.h"
#include "AssetFwd.h"
#include "AssetRefListener.h"
#include "OgreModuleFwd.h"

namespace Ogre { class Matrix4; }

/// Adds a heightmap-based terrain to the scene.
/** <table class="header">

    <tr>
    <td>
    <h2>Terrain</h2>
    Adds a heightmap-based terrain to the scene. A Terrain is composed of a rectangular grid of adjacent "patches".
    Each patch is a fixed-size 16x16 height map.

    Registered by EnvironmentComponents plugin.

    <b>Attributes:</b>
    <ul>
    <li>Transform: nodeTransformation
    <div> @copydoc nodeTransformation </div>
    <li>int: xPatches
    <div> @copydoc xPatches</div>
    <li>int: yPatches
    <div> @copydoc yPatches</div>
    <li>float: uScale
    <div> @copydoc uScale </div>
    <li>float : vScale
    <div> @copydoc vScale </div>
    <li>AssetReference: material
    <div> @copydoc material </div>
    <li>AssetReference: heightMap
    <div> @copydoc heightMap </div>
    </ul>

    Note that the way the textures are used depends completely on the material. For example, the default height-based terrain material "Rex/TerrainPCF"
    only uses the texture channels 0-3, and blends between those based on the terrain height values.

    Does not emit or react on any actions.

    <b>Does not depend on any other components</b>. Currently Terrain stores its own transform matrix, so it does not depend on the Placeable component. It might be more consistent
    to create a dependency to Placeable, so that the position of the terrain is editable in the same way the position of other placeables is done.
    </table> */
class ENVIRONMENT_MODULE_API EC_Terrain : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Terrain", 11)

public:
    /// @cond PRIVATE
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Terrain(Scene* scene);
    /// @endcond
    virtual ~EC_Terrain();

    /// Specifies the transformation matrix of the terrain
    Q_PROPERTY(Transform nodeTransformation READ getnodeTransformation WRITE setnodeTransformation);
    DEFINE_QPROPERTY_ATTRIBUTE(Transform, nodeTransformation);

    /// The number of patches to generate in the terrain in the horizontal direction, in the range [0, 256].
    Q_PROPERTY(uint xPatches READ getxPatches WRITE setxPatches);
    DEFINE_QPROPERTY_ATTRIBUTE(uint, xPatches);

    /// The number of patches to generate in the terrain in the vertical direction, in the range [0, 256].
    Q_PROPERTY(uint yPatches READ getyPatches WRITE setyPatches);
    DEFINE_QPROPERTY_ATTRIBUTE(uint, yPatches);

    /// Texture U coordinate scaling factor.
    Q_PROPERTY(float uScale READ getuScale WRITE setuScale);
    DEFINE_QPROPERTY_ATTRIBUTE(float, uScale);

    /// Texture V coordinate scaling factor.
    Q_PROPERTY(float vScale READ getvScale WRITE setvScale);
    DEFINE_QPROPERTY_ATTRIBUTE(float, vScale);

    /// Specifies the material to use when rendering the terrain.
    Q_PROPERTY(AssetReference material READ getmaterial WRITE setmaterial);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, material);

    /// Specifies the height map used to generate the terrain.
    Q_PROPERTY(AssetReference heightMap READ getheightMap WRITE setheightMap);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, heightMap);

    /// Returns the minimum and maximum extents of terrain heights.
    void GetTerrainHeightRange(float &minHeight, float &maxHeight) const;

    /// Each patch is a square containing this many vertices per side.
    static const uint cPatchSize = 16;

    /// Describes a single patch that is present in the scene.
    /** A patch can be in one of the following three states:
        - not loaded. The height data nor the GPU data is present, but the Patch struct itself is initialized. heightData.size() == 0, node == entity == 0. meshGeometryName == "".
        - heightmap data loaded. The heightData vector contains the heightmap data, but the visible GPU vertex data itself has not been generated yet, due to the neighbors
          of this patch not being present yet. node == entity == 0, meshGeometryName == "". patch_geometry_dirty == true.
        - fully loaded. The GPU data is also loaded and the node, entity and meshGeometryName fields specify the used GPU resources. */
    struct Patch
    {
        Patch():x(0),y(0), node(0), entity(0), patch_geometry_dirty(true) {}

        /// X-coordinate on the grid of patches. In the range [0, EC_Terrain::PatchWidth()].
        uint x;

        /// Y-coordinate on the grid of patches. In the range [0, EC_Terrain::PatchHeight()].
        uint y;

        /// Typically this will be a 16x16 array of height values in world coordinates.
        /// If the length is zero, this patch hasn't been loaded in yet.
        std::vector<float> heightData;

        /// Ogre -specific: Store a reference to the actual render hierarchy node.
        Ogre::SceneNode *node;

        /// Ogre -specific: Store a reference to the entity that is attached to the above SceneNode.
        Ogre::Entity *entity;

        /// The name of the Ogre Mesh resource that contains the GPU geometry data for this patch.
        std::string meshGeometryName;

        /// If true, the CPU-side heightmap data has changed, but we haven't yet updated
        /// the GPU-side geometry resources since the neighboring patches haven't been loaded
        /// in yet.
        bool patch_geometry_dirty;

        /// Call only when you've checked that this patch has been loaded in.
        float GetHeightValue(uint x, uint y) const { return heightData[y*cPatchSize+x]; }
    };
    
    /// @return The patch at given (x,y) coordinates. Pass in values in range [0, PatchWidth()/PatchHeight[.
    Patch &GetPatch(uint patchX, uint patchY)
    {
        assert(patchX >= 0);
        assert(patchY >= 0);
        assert(patchX < patchWidth);
        assert(patchY < patchHeight);
        return patches[patchY * patchWidth + patchX];
    }

    /// @return The patch at given (x,y) coordinates. Pass in values in range [0, PatchWidth()/PatchHeight[. Read only.
    const Patch &GetPatch(uint patchX, uint patchY) const
    {
        assert(patchX >= 0);
        assert(patchY >= 0);
        assert(patchX < patchWidth);
        assert(patchY < patchHeight);
        return patches[patchY * patchWidth + patchX];
    }

    /// Calculates and returns the vertex normal for the given terrain vertex.
    /// @param patchX The patch to read from, [0, PatchWidth()[.
    /// @param patchY The patch to read from, [0, PatchHeight()[.
    /// @param xinside The vertex inside the patch to compute the normal for, [0, cPatchSize[.
    /// @param yinside The vertex inside the patch to compute the normal for, [0, cPatchSize[.
    float3 CalculateNormal(uint x, uint y, uint xinside, uint yinside) const;

    float3 CalculateNormal(uint mapX, uint mapY) const { return CalculateNormal( (uint) mapX / cPatchSize, (uint) mapY / cPatchSize, mapX % cPatchSize, mapY % cPatchSize); }

public slots:
    /// Returns true if the given patch exists, i.e. whether the given coordinates are within the current terrain patch dimensions.
    /** This function does not tell whether the data for the patch is actually loaded on the CPU or the GPU. */
    bool PatchExists(uint patchX, uint patchY) const
    {
        return patchX >= 0 && patchY >= 0 && patchX < patchWidth && patchY < patchHeight && patchY * patchWidth + patchX < (int)patches.size();
    }

    /// Returns true if all the patches on the terrain are loaded on the CPU, i.e. if all the terrain height data has been streamed in from the server side.
    bool AllPatchesLoaded() const
    {
        for(uint y = 0; y < patchHeight; ++y)
            for(uint x = 0; x < patchWidth; ++x)
                if (!PatchExists(x,y) || GetPatch(x,y).heightData.size() == 0 || GetPatch(x,y).node == 0)
                    return false;

        return true;
    }

    /// Returns the height value on the given terrain grid point.
    /// @param x In the range [0, EC_Terrain::PatchWidth * EC_Terrain::cPatchSize [.
    /// @param y In the range [0, EC_Terrain::PatchHeight * EC_Terrain::cPatchSize [.
    float GetPoint(uint x, uint y) const;

    /// Sets a new height value to the given terrain map vertex. Marks the patch that vertex is part of dirty,
    /// but does not immediately recreate the GPU surfaces. Use the RegenerateDirtyTerrainPatches() function
    /// to regenerate the visible Ogre mesh geometry.
    void SetPointHeight(uint x, uint y, float height);
    
    /// Returns the point on the terrain in world space that lies on top of the given world space coordinate.
    /// @param point The point in world space to get the corresponding map point (in world space) for.
    float3 GetPointOnMap(const float3 &point) const;

    /// Returns the point on the terrain in local space that lies on top of the given world space coordinate.
    /// @param point The point in world space to get the corresponding map point (in local space of the terrain) for.
    float3 GetPointOnMapLocal(const float3 &point) const;

    /// Returns the signed distance (in world space) of the given point to the corresponding map point on the terrain.
    /// @param point The point in world space to test.
    /// @return The signed distance to the terrain. If the given point is above the terrain, the distance test returns a positive number.
    /// If the point is below the terrain, a negative number is returned. The above/below concept is measured along the
    /// local up axis of the terrain (not the global world space up axis).
    float GetDistanceToTerrain(const float3 &point) const;
    
    /// Returns true if given point in world space is on top (in terms of the local terrain up axis) of, or lying on, the terrain.
    bool IsOnTopOfMap(const float3 &point) const;

    /// Returns the interpolated height value of the terrain at the given fractional coordinate.
    /// @param x In the range [0, EC_Terrain::PatchWidth * EC_Terrain::cPatchSize-1.0f ].
    /// @param y In the range [0, EC_Terrain::PatchHeight * EC_Terrain::cPatchSize-1.0f ].
    float GetInterpolatedHeightValue(float x, float y) const;

    /// Returns the triangle and barycentric UV of the triangle at the given map coordinate, in local space.
    /// @param n1 [out]
    /// @param n2 [out]
    /// @param n3 [out]
    /// @param u [out]
    /// @param v [out]
    void GetTriangleNormals(float x, float y, float3 &n1, float3 &n2, float3 &n3, float &u, float &v) const;

    /// Returns the vertices and barycentric UV of the triangle at the given map coordinate, in local space.
    /// @param n1 [out]
    /// @param n2 [out]
    /// @param n3 [out]
    /// @param u [out]
    /// @param v [out]
    void GetTriangleVertices(float x, float y, float3 &v1, float3 &v2, float3 &v3, float &u, float &v) const;

    /// Computes the terrain plane normal (uninterpolated triangle normal) at the given map coordinate.
    float3 GetPlaneNormal(float x, float y) const;

    /// Returns the triangle normal of the given triangle on the map, given the local space 2D map coordinates for the point.
    /// The normal is returned in *world* space.
    float3 GetInterpolatedNormal(float x, float y) const;

    /// Helper function, which returns for given world coordinate point terrain rotation in Euler angles. 
    /// @note This assumes that "mesh" which is rotation for terrain is searched is orginally authored to look -y - axis.
    /// \todo This function will be deleted.
//    float3 GetTerrainRotationAngles(float x, float y, float z, const float3& direction) const;

    /// Returns the local terrain tangent frame at the given terrain world coordinates.
    /// @param worldPoint A point in the world space. This point is projected onto the terrain, and the tangent frame along
    ///     the terrain on that point is returned.
    float3x4 TangentFrame(const float3 &worldPoint) const;

    float3 Tangent(const float3 &worldPoint, const float3 &worldDir) const;

    /// Returns the local->world transform of the terrain.
    float3x4 WorldTransform() const;

    /// Removes all stored terrain patches and the associated Ogre scene nodes.
    void Destroy();

    /// Releases all GPU resources used for the given patch.
    void DestroyPatch(uint patchX, uint patchY);

    /// Makes all the vertices of the given patch flat with the given height value.
    /** Dirties the patch, but does not regenerate it. */
    void MakePatchFlat(uint patchX, uint patchY, float heightValue);

    /// Makes the whole terrain flat with the given height value.
    /** Dirties the whole terrain, but does not regenerate it. */
    void MakeTerrainFlat(float heightValue);

    /// Performs an 1D affine transform on the height values of the whole terrain, i.e. maps each vertex h' = scale*h + offset.
    /** Dirties the whole terrain, but does not regenerate it. */
    void AffineTransform(float scale, float offset);

    /// Affinely remaps the height values of the whole terrain in such a way that after the transform, the lowest point of
    /// the terrain has the height value of minHeight, and the highest point of the terrain has a height value of maxHeight.
    /// Dirties the whole terrain, but does not regenerate it.
    void RemapHeightValues(float minHeight, float maxHeight);

    /// Returns how many patches there are in the terrain in the x-direction.
    /// This differs from the xPatches attribute in that the PatchWidth tells how many patches there
    /// are *currently initialized*, whereas xPatches tells the number of patches wide the terrain is
    /// going to be, as soon as we get a chance to update the new geometry data.
    uint PatchWidth() const { return patchWidth; }

    /// Returns how many patches there are in the terrain in the y-direction.
    /// This value is understood in the similar way as above.
    uint PatchHeight() const { return patchHeight; }

    /// Returns the number of vertices in the whole terrain in the local X-direction.
    uint VerticesWidth() const { return PatchWidth() * cPatchSize; }

    /// Returns the number of vertices in the whole terrain in the local Y-direction.
    uint VerticesHeight() const { return PatchHeight() * cPatchSize; }

    /// Saves the height map data and the associated per-vertex attributes to a file. This is a binary
    /// dump file, and as a convention, use the file suffix ".ntf" for these.
    /// @return True if the save succeeded.
    bool SaveToFile(QString filename);

    /// Loads the terrain height map data from the given binary dump file (.ntf).
    /** You should prefer using the Attribute heightMap to recreate the terrain from a terrain file instead of calling this function directly,
        since this function only performs a local (hidden) change, whereas the heightMap attribute change is visible both
        locally and on the network.
        @note Calling this function will not update the 'heightMap' attribute. If you want to load the terrain from a file
            in such a way that is synchronized to the network, just set the 'heightMap' source attribute in a replicated way.
        @return True if loading succeeded. */
    bool LoadFromFile(QString filename);

    /// Loads the terrain height map data from the given in-memory .ntf file buffer.
    bool LoadFromDataInMemory(const char *data, size_t numBytes);

    void NormalizeImage(QString filename) const;

    /// Loads the terrain from the given image file.
    /** Adjusts the xPatches and yPatches properties to that of the image file, 
        and clears the heightMap source attribute. This function is intended to be used as a processing tool. Calling this  
        function will get the terrain contents desynchronized between the local system and network. The file is loaded using Ogre, so
        this supports all the file formats Ogre has codecs loaded for(you can see a list of those in the console at startup).
        Calling this function will regenerate all terrain patches on the GPU. */
    bool LoadFromImageFile(QString filename, float offset, float scale);

    /// Saves the terrain to an image file.
    /** The file format is determined from the suffix. The file is saved using Ogre, so
        this supports all the file formats Ogre has codecs loaded for(you can see a list of those in the console at startup).
        By default, the height range of the terrain is fitted into the [0.0, 1.0] range, i.e. the smallest position of the terrain
        gets the 0.0 grayscale pixel value in the image, and the largest position gets the 1.0 value.
        The file that is saved will be a three-channel color image, but will only contain grayscale values. */
    bool SaveToImageFile(QString filename, float minHeight = -1e9f, float maxHeight = 1e9f);

    /// Converts the given Ogre Mesh to a terrain grid.
    /** This function will raycast the contours of the mesh to generate the height map data points.
        Uses the default identity matrix for the transform.
        @note The Mesh resource must be previously loaded into the Ogre Mesh resource pool.
        Use AssetAPI::SanitateAssetRef() for the asset's name in order to generate the proper resource name.
        @param ogreMeshResourceName Internal Ogre resource name. */
    void GenerateFromOgreMesh(QString ogreMeshResourceName);

    /// @overload
    /** @param ogreMeshResourceName Internal Ogre resource name.
        @param transform Desired tranform for the generated terrain. */
    void GenerateFromOgreMesh(QString ogreMeshResourceName, const Ogre::Matrix4 &transform);

    /// Converts entity's mesh to a terrain grid.
    /** @note Then entity has to have EC_Name, EC_Mesh and EC_Placeable present.
        @param entityName Name of the entity. */
    void GenerateFromSceneEntity(QString entityName);

    /// Marks all terrain patches dirty.
    void DirtyAllTerrainPatches();

    void RegenerateDirtyTerrainPatches();

    /// Returns the minimum height value in the whole terrain.
    /** This function blindly iterates through the whole terrain, so avoid calling it in performance-critical code. */
    float GetTerrainMinHeight() const;

    /// Returns the maximum height value in the whole terrain.
    /** This function blindly iterates through the whole terrain, so avoid calling it in performance-critical code. */
    float GetTerrainMaxHeight() const;

    /// Resizes the terrain and recreates it.
    /// newWidth and newHeight are the size of the new terrain, in # patches.
    /// oldPatchStartX&Y specify the patch offset to copy the old terrain height values from.
    void Resize(uint newWidth, uint newHeight, uint oldPatchStartX = 0, uint oldPatchStartY = 0);

signals:
    /// Emitted when the terrain data is regenerated.
    void TerrainRegenerated();

private slots:
    /// Emitted when the parrent entity has been set.
    void UpdateSignals();

    void MaterialAssetLoaded(AssetPtr asset);
    void TerrainAssetLoaded(AssetPtr asset);

    /// (Re)checks whether this entity has EC_Placeable (or if it was just added or removed), and reparents the rootNode of this component to it or the scene root.
    /** Additionally re-applies the visibility of each terrain patch that is currently attached to the terrain node. */
    void AttachTerrainRootNode();

private:
    void AttributesChanged();

    /// Creates the patch parent/root node if it does not exist.
    /** After this function returns, the 'root' member node will exist, unless Ogre rendering subsystem fails. */
    void CreateRootNode();

    void CreateOgreTerrainPatchNode(Ogre::SceneNode *&node, uint patchX, uint patchY);

    /// Sets the given patch to use the currently set material and textures.
    void UpdateTerrainPatchMaterial(uint patchX, uint patchY);

    /// Updates the root node transform from the current attribute values, if the root node exists.
    void UpdateRootNodeTransform();

    /// Readjusts the terrain to contain the given number of patches in the horizontal and vertical directions.
    /** Preserves as much of the terrain height data as possible. Dirties the patches, but does not regenerate them.
        @note This function does not adjust the xPatches or yPatches attributes. */
    void ResizeTerrain(uint newPatchWidth, uint newPatchHeight);

    /// Updates the terrain material with the new texture on the given texture unit index.
    /// @param index The texture unit index to set the new texture to.
    /// @param textureName The Ogre texture resource name to set.
    void SetTerrainMaterialTexture(uint index, const QString &textureName);

    /// Creates Ogre geometry data for the single given patch, or updates the geometry for an existing
    /// patch if the associated Ogre resources already exist.
    void GenerateTerrainGeometryForOnePatch(uint patchX, uint patchY);

    shared_ptr<AssetRefListener> heightMapAsset;

    /// For all terrain patches, we maintain a global parent/root node to be able to transform the whole terrain at one go.
    Ogre::SceneNode *rootNode;

    uint patchWidth;
    uint patchHeight;

    /// Specifies the asset source from which the height map is currently loaded from. Used to shadow the heightMap attribute so that if
    /// the same value is received from the network, reloading the terrain can be avoided.
    QString currentHeightmapAssetSource;

    /// Specifies the Ogre material name of the material that is currently being used to display the terrain.
    QString currentMaterial;

    /// Stores the actual height patches.
    std::vector<Patch> patches;
    
    /// Ogre world for referring to the Ogre scene manager
    OgreWorldWeakPtr world_;
};
