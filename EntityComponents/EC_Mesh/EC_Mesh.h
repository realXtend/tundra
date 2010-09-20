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

/**

<table class="header">
<tr>
<td>
<h2>Mesh</h2>
Note if you are planning to remove the skeleton you need to relogin to the server.

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>
<li>Transform: nodePosition_
<div></div> meshResouceId_
<li>QString: skeletonId_
<div></div> meshMaterial_
<li>QString: 
<div></div> 
<li>std::vector<QVariant>: 
<div></div> 
<li>float: drawDistance_
<div></div> 
<li>bool: castShadows_
<div></div> 
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"SetMesh": Add or change entity's mesh.
<li>"RemoveMesh": 
<li>"SetMaterial": 
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
//! Note if you are planning to remove the skeleton you need to relogin to the server.
class EC_Mesh : public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_Mesh);
    
public:
    //! Destructor.
    ~EC_Mesh();

    virtual bool IsSerializable() const { return true; }
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface *data);

    Attribute<Transform> nodePosition_;
    Attribute<QString> meshResouceId_;
    Attribute<QString> skeletonId_;
    Attribute<std::vector<QVariant> > meshMaterial_;
    Attribute<float> drawDistance_;
    Attribute<bool> castShadows_;

public slots:
    //! Add or change entity's mesh.
    void SetMesh(const QString &name);
    void RemoveMesh();
    bool SetMaterial(uint index, const QString &material_name);
    //! Attach skeleton to entity.
    void AttachSkeleton(const QString &skeletonName);

private slots:
    //! Emitted when the parrent entity has been setted.
    void UpdateSignals();
    //! Emitted when some of the attributes has been changed.
    void AttributeUpdated(Foundation::ComponentInterface *component, AttributeInterface *attribute);

signals:
    void OnMeshChanged();
    void OnMaterialChanged(uint index, const QString &material_name);
    void OnSkeletonChanged(QString skeleton_name);
    

private:
    //! Constuctor.
    /*! \param module Module.
     */
    explicit EC_Mesh(Foundation::ModuleInterface *module);
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
    
    bool HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data);
    bool HandleMeshResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data);
    bool HandleSkeletonResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data);
    bool HandleMaterialResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data);

    OgreRenderer::RendererWeakPtr renderer_;
    Ogre::Entity* entity_;
    Ogre::SceneNode* node_;
    Ogre::Skeleton* skeleton_;
    bool attached_;
    event_category_id_t resource_event_category_;

    typedef std::vector<request_tag_t> AssetRequestArray;
    AssetRequestArray materialRequestTags_;

    typedef std::pair<request_tag_t, std::string> ResourceKeyPair;
    typedef boost::function<bool(event_id_t,Foundation::EventDataInterface*)> MeshEventHandlerFunction;
    typedef std::map<ResourceKeyPair, MeshEventHandlerFunction> MeshResourceHandlerMap;
    MeshResourceHandlerMap resRequestTags_;
};

#endif