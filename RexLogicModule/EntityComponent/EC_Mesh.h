// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_EC_Mesh_h
#define incl_RexLogicModule_EC_Mesh_h

#include "ComponentInterface.h"
#include "AttributeInterface.h"
#include "Declare_EC.h"
#include "CoreTypes.h"
#include "RexTypes.h"
#include "ResourceInterface.h"
#include "Quaternion.h"

#include <QVariant>

namespace Ogre
{
    class Entity;
    class Mesh;
    //class Node;
    class SceneNode;
}

namespace OgreRenderer
{
    class Renderer;
    class OgreMeshResource;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;
}

class EC_Mesh : public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_Mesh);
    
public:
    //! Destructor.
    ~EC_Mesh();

    virtual bool IsSerializable() const { return true; }

    //! Gets placeable component
    Foundation::ComponentPtr GetPlaceable() const { return placeable_; }
    
    //! Sets placeable component
    void SetPlaceable(Foundation::ComponentPtr placeable);

    //! Don't use this unless you know what you are doing.
    bool HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data);

    //! Add or change entity's mesh.
    void SetMesh(const std::string &name, bool clone = false);
    void RemoveMesh();
    bool SetMaterial(uint index, const std::string& material_name);

    Foundation::Attribute<std::string> meshResouceId_;
    Foundation::Attribute<std::vector<QVariant> > meshMaterial_;
    Foundation::Attribute<Real> drawDistance_;
    Foundation::Attribute<bool> castShadows_;

private slots:
    //! Emitted when the parrent entity has been setted.
    void UpdateSignals();
    //! Emitted when some of the attributes has been changed.
    void AttributeUpdated(Foundation::ComponentInterface *component, Foundation::AttributeInterface *attribute);

signals:
    void OnMeshChanged();
    void OnMaterialChanged(uint index, const std::string &material_name);

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
    //! handles mesh ready event.
    void HandleMeshReady(Foundation::ResourcePtr res);
    //! handles material ready event.
    void HandleMaterialReady(uint index, Foundation::ResourcePtr res);
    
    //! Checks if component's and entity's materials differ from each other
    /*! so that we can ensure that material resources are only requested when materials have actually changed.
     *! @return Return true if materials differ from each other.
     */
    bool HasMaterialsChanged() const;
    //! Search if component is holding EC_OgrePlacable component.
    void FindPlaceableComponent();
    //! Attach a new entity to scene node that world scene owns.
    void AttachEntity();
    //! Detach entity from the scene node.
    void DetachEntity();

    Foundation::ComponentPtr placeable_;
    Foundation::Framework *framework_;
    OgreRenderer::RendererWeakPtr renderer_;
    typedef std::set<request_tag_t> AssetRequestTags;
    AssetRequestTags requestTags_;
    typedef std::vector<request_tag_t> AssetRequestArray;
    AssetRequestArray materialRequestTags_;
    //! Ogre mesh entity
    Ogre::Entity* entity_;
    //! adjustment scene node (scaling/offset/orientation modifications)
    Ogre::SceneNode* node_;
    //! Has mesh attach to a scene.
    bool attached_;
};

#endif