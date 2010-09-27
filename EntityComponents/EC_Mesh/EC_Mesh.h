// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Mesh_EC_Mesh_h
#define incl_EC_Mesh_EC_Mesh_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "CoreTypes.h"
#include "RexTypes.h"
#include "ResourceInterface.h"
#include "Quaternion.h"
#include "Transform.h"

#include <QVariant>

namespace Ogre
{
    class Entity;
    class SceneNode;
    class Skeleton;
}

namespace OgreRenderer
{
    class Renderer;
    class OgreMeshResource;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;
}

/**

<table class="header">
<tr>
<td>
<h2>Mesh</h2>
EC_Mesh component can be used to set mesh to a world scene. There are options to apply new materials and skeleton to mesh.
To use EC_Mesh user need to upload a mesh, material and skeleton (optional) assets to the server or mesh wont be diplayed on scene.
If you aren't sure switch mesh is the skeleton made for don't try to apply it randombly on some mesh, cause this might cause a crash.

Note if you are planning to remove the skeleton you need to relogin to the server.

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>
<li>Transform: nodePosition
<div>Transformation attribute is used to do some position, rotation and scale adjustments.</div>
<li>QString: meshResouceId
<div>Mesh resource ref is a asset id for a mesh resource that user wants to apply to scene.</div>
<li>QString: skeletonId
<div>Skeleton asset ref is a string that should contain skeleton asset id.</div>
<li>QVariantList: meshMaterial
<div>Mesh material ref is a string list that can contain x number of materials and each material is applied to.</div> 
<li>float: drawDistance
<div>Distance where the mesh is shown from the camera.</div> 
<li>bool: castShadows
<div>Will the mesh cast shadows.</div> 
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"SetMesh": Add or change entity's mesh.
<li>"RemoveMesh": Remove mesh from entity.
<li>"SetMaterial": Set material to a wanted subMesh.
<li>"AttachSkeleton": Attach skeleton to entity.
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component OgrePlaceable</b>.  
</table>
*/
class EC_Mesh : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_Mesh);
    
public:
    //! Destructor.
    ~EC_Mesh();

    virtual bool IsSerializable() const { return true; }

    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData *data);

    //! Transformation attribute is used to do some position, rotation and scale adjustments.
    //! @todo Transform attribute is not working in js need to expose it to QScriptEngine somehow.
    Q_PROPERTY(Transform nodeTransformation READ getnodeTransformation WRITE setnodeTransformation);
    DEFINE_QPROPERTY_ATTRIBUTE(Transform, nodeTransformation);

    //! Mesh resource id is a asset id for a mesh resource that user wants to apply (Will handle resource request automaticly).
    Q_PROPERTY(QString meshResourceId READ getmeshResourceId WRITE setmeshResourceId);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, meshResourceId);

    //! Skeleton asset id, will handle request resource automaticly.
    Q_PROPERTY(QString skeletonId READ getskeletonId WRITE setskeletonId);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, skeletonId);

    //! Mesh material id list that can contain x number of materials, material requests are handled automaticly.
    //! @todo replace std::vector to QVariantList.
    Q_PROPERTY(QVariantList meshMaterial READ getmeshMaterial WRITE setmeshMaterial);
    DEFINE_QPROPERTY_ATTRIBUTE(QVariantList, meshMaterial);

    //! Mesh draw distance.
    Q_PROPERTY(float drawDistance READ getdrawDistance WRITE setdrawDistance);
    DEFINE_QPROPERTY_ATTRIBUTE(float, drawDistance);

    //! Will the mesh cast shadows.
    Q_PROPERTY(bool castShadows READ getcastShadows WRITE setcastShadows);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, castShadows);

public slots:
    //! Add or change entity's mesh.
    /*! @param name Ogre mesh resource name.
     */
    void SetMesh(const QString &name);

    //! Remove mesh from entity.
    void RemoveMesh();

    //! Set material to wanted subMesh.
    /*! @param index a sub mesh index that material is attached to.
     *  @param material_name Ogre material name (assumming that ogre have that material loaded).
     *  @return did we successfully added a new material to a sub mesh.
     */
    bool SetMaterial(uint index, const QString &material_name);

    //! Attach skeleton to entity.
    /*! @param skeletonName Ogre skeleton resource name.
     */
    void AttachSkeleton(const QString &skeletonName);

signals:
    //! Singal is emmitted when mesh has successfully loaded and applyed to entity.
    void OnMeshChanged();

    //! Signal is emmitted when material has succussfully applyed to sub mesh.
    void OnMaterialChanged(uint index, const QString &material_name);

    //! Signal is emmitted when skeleton has successfully applied to entity.
    void OnSkeletonChanged(QString skeleton_name);

private slots:
    //! Emitted when the parrent entity has been set.
    void UpdateSignals();

    //! Emitted when some of the attributes has been changed.
    void AttributeUpdated(IAttribute *attribute);

private:
    //! Constuctor.
    /*! \param module Module.
     */
    explicit EC_Mesh(IModule *module);
    //! Request asset using it's resource id and resource type. If resource has been found at the renderer service return it's tag to user.
    /*! @param id Resource id.
     *  @param type Resource type.
     *  @return requset tag, null if the resource was not found.
     */
    request_tag_t RequestResource(const std::string& id, const std::string& type);
    
    ComponentPtr FindPlaceable() const;
    //! Checks if component's and entity's materials differ from each other
    /*! so that we can ensure that material resources are only requested when materials have actually changed.
     *! @return Return true if materials differ from each other.
     */
    bool HasMaterialsChanged() const;
    //! Attach a new entity to scene node that world scene owns.
    void AttachEntity();
    //! Detach entity from the scene node.
    void DetachEntity();
    
    bool HandleResourceEvent(event_id_t event_id, IEventData* data);
    bool HandleMeshResourceEvent(event_id_t event_id, IEventData* data);
    bool HandleSkeletonResourceEvent(event_id_t event_id, IEventData* data);
    bool HandleMaterialResourceEvent(event_id_t event_id, IEventData* data);

    OgreRenderer::RendererWeakPtr renderer_;
    Ogre::Entity* entity_;
    Ogre::SceneNode* node_;
    Ogre::Skeleton* skeleton_;
    bool attached_;
    event_category_id_t resource_event_category_;

    typedef std::vector<request_tag_t> AssetRequestArray;
    AssetRequestArray materialRequestTags_;

    typedef std::pair<request_tag_t, std::string> ResourceKeyPair;
    typedef boost::function<bool(event_id_t,IEventData*)> MeshEventHandlerFunction;
    typedef std::map<ResourceKeyPair, MeshEventHandlerFunction> MeshResourceHandlerMap;
    MeshResourceHandlerMap resRequestTags_;
};

#endif