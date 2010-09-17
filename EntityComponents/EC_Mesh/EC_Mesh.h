// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Mesh_EC_Mesh_h
#define incl_EC_Mesh_EC_Mesh_h

#include "ComponentInterface.h"
#include "AttributeInterface.h"
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

/// Makes the mesh component.
/**

<table style="margin: 20px;">
<tr>
<td style="width:500px; height: 100px; border: solid 1px black; background-color: #e0d0ff; vertical-align: top; padding: 5px;">
<h2>Mesh</h2>
Makes the mesh component.

User can apply new mesh to scene and change it's materials and skeleton.
EC_Mesh component can be used to set mesh to a world scene. There are options to apply new materials and skeleton to mesh.

Note! Animation or attachments are not tested with this component as yet.

Note! if you are planning to remove the skeleton and want that it's realy gone you should relogin to the server.
that is cause Ogre doesn't have a serivice that will remove applied skeleton from the mesh so we need to recreate
the mesh to get the change shown.

<b>Attributes</b>:
<ul>
<li>Transform: Transform.
<div style="margin: 5px;">Transformation attribute is used to do some position, rotation and scale adjustments.</div> 
<li>QString: Mesh ref. 
<div style="margin: 5px;">Mesh resource ref is a asset id for a mesh resource that user wants to apply.</div> 
<li>QString: Skeleton ref.
<div style="margin: 5px;">Skeleton asset ref is a asset id for a mesh resource mesh will use.</div> 
<li>QVariantList: Mesh materials.
<div style="margin: 5px;">Mesh material ref list that can contain x number of materials.</div> 
<li>float: Draw distance.
<div style="margin: 5px;">Mesh draw distance.</div> 
<li>bool: Cast shadows.
<div style="margin: 5px;">Will the mesh cast shadows.</div> 
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>void SetMesh(const QString &name);
<div style="margin: 5px;">Add or change entity's mesh.</div> 
<li>void RemoveMesh();
<div style="margin: 5px;">Remove mesh from entity.</div> 
<li>bool SetMaterial(uint index, const QString &material_name);
<div style="margin: 5px;">Set material to wanted subMesh.</div> 
<li>void AttachSkeleton(const QString &skeletonName);
<div style="margin: 5px;">Attach skeleton to entity.</div> 
</ul>
<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component OgrePlaceable</b>. The position in the OgrePlaceable component specifies the position in the world space where this mesh is placed at. 
</table>

*/
class EC_Mesh : public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_Mesh);
    
public:
    //! Destructor.
    ~EC_Mesh();

    virtual bool IsSerializable() const { return true; }

    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData *data);

    //! Transformation attribute is used to do some position, rotation and scale adjustments.
    DEFINE_QPROPERTY_ATTRIBUTE(Transform, nodeTransformation);

    //! Mesh resource id is a asset id for a mesh resource that user wants to apply (Will handle resource request automaticly).
    DEFINE_QPROPERTY_ATTRIBUTE(QString, meshResourceId);

    //! Skeleton asset id, will handle request resource automaticly.
    DEFINE_QPROPERTY_ATTRIBUTE(QString, skeletonId);

    //! Mesh material id list that can contain x number of materials, material requests are handled automaticly.
    //! @todo replace std::vector to QVariantList.
    DEFINE_QPROPERTY_ATTRIBUTE(QVariantList, meshMaterial);

    //! Mesh draw distance.
    DEFINE_QPROPERTY_ATTRIBUTE(float, drawDistance);

    //! Will the mesh cast shadows.
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
    //! Emitted when the parrent entity has been setted.
    void UpdateSignals();

    //! Emitted when some of the attributes has been changed.
    void AttributeUpdated(Foundation::ComponentInterface *component, AttributeInterface *attribute);

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
    
    Foundation::ComponentPtr FindPlaceable() const;
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