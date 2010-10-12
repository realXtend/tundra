// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "Entity.h"

#include <Ogre.h>
#include <QDebug>

using namespace OgreRenderer;

EC_Placeable::EC_Placeable(IModule* module) :
    IComponent(module->GetFramework()),
    renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
    scene_node_(0),
    link_scene_node_(0),
    attached_(false),
    select_priority_(0),
    transform(this, "Transform")
{
    RendererPtr renderer = renderer_.lock();
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    link_scene_node_ = scene_mgr->createSceneNode();
    scene_node_ = scene_mgr->createSceneNode();
    link_scene_node_->addChild(scene_node_);
    
    // In case the placeable is used for camera control, set fixed yaw axis
    link_scene_node_->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);

    // Hook the transform attribute change
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
        SLOT(HandleAttributeChanged(IAttribute*, AttributeChange::Type)));

    connect(this, SIGNAL(ParentEntitySet()), SLOT(RegisterActions()));
}

EC_Placeable::~EC_Placeable()
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

void EC_Placeable::SetParent(ComponentPtr placeable)
{
    if ((placeable.get() != 0) && (!dynamic_cast<EC_Placeable*>(placeable.get())))
    {
        OgreRenderingModule::LogError("Attempted to set parent placeable which is not " + TypeNameStatic().toStdString());
        return;
    }
    DetachNode();
    parent_ = placeable;
    AttachNode();
}

Vector3df EC_Placeable::GetPosition() const
{
    const Ogre::Vector3& pos = link_scene_node_->getPosition();
    return Vector3df(pos.x, pos.y, pos.z);
}

Quaternion EC_Placeable::GetOrientation() const
{
    const Ogre::Quaternion& orientation = link_scene_node_->getOrientation();
    return Quaternion(orientation.x, orientation.y, orientation.z, orientation.w);
}

Vector3df EC_Placeable::GetScale() const
{
    const Ogre::Vector3& scale = scene_node_->getScale();
    return Vector3df(scale.x, scale.y, scale.z);
}

Vector3df EC_Placeable::GetLocalXAxis() const
{
    const Ogre::Vector3& xaxis = link_scene_node_->getOrientation().xAxis();
    return Vector3df(xaxis.x, xaxis.y, xaxis.z);
}

QVector3D EC_Placeable::GetQLocalXAxis() const
{
    Vector3df xaxis= GetLocalXAxis();
    return QVector3D(xaxis.x, xaxis.y, xaxis.z);
}

Vector3df EC_Placeable::GetLocalYAxis() const
{
    const Ogre::Vector3& yaxis = link_scene_node_->getOrientation().yAxis();
    return Vector3df(yaxis.x, yaxis.y, yaxis.z);
}

QVector3D EC_Placeable::GetQLocalYAxis() const
{
    Vector3df yaxis= GetLocalYAxis();
    return QVector3D(yaxis.x, yaxis.y, yaxis.z);
}

Vector3df EC_Placeable::GetLocalZAxis() const
{
    const Ogre::Vector3& zaxis = link_scene_node_->getOrientation().zAxis();
    return Vector3df(zaxis.x, zaxis.y, zaxis.z);
}

QVector3D EC_Placeable::GetQLocalZAxis() const
{
    Vector3df zaxis= GetLocalZAxis();
    return QVector3D(zaxis.x, zaxis.y, zaxis.z);
}

void EC_Placeable::SetPosition(const Vector3df& position)
{
    link_scene_node_->setPosition(Ogre::Vector3(position.x, position.y, position.z));
    AttachNode(); // Nodes become visible only after having their position set at least once
}

void EC_Placeable::SetOrientation(const Quaternion& orientation)
{
    link_scene_node_->setOrientation(Ogre::Quaternion(orientation.w, orientation.x, orientation.y, orientation.z));
}

void EC_Placeable::LookAt(const Vector3df& look_at)
{
    // Don't rely on the stability of the lookat (since it uses previous orientation), 
    // so start in identity transform
    link_scene_node_->setOrientation(Ogre::Quaternion::IDENTITY);
    link_scene_node_->lookAt(Ogre::Vector3(look_at.x, look_at.y, look_at.z), Ogre::Node::TS_WORLD);
}

void EC_Placeable::SetYaw(float radians)
{
    link_scene_node_->yaw(Ogre::Radian(radians), Ogre::Node::TS_WORLD);
}

void EC_Placeable::SetPitch(float radians)
{
    link_scene_node_->pitch(Ogre::Radian(radians));
}

void EC_Placeable::SetRoll(float radians)
{
    link_scene_node_->roll(Ogre::Radian(radians));
} 

float EC_Placeable::GetYaw() const
{
    const Ogre::Quaternion& orientation = link_scene_node_->getOrientation();
    return orientation.getYaw().valueRadians();
}
float EC_Placeable::GetPitch() const
{
    const Ogre::Quaternion& orientation = link_scene_node_->getOrientation();
    return orientation.getPitch().valueRadians();
}
float EC_Placeable::GetRoll() const
{
    const Ogre::Quaternion& orientation = link_scene_node_->getOrientation();
    return orientation.getRoll().valueRadians();
}

void EC_Placeable::SetScale(const Vector3df& scale)
{
    scene_node_->setScale(Ogre::Vector3(scale.x, scale.y, scale.z));
}

void EC_Placeable::AttachNode()
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
        EC_Placeable* parent = checked_static_cast<EC_Placeable*>(parent_.get());
        parent_node = parent->GetLinkSceneNode();
    }
    
    parent_node->addChild(link_scene_node_);
    attached_ = true;
}

void EC_Placeable::DetachNode()
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
        EC_Placeable* parent = checked_static_cast<EC_Placeable*>(parent_.get());
        parent_node = parent->GetLinkSceneNode();
    }
    
    parent_node->removeChild(link_scene_node_);
    attached_ = false;
}

//experimental QVector3D acessors
QVector3D EC_Placeable::GetQPosition() const
{
    //conversions, conversions, all around
    //.. if this works, and QVector3D is good, we should consider porting Vector3df for that
    Vector3df rexpos = GetPosition();
    return QVector3D(rexpos.x, rexpos.y, rexpos.z);
}

void EC_Placeable::SetQPosition(const QVector3D newpos)
{
    SetPosition(Vector3df(newpos.x(), newpos.y(), newpos.z()));
}


QQuaternion EC_Placeable::GetQOrientation() const 
{
    Quaternion rexort = GetOrientation();
    return QQuaternion(rexort.w, rexort.x, rexort.y, rexort.z);
}

void EC_Placeable::SetQOrientation(const QQuaternion newort)
{
    SetOrientation(Quaternion(newort.x(), newort.y(), newort.z(), newort.scalar()));
}


QVector3D EC_Placeable::GetQScale() const
{
    Vector3df rexscale = GetScale();
    return QVector3D(rexscale.x, rexscale.y, rexscale.z);
}

void EC_Placeable::SetQScale(const QVector3D newscale)
{
    SetScale(Vector3df(newscale.x(), newscale.y(), newscale.z()));
}

QVector3D EC_Placeable::translate(int axis, float amount)
{
    Ogre::Matrix3 m;
    Ogre::Vector3 v;
    float x, y, z;
    x = y = z = 0.0;
    m.SetColumn(0,  link_scene_node_->getOrientation().xAxis());
    m.SetColumn(1,  link_scene_node_->getOrientation().yAxis());
    m.SetColumn(2,  link_scene_node_->getOrientation().zAxis());
    switch(axis) {
        case 0:
            x = amount;
            break;
        case 1:
            y = amount;
            break;
        case 2:
            z = amount;
            break;
        default:
            // nothing, don't translate
            break;

    }
    link_scene_node_->translate(m, Ogre::Vector3(x, y, z), Ogre::Node::TS_LOCAL);
    const Ogre::Vector3 newpos = link_scene_node_->getPosition();
    return QVector3D(newpos.x, newpos.y, newpos.z);
}

void EC_Placeable::HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type change)
{
    if (attribute == &transform)
    {
        // Safety & legacy world compatibility: in non-networksynced mode, do nothing
        if (!GetNetworkSyncEnabled())
            return;
        if (!link_scene_node_)
            return;
        
        Transform newTransform = transform.Get();
        link_scene_node_->setPosition(newTransform.position.x, newTransform.position.y, newTransform.position.z);
        Quaternion orientation(DEGTORAD * newTransform.rotation.x,
                          DEGTORAD * newTransform.rotation.y,
                          DEGTORAD * newTransform.rotation.z);
        link_scene_node_->setOrientation(Ogre::Quaternion(orientation.w, orientation.x, orientation.y, orientation.z));

        // Prevent Ogre exception from zero scale
        if (newTransform.scale.x < 0.0000001f)
            newTransform.scale.x = 0.0000001f;
        if (newTransform.scale.y < 0.0000001f)
            newTransform.scale.y = 0.0000001f;
        if (newTransform.scale.z < 0.0000001f)
            newTransform.scale.z = 0.0000001f;

        link_scene_node_->setScale(newTransform.scale.x, newTransform.scale.y, newTransform.scale.z);
        
        AttachNode(); // Nodes become visible only after having their position set at least once
    }
}

void EC_Placeable::Show()
{
	if (!link_scene_node_)
        return;

link_scene_node_->setVisible(true);
}

void EC_Placeable::Hide()
{
		if (!link_scene_node_)
			return;	
		
		link_scene_node_->setVisible(false);
}

void EC_Placeable::ToggleVisibility()
{
    if (!link_scene_node_)
        return;

    link_scene_node_->flipVisibility();
}

void EC_Placeable::RegisterActions()
{
    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (entity)
    {
        // Generic actions
        entity->ConnectAction("ShowEntity", this, SLOT(Show()));
        entity->ConnectAction("HideEntity", this, SLOT(Hide()));
        entity->ConnectAction("ToggleEntity", this, SLOT(ToggleVisibility()));

    }
}

