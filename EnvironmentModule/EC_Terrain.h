// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_TerrainPatch_h
#define incl_EC_TerrainPatch_h

#include "EnvironmentModuleApi.h"
#include "IComponent.h"
#include "Declare_EC.h"
#include "Vector3D.h"
#include "Transform.h"

namespace Ogre
{
    class SceneNode;
    class Entity;
}

namespace Environment
{
	/**

<table class="header">

<tr>
<td>
<h2>Terrain</h2>
Stores all the terrain patches (typically 16x16) in a given terrain.

Registered by Environment::EnvironmentModule.

<b>No Attributes</b>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Doesn't Depend on any components</b>. 
</table>

*/
/// Stores all the terrain patches (typically 16x16) in a given terrain.
class ENVIRONMENT_MODULE_API EC_Terrain : public IComponent
{
    Q_OBJECT

    DECLARE_EC(EC_Terrain);
public:
    virtual ~EC_Terrain();

    virtual bool IsSerializable() const { return true; }

    /// Each patch is a square containing this many vertices per side.
    static const int cPatchSize = 16;

    /// Describes a single patch that is present in the scene. A patch can be in one of the following three states:
    /// - not loaded. The height data nor the GPU data is present, but the Patch struct itself is initialized. heightData.size() == 0, node == entity == 0. meshGeometryName == "".
    /// - heightmap data loaded. The heightData vector contains the heightmap data, but the visible GPU vertex data itself has not been generated yet, due to the neighbors
    ///   of this patch not being present yet. node == entity == 0, meshGeometryName == "". patch_geometry_dirty == true.
    /// - fully loaded. The GPU data is also loaded and the node, entity and meshGeometryName fields specify the used GPU resources.
    struct Patch
    {
        Patch():x(0),y(0), node(0), entity(0), patch_geometry_dirty(true) {}

        /// X-coordinate on the grid of patches. In the range [0, EC_Terrain::PatchWidth()].
        int x;

        /// Y-coordinate on the grid of patches. In the range [0, EC_Terrain::PatchHeight()].
        int y;

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
        float GetHeightValue(int x, int y) const { return heightData[y*cPatchSize+x]; }
    };
    
    /// @return The patch at given (x,y) coordinates. Pass in values in range [0, PatchWidth()/PatchHeight[.
    Patch &GetPatch(int patchX, int patchY)
    {
        assert(patchX >= 0);
        assert(patchY >= 0);
        assert(patchX < patchWidth);
        assert(patchY < patchHeight);
        return patches[patchY * patchWidth + patchX];
    }

    /// @return The patch at given (x,y) coordinates. Pass in values in range [0, PatchWidth()/PatchHeight[. Read only.
    const Patch &GetPatch(int patchX, int patchY) const
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
    /// @param vertexX The vertex inside the patch to compute the normal for, [0, cPatchSize[.
    /// @param vertexY The vertex inside the patch to compute the normal for, [0, cPatchSize[.
    Vector3df CalculateNormal(int patchX, int patchY, int vertexX, int vertexY);

public slots:
    /// Returns true if the given patch exists, i.e. whether the given coordinates are within the current terrain patch dimensions.
    /// This function does not tell whether the data for the patch is actually loaded on the CPU or the GPU.
    bool PatchExists(int patchX, int patchY) const
    {
        return patchX >= 0 && patchY >= 0 && patchX < patchWidth && patchY < patchHeight && patchY * patchWidth + patchX < patches.size();
    }

    /// Returns true if all the patches on the terrain are loaded on the CPU, i.e. if all the terrain height data has been streamed in from
    /// the server side.
    bool AllPatchesLoaded() const
    {
        for(int y = 0; y < patchHeight; ++y)
            for(int x = 0; x < patchWidth; ++x)
                if (!PatchExists(x,y) || GetPatch(x,y).heightData.size() == 0 || GetPatch(x,y).node == 0)
                    return false;

        return true;
    }

    /// Returns the height value on the given terrain grid point.
    /// @param x In the range [0, EC_Terrain::PatchWidth * EC_Terrain::cPatchSize [.
    /// @param y In the range [0, EC_Terrain::PatchHeight * EC_Terrain::cPatchSize [.
    float GetPoint(int x, int y) const;

    /// Returns the interpolated height value of the terrain at the given fractional coordinate.
    /// @param x In the range [0, EC_Terrain::PatchWidth * EC_Terrain::cPatchSize-1.0f ].
    /// @param y In the range [0, EC_Terrain::PatchHeight * EC_Terrain::cPatchSize-1.0f ].
    float GetInterpolatedHeightValue(float x, float y) const;

    /// Removes all stored terrain patches and the associated Ogre scene nodes.
    void Destroy();

    /// Releases all GPU resources used for the given patch.
    void DestroyPatch(int patchX, int patchY);

    /// Makes all the vertices of the given patch flat with the given height value.
    void MakePatchFlat(int patchX, int patchY, float heightValue);

    /// Returns how many patches there are in the terrain in the x-direction.
    /// This differs from the xPatches attribute in that the PatchWidth tells how many patches there
    /// are *currently initialized*, whereas xPatches tells the number of patches wide the terrain is
    /// going to be, as soon as we get a chance to update the new geometry data.
    int PatchWidth() const { return patchWidth; }

    /// Returns how many patches there are in the terrain in the y-direction.
    /// This value is understood in the similar way as above.
    int PatchHeight() const { return patchHeight; }

public:
    Q_PROPERTY(Transform nodeTransformation READ getnodeTransformation WRITE setnodeTransformation NOTIFY TransformChanged);
    DEFINE_QPROPERTY_ATTRIBUTE(Transform, nodeTransformation);

    Q_PROPERTY(int xPatches READ getxPatches WRITE setxPatches NOTIFY TerrainSizeChanged);
    DEFINE_QPROPERTY_ATTRIBUTE(int, xPatches);

    Q_PROPERTY(int yPatches READ getyPatches WRITE setyPatches NOTIFY TerrainSizeChanged);
    DEFINE_QPROPERTY_ATTRIBUTE(int, yPatches);

    Q_PROPERTY(QString material READ getmaterial WRITE setmaterial NOTIFY MaterialChanged);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, material);

    Q_PROPERTY(QString texture0 READ gettexture0 WRITE settexture0 NOTIFY TextureChanged);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, texture0);

    Q_PROPERTY(QString texture1 READ gettexture1 WRITE settexture1 NOTIFY TextureChanged);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, texture1);

    Q_PROPERTY(QString texture2 READ gettexture2 WRITE settexture2 NOTIFY TextureChanged);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, texture2);

    Q_PROPERTY(QString texture3 READ gettexture3 WRITE settexture3 NOTIFY TextureChanged);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, texture3);

    Q_PROPERTY(QString texture4 READ gettexture4 WRITE settexture4 NOTIFY TextureChanged);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, texture4);

    void RegenerateDirtyTerrainPatches();

    /// Returns the minimum and maximum extents of terrain heights.
    void GetTerrainHeightRange(float &minHeight, float &maxHeight);

signals:
    // The following signals are emitted from different events, so the parameter can not be used.
    void TerrainSizeChanged(int /*newSize*/);
    void TextureChanged(QString /*newTexture*/);

    void MaterialChanged(QString newMaterial);

private slots:
    //! Emitted when the parrent entity has been set.
    void UpdateSignals();

    //! Emitted when some of the attributes has been changed.
    void AttributeUpdated(IAttribute *attribute);

public slots:

    void OnTerrainSizeChanged();

    void OnMaterialChanged();

    void OnTextureChanged();

private:
    explicit EC_Terrain(IModule* module);

    /// Creates the patch parent/root node if it does not exist. After this function returns, the 'root' member node will exist, unless
    /// Ogre rendering subsystem fails.
    void CreateRootNode();

    /// For all terrain patches, we maintain a global parent/root node to be able to transform the whole terrain at one go.
    ///\todo Perhaps this should be unified with the ECOgrePlaceable.
    Ogre::SceneNode *rootNode;

    int patchWidth;

    int patchHeight;

    /// Stores the actual height patches.
    std::vector<Patch> patches;

    void CreateOgreTerrainPatchNode(Ogre::SceneNode *&node, int patchX, int patchY);

    /// Sets the given patch to use the currently set material and textures.
    void UpdateTerrainPatchMaterial(int patchX, int patchY);

    void ResizeTerrain(int newPatchWidth, int newPatchHeight);

    /// Updates the terrain material with the new texture on the given texture unit index.
    /// @param index The texture unit index to set the new texture to.
    /// @param textureName The Ogre texture resource name to set.
    void SetTerrainMaterialTexture(int index, const char *textureName);

    void GenerateTerrainGeometryForOnePatch(int patchX, int patchY);
};
}

#endif
