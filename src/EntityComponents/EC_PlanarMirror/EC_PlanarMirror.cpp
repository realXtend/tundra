/**
    For conditions of distribution and use, see copyright notice in LICENSE
 
    @file   EC_PlanarMirror.cpp
    @brief  Enables one to create planar mirrors. */

#include "Math/MathFwd.h"
#include "EC_PlanarMirror.h"

#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "OgreWorld.h"

#include "Framework.h"
#include "Entity.h"
#include "EC_Placeable.h"
#include "LoggingFunctions.h"
#include "FrameAPI.h"
#include "EC_Camera.h"
#include "EC_RttTarget.h"
#include "UiAPI.h"
#include "UiMainWindow.h"

#include <OgreTextureUnitState.h>
#include <OgreCamera.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreEntity.h>
#include <OgreMeshManager.h>
#include <OgreMaterialManager.h>

int EC_PlanarMirror::mirror_cam_num_ = 0;

EC_PlanarMirror::EC_PlanarMirror(Scene *scene) :
    IComponent(scene),
    reflectionPlaneVisible(this, "Show reflection plane", true),
    mirror_texture_(0),
    tex_unit_state_(0),
    mat_(0),
    mirror_plane_entity_(0),
    mirror_plane_(0)
{
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(Initialize()));
}

EC_PlanarMirror::~EC_PlanarMirror()
{
    if(!renderer_)
        return;
    Ogre::SceneManager *mngr = renderer_->GetActiveOgreWorld()->OgreSceneManager();
    tex_unit_state_->setProjectiveTexturing(false);
    mngr->destroyEntity(mirror_plane_entity_);
    SAFE_DELETE(mirror_plane_);
}

void EC_PlanarMirror::AttributesChanged()
{
    if (!ViewEnabled())
        return;

    if (reflectionPlaneVisible.ValueChanged())
        mirror_plane_entity_->setVisible(getreflectionPlaneVisible());
}

void EC_PlanarMirror::Update(float val)
{
    if (!ViewEnabled())
        return;
    if (!renderer_)
        return;

    const Ogre::Camera* cam = renderer_->MainOgreCamera();

    if (mirror_cam_)
    {
        mirror_cam_->setOrientation(cam->getRealOrientation());
        mirror_cam_->setPosition(cam->getRealPosition());
        //mirror_cam_->getViewport()->update();
    }
}

void EC_PlanarMirror::Initialize()
{
    if (!ViewEnabled())
        return;

    if (framework->GetModule<OgreRenderer::OgreRenderingModule>())
        renderer_ = framework->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();

    if (!renderer_)
    {
        LogError("EC_PlanarMirror could not acquire Renderer, cannot initialize.");
        return;
    }
    
    Entity *entity = ParentEntity();
    assert(entity);
    if (!entity)
        return;

    EC_Camera *cam = entity->GetComponent<EC_Camera>().get();
    assert(cam);
    if (!cam)
    {
        LogError("No EC_Camera on this entity. EC_PlanarMirror can't function.");
        return;
    }
    EC_Placeable *placeable = entity->GetComponent<EC_Placeable>().get();
    assert(placeable);
    if (!placeable)
    {
        LogError("No EC_Placeable on this entity. EC_PlanarMirror can't function.");
        return;
    }

    EC_RttTarget *target = entity->GetComponent<EC_RttTarget>().get();
    assert(target);
    if (!target)
    {
        LogError("No EC_RttTarget on this entity. EC_PlanarMirror can't function.");
        return;
    }

    const Ogre::Camera* v_cam = renderer_->MainOgreCamera();
    const Ogre::Viewport* vp = renderer_->MainViewport();

    mirror_cam_ = cam->GetCamera();
    mirror_cam_->setFarClipDistance(v_cam->getFarClipDistance());
    mirror_cam_->setNearClipDistance(v_cam->getNearClipDistance());
    mirror_cam_->setAutoAspectRatio(false);
    mirror_cam_->setAspectRatio(Ogre::Real(vp->getActualWidth())/Ogre::Real(vp->getActualHeight()));
    mirror_cam_->setFOVy(v_cam->getFOVy());

    QString texname = target->gettextureName();
    Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().getByName(Ogre::String(texname.toStdString()));
    
    if (!tex.get())
    {
        LogError("Texture called " + texname.toStdString() +  " not found. EC_PlanarMirror can't function. (Propably something wrong in the EC_RttTarget).");
        return;
    }
    mirror_texture_ = tex.get();

    CreatePlane();
    placeable->GetSceneNode()->attachObject(mirror_plane_entity_);
    placeable->GetSceneNode()->attachObject(mirror_plane_);

    mirror_cam_->enableCustomNearClipPlane(mirror_plane_);
    mirror_cam_->enableReflection(mirror_plane_);

    mirror_cam_num_++;
    disconnect(this, SIGNAL(ParentEntitySet()), this, SLOT(Initialize()));

    mirror_cam_->getViewport()->setOverlaysEnabled(false);

    connect(framework->Frame(), SIGNAL(Updated(float)), this, SLOT(Update(float)), Qt::UniqueConnection);
    connect(framework->Ui()->MainWindow(), SIGNAL(WindowResizeEvent(int, int)), this, SLOT(WindowResized(int,int)), Qt::UniqueConnection);
}

void EC_PlanarMirror::WindowResized(int w,int h)
{
    if (!ViewEnabled())
        return;

    if(renderer_)
    {
        const Ogre::Viewport* vp = renderer_->MainViewport();
        mirror_cam_->setAspectRatio(Ogre::Real(vp->getActualWidth())/Ogre::Real(vp->getActualHeight()));
    }
}

void EC_PlanarMirror::CreatePlane()
{
    if (!ViewEnabled())
        return;

    mirror_plane_ = new Ogre::MovablePlane("mirror_plane" + Ogre::StringConverter::toString(mirror_cam_num_));
    mirror_plane_->d = 0;
    mirror_plane_->normal = Ogre::Vector3::UNIT_Y;
    Ogre::MeshManager::getSingleton().createPlane("mirror_plane_mesh_" + Ogre::StringConverter::toString(mirror_cam_num_),
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, *mirror_plane_, 80, 80, 1, 1, true, 1, 1, 1, Ogre::Vector3::UNIT_Z);
    mirror_plane_entity_ = renderer_->GetActiveOgreWorld()->OgreSceneManager()->createEntity("mirror_plane_entity" + Ogre::StringConverter::toString(mirror_cam_num_),
        "mirror_plane_mesh_" + Ogre::StringConverter::toString(mirror_cam_num_));

    ///for now
    mat_ = dynamic_cast<Ogre::Material*>(Ogre::MaterialManager::getSingleton().create(
        "MirrorMat" + Ogre::StringConverter::toString(mirror_cam_num_), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).get());
    tex_unit_state_ = mat_->getTechnique(0)->getPass(0)->createTextureUnitState(mirror_texture_->getName());
    tex_unit_state_->setProjectiveTexturing(true, mirror_cam_);
    tex_unit_state_->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
    Ogre::MaterialPtr mat(mat_);
    mirror_plane_entity_->setMaterial(mat);
}

Ogre::Texture* EC_PlanarMirror::GetMirrorTexture() const
{
    return mirror_texture_;
}
