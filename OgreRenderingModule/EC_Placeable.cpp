// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "Entity.h"
#include "LoggingFunctions.h"

#include <Ogre.h>

using namespace OgreRenderer;

void SetShowBoundingBoxRecursive(Ogre::SceneNode* node, bool enable)
{
    if (!node)
        return;
    node->showBoundingBox(enable);
    int numChildren = node->numChildren();
    for(int i = 0; i < numChildren; ++i)
    {
        Ogre::SceneNode* childNode = dynamic_cast<Ogre::SceneNode*>(node->getChild(i));
        if (childNode)
            SetShowBoundingBoxRecursive(childNode, enable);
    }
}

EC_Placeable::EC_Placeable(IModule* module) :
    IComponent(module->GetFramework()),
    renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
    scene_node_(0),
    link_scene_node_(0),
    attached_(false),
    select_priority_(0),
    transform(this, "Transform"),
    drawDebug(this, "Show bounding box", false),
    visible(this, "Visible", true)
{
    // Enable network interpolation for the transform
    static AttributeMetadata transAttrData;
    static AttributeMetadata nonDesignableAttrData;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        transAttrData.interpolation = AttributeMetadata::Interpolate;
        nonDesignableAttrData.designable = false;
        metadataInitialized = true;
    }
    transform.SetMetadata(&transAttrData);

    RendererPtr renderer = renderer_.lock();
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    link_scene_node_ = scene_mgr->createSceneNode(renderer->GetUniqueObjectName("EC_Placeable_LinkSceneNode"));
    scene_node_ = scene_mgr->createSceneNode(renderer->GetUniqueObjectName("EC_Placeable_SceneNode"));
    link_scene_node_->addChild(scene_node_);
    
    // In case the placeable is used for camera control, set fixed yaw axis
    link_scene_node_->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);

    // Hook the transform attribute change
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)),
        SLOT(HandleAttributeChanged(IAttribute*, AttributeChange::Type)));

    connect(this, SIGNAL(ParentEntitySet()), SLOT(RegisterActions()));
    
    AttachNode();
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
        LogError("Attempted to set parent placeable which is not of type \"" + TypeNameStatic() +"\"!");
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

void EC_Placeable::SetPosition(const Vector3df &pos)
{
   if (!pos.IsFinite())
   {
        LogError("EC_Placeable::SetPosition called with a vector that is not finite!");
        return;
   }
    // link_scene_node_->setPosition(Ogre::Vector3(pos.x, pos.y, pos.z));
    Transform newtrans = transform.Get();
    newtrans.SetPos(pos.x, pos.y, pos.z);
    transform.Set(newtrans, AttributeChange::Default);
}

void EC_Placeable::SetOrientation(const Quaternion& orientation)
{
    // link_scene_node_->setOrientation(Ogre::Quaternion(orientation.w, orientation.x, orientation.y, orientation.z));
    Transform newtrans = transform.Get();
    Vector3df result;
    orientation.toEuler(result);
    newtrans.SetRot(result.x * RADTODEG, result.y * RADTODEG, result.z * RADTODEG);
    transform.Set(newtrans, AttributeChange::Default);
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

void EC_Placeable::SetScale(const Vector3df& newscale)
{
    scene_node_->setScale(Ogre::Vector3(newscale.x, newscale.y, newscale.z));
    // AttachNode(); // Nodes become visible only after having their position set at least once

    Transform newtrans = transform.Get();
    newtrans.SetScale(newscale.x, newscale.y, newscale.z);
    transform.Set(newtrans, AttributeChange::Default);
}

void EC_Placeable::AttachNode()
{
    if (renderer_.expired())
    {
        LogError("EC_Placeable::AttachNode: No renderer available to call this function!");
        return;
    }
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
    {
        LogError("EC_Placeable::DetachNode: No renderer available to call this function!");
        return;
    }
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
    const Transform& trans = transform.Get();
    return QVector3D(trans.position.x, trans.position.y, trans.position.z);
}

void EC_Placeable::SetQPosition(QVector3D newpos)
{
    Transform trans = transform.Get();
    trans.position.x = newpos.x();
    trans.position.y = newpos.y();
    trans.position.z = newpos.z();
    transform.Set(trans, AttributeChange::Default);
    emit PositionChanged(newpos);
}


QQuaternion EC_Placeable::GetQOrientation() const 
{
    const Transform& trans = transform.Get();
    Quaternion orientation(DEGTORAD * trans.rotation.x,
                      DEGTORAD * trans.rotation.y,
                      DEGTORAD * trans.rotation.z);
    return QQuaternion(orientation.w, orientation.x, orientation.y, orientation.z);
}

void EC_Placeable::SetQOrientation(QQuaternion newort)
{
    Transform trans = transform.Get();
    
    Quaternion q(newort.x(), newort.y(), newort.z(), newort.scalar());
    
    Vector3df eulers;
    q.toEuler(eulers);
    trans.rotation.x = eulers.x * RADTODEG;
    trans.rotation.y = eulers.y * RADTODEG;
    trans.rotation.z = eulers.z * RADTODEG;
    transform.Set(trans, AttributeChange::Default);
    
    emit OrientationChanged(newort);
}

QVector3D EC_Placeable::GetQScale() const
{
    const Transform& trans = transform.Get();
    return QVector3D(trans.scale.x, trans.scale.y, trans.scale.z);
}

void EC_Placeable::SetQScale(QVector3D newscale)
{
    Transform trans = transform.Get();
    trans.scale.x = newscale.x();
    trans.scale.y = newscale.y();
    trans.scale.z = newscale.z();
    transform.Set(trans, AttributeChange::Default);
    emit ScaleChanged(newscale);
}

void EC_Placeable::SetQOrientationEuler(QVector3D newrot)
{
    Transform trans = transform.Get();
    trans.rotation.x = newrot.x();
    trans.rotation.y = newrot.y();
    trans.rotation.z = newrot.z();
    transform.Set(trans, AttributeChange::Default);
    
    Quaternion orientation(DEGTORAD * newrot.x(),
                      DEGTORAD * newrot.y(),
                      DEGTORAD * newrot.z());
    emit OrientationChanged(QQuaternion(orientation.w, orientation.x, orientation.y, orientation.z));
}

QVector3D EC_Placeable::GetQOrientationEuler() const
{
    const Transform& trans = transform.Get();
    return QVector3D(trans.rotation.x, trans.rotation.y, trans.rotation.z);
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
        if (!link_scene_node_ || !scene_node_)
            return;
        
        const Transform& trans = transform.Get();
        if (trans.position.IsFinite())
        {
            link_scene_node_->setPosition(trans.position.x, trans.position.y, trans.position.z);
        }
        
        Quaternion orientation(DEGTORAD * trans.rotation.x,
                          DEGTORAD * trans.rotation.y,
                          DEGTORAD * trans.rotation.z);

        if (orientation.IsFinite())
            link_scene_node_->setOrientation(Ogre::Quaternion(orientation.w, orientation.x, orientation.y, orientation.z));
        else
            ::LogError("EC_Placeable: transform attribute changed, but orientation not valid!");

        // Prevent Ogre exception from zero scale
        Vector3df scale(trans.scale.x, trans.scale.y, trans.scale.z);
        if (scale.x < 0.0000001f)
            scale.x = 0.0000001f;
        if (scale.y < 0.0000001f)
            scale.y = 0.0000001f;
        if (scale.z < 0.0000001f)
            scale.z = 0.0000001f;

        scene_node_->setScale(scale.x, scale.y, scale.z);
    }
    else if (attribute == &drawDebug)
    {
        SetShowBoundingBoxRecursive(link_scene_node_, drawDebug.Get());
    }
    else if (attribute == &visible && link_scene_node_)
        link_scene_node_->setVisible(visible.Get());
    /*
    else if(attribute == &position)
    {
        if (!link_scene_node_)
            return;
        QVector3D newPosition = position.Get();
        link_scene_node_->setPosition(newPosition.x(), newPosition.y(), newPosition.z());
        AttachNode();
    }
    else if(attribute == &scale)
    {
        if (!link_scene_node_)
            return;
        QVector3D newScale = scale.Get();
        scene_node_->setScale(newScale.x(), newScale.y(), newScale.z());
        AttachNode();
    }
    */
}

Vector3df EC_Placeable::GetRotationFromTo(const Vector3df& from, const Vector3df& to)
{
    Quaternion orientation;
    orientation.rotationFromTo(from,to);
    Vector3df result;
    orientation.toEuler(result);
    result *= RADTODEG;
    return result;
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


void EC_Placeable::Translate(const Vector3df& translation)
{
    Transform newTrans = transform.Get();
    newTrans.position += translation;
    transform.Set(newTrans, AttributeChange::Default);
}

void EC_Placeable::TranslateRelative(const Vector3df& translation)
{
    Transform newTrans = transform.Get();
    newTrans.position += GetOrientation() * translation;
    transform.Set(newTrans, AttributeChange::Default);
}

Vector3df EC_Placeable::GetRelativeVector(const Vector3df& vec)
{
    return GetOrientation() * vec;
}
