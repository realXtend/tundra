// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include <Ogre.h>

#include "XMLUtilities.h"

#include <QDomDocument>

using namespace RexTypes;

namespace OgreRenderer
{
    EC_OgrePlaceable::EC_OgrePlaceable(Foundation::ModuleInterface* module) :
        Foundation::ComponentInterface(module->GetFramework()),
        renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
        scene_node_(0),
        link_scene_node_(0),
        attached_(false),
        select_priority_(0)
    {
        RendererPtr renderer = renderer_.lock();      
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        link_scene_node_ = scene_mgr->createSceneNode();
        scene_node_ = scene_mgr->createSceneNode();
        link_scene_node_->addChild(scene_node_);
        
        // In case the placeable is used for camera control, set fixed yaw axis
        link_scene_node_->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);            
    }
    
    EC_OgrePlaceable::~EC_OgrePlaceable()
    {
        if (renderer_.expired())
            return;
        RendererPtr renderer = renderer_.lock();  
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
                        
        if (scene_node_ && link_scene_node_)
        {
            link_scene_node_->removeChild(scene_node_);
        }
        
        if (scene_node_)
        {
            scene_mgr->destroySceneNode(scene_node_);
            scene_node_ = 0;
        }
        
        if (link_scene_node_)
        {
            DetachNode();
            
            scene_mgr->destroySceneNode(link_scene_node_);
            link_scene_node_ = 0;
        }
    }
    
    void EC_OgrePlaceable::SetParent(Foundation::ComponentPtr placeable)
    {
        if ((placeable.get() != 0) && (!dynamic_cast<EC_OgrePlaceable*>(placeable.get())))
        {
            OgreRenderingModule::LogError("Attempted to set parent placeable which is not " + TypeNameStatic());
            return;
        }
        DetachNode();
        parent_ = placeable;
        AttachNode();
    }
    
    void EC_OgrePlaceable::SerializeTo(QDomDocument& doc, QDomElement& base_element) const
    {
        QDomElement comp_element = BeginSerialization(doc, base_element);
        WriteAttribute(doc, comp_element, "position", WriteVector3(GetPosition()));
        WriteAttribute(doc, comp_element, "orientation", WriteQuaternion(GetOrientation()));
        WriteAttribute(doc, comp_element, "scale", WriteVector3(GetScale()));
    }
    
    void EC_OgrePlaceable::DeserializeFrom(QDomElement& element)
    {
        // Check that type is right, otherwise do nothing
        if (!BeginDeserialization(element))
            return;
        
        Vector3df pos = ParseVector3(ReadAttribute(element, "position"));
        Quaternion orient = ParseQuaternion(ReadAttribute(element, "orientation"));
        Vector3df scale = ParseVector3(ReadAttribute(element, "scale"));
        
        SetPosition(pos);
        SetOrientation(orient);
        SetScale(scale);
        
        OnChanged();
    }
    
    Vector3df EC_OgrePlaceable::GetPosition() const
    {
        const Ogre::Vector3& pos = link_scene_node_->getPosition();
        return Vector3df(pos.x, pos.y, pos.z);
    }

    Quaternion EC_OgrePlaceable::GetOrientation() const
    {
        const Ogre::Quaternion& orientation = link_scene_node_->getOrientation();
        return Quaternion(orientation.x, orientation.y, orientation.z, orientation.w);
    }
    
    Vector3df EC_OgrePlaceable::GetScale() const
    {
        const Ogre::Vector3& scale = scene_node_->getScale();
        return Vector3df(scale.x, scale.y, scale.z);
    }
    
    void EC_OgrePlaceable::SetPosition(const Vector3df& position)
    {
        link_scene_node_->setPosition(Ogre::Vector3(position.x, position.y, position.z));
        AttachNode(); // Nodes become visible only after having their position set at least once
    }

    void EC_OgrePlaceable::SetOrientation(const Quaternion& orientation)
    {
        link_scene_node_->setOrientation(Ogre::Quaternion(orientation.w, orientation.x, orientation.y, orientation.z));
    }

    void EC_OgrePlaceable::LookAt(const Vector3df& look_at)
    {
        // Don't rely on the stability of the lookat (since it uses previous orientation), 
        // so start in identity transform
        link_scene_node_->setOrientation(Ogre::Quaternion::IDENTITY);
        link_scene_node_->lookAt(Ogre::Vector3(look_at.x, look_at.y, look_at.z), Ogre::Node::TS_WORLD);        
    }
    
    void EC_OgrePlaceable::Yaw(Real radians)
    {
        link_scene_node_->yaw(Ogre::Radian(radians), Ogre::Node::TS_WORLD);
    }

    void EC_OgrePlaceable::Pitch(Real radians)
    {
        link_scene_node_->pitch(Ogre::Radian(radians));
    }
 
   void EC_OgrePlaceable::Roll(Real radians)
    {
        link_scene_node_->roll(Ogre::Radian(radians));
    } 
    
    void EC_OgrePlaceable::SetScale(const Vector3df& scale)
    {
        scene_node_->setScale(Ogre::Vector3(scale.x, scale.y, scale.z));
    }       

    void EC_OgrePlaceable::AttachNode()
    {
        if (renderer_.expired())
            return;
        RendererPtr renderer = renderer_.lock();  
            
        if (attached_)
            return;
                
        Ogre::SceneNode* parent_node;
        
        if (!parent_)
        {
            Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
            parent_node = scene_mgr->getRootSceneNode();
        }
        else
        {
            EC_OgrePlaceable* parent = checked_static_cast<EC_OgrePlaceable*>(parent_.get());
            parent_node = parent->GetLinkSceneNode();
        }
        
        parent_node->addChild(link_scene_node_);
        attached_ = true;
    }
    
    void EC_OgrePlaceable::DetachNode()
    {
        if (renderer_.expired())
            return;
        RendererPtr renderer = renderer_.lock();  
            
        if (!attached_)
            return;
            
        Ogre::SceneNode* parent_node;
        
        if (!parent_)
        {
            Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
            parent_node = scene_mgr->getRootSceneNode();
        }
        else
        {
            EC_OgrePlaceable* parent = checked_static_cast<EC_OgrePlaceable*>(parent_.get());
            parent_node = parent->GetLinkSceneNode();
        }
        
        parent_node->removeChild(link_scene_node_);
        attached_ = false;
    }

    //experimental QVector3D acessors
    QVector3D EC_OgrePlaceable::GetQPosition() const
    {
        //conversions, conversions, all around
        //.. if this works, and QVector3D is good, we should consider porting Vector3df for that
        Vector3df rexpos = GetPosition();
        return QVector3D(rexpos.x, rexpos.y, rexpos.z);
    }
    
    void EC_OgrePlaceable::SetQPosition(const QVector3D newpos)
    {
        SetPosition(Vector3df(newpos.x(), newpos.y(), newpos.z()));
    }
    

    QQuaternion EC_OgrePlaceable::GetQOrientation() const 
    {
        Quaternion rexort = GetOrientation();
        return QQuaternion(rexort.w, rexort.x, rexort.y, rexort.z);
    }

    void EC_OgrePlaceable::SetQOrientation(const QQuaternion newort)
    {
        SetOrientation(Quaternion(newort.x(), newort.y(), newort.z(), newort.scalar()));
    }

    
    QVector3D EC_OgrePlaceable::GetQScale() const
    {
        Vector3df rexscale = GetScale();
        return QVector3D(rexscale.x, rexscale.y, rexscale.z);
    }

    void EC_OgrePlaceable::SetQScale(const QVector3D newscale)
    {
        SetScale(Vector3df(newscale.x(), newscale.y(), newscale.z()));
    }
}
