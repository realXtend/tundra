// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_RttTarget.h"
#include "OgreRenderingModule.h"
#include "EC_OgreCamera.h"
#include "OgreMaterialUtils.h"
#include "Frame.h"
#include "Entity.h"
#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_RttTarget");


EC_RttTarget::EC_RttTarget(IModule* module) :
  IComponent(module->GetFramework()),
  targettexture(this, "Target texture", "RttTex")
{
    QObject::connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(AttributeUpdated(IAttribute*)));

    //can't do immediately here, 'cause getcomponent crashes
    //.. is not allowed to get other components in the creation of a component. ok?
    framework_->GetFrame()->DelayedExecute(0.1f, this, SLOT(PrepareRtt()));
}

EC_RttTarget::~EC_RttTarget()
{
  //XXX didn't have a ref to renderer here yet. is this really required?
  //if(renderer_.expired())
  //      return;

    //if (!image_rendering_texture_name_.empty())
    Ogre::TextureManager::getSingleton().remove(targettexture.Get().toStdString());
    //does this remove also the rendertarget with the viewports etc? seems so?
    
    Ogre::MaterialManager::getSingleton().remove(material_name_);
}

void EC_RttTarget::PrepareRtt()
{
    //\todo XXX make these attributes, and reconfig via AttributeUpdated when they change
    uint width = 400;
    uint height = 300;

    // Get the camera ec
    EC_OgreCamera *ec_camera = this->GetParentEntity()->GetComponent<EC_OgreCamera>().get();
    if (!ec_camera)
    {
        LogInfo("No camera for rtt.");
        return; //XXX note: doesn't reschedule, so won't start working if cam added afterwards
    }

    ec_camera->GetCamera()->setAspectRatio(Ogre::Real(width) / Ogre::Real(height));

    tex_ = Ogre::TextureManager::getSingleton().getByName(targettexture.Get().toStdString());
    if (tex_.isNull())
    {
        tex_ = Ogre::TextureManager::getSingleton()
          .createManual(
                        targettexture.Get().toStdString(), 
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                        Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET);
    }

    Ogre::RenderTexture *render_texture = tex_->getBuffer()->getRenderTarget();
    if (render_texture)
    {
        render_texture->removeAllViewports();
        Ogre::Viewport *vp = 0;
        vp = render_texture->addViewport(ec_camera->GetCamera());
        // Exclude ui overlays
        vp->setOverlaysEnabled(false);
        // Exclude highlight mesh from rendering
        vp->setVisibilityMask(0x2);

        render_texture->update(false);
        tex_->getBuffer()->getRenderTarget()->setAutoUpdated(false);
    }

    else
        LogError("render target texture getting failed.");

    //create material to show the texture
    material_name_ = targettexture.Get().toStdString() + "_mat"; //renderer_.lock()->GetUniqueObjectName("EC_BillboardWidget_mat");
    OgreRenderer::CloneMaterial("HoveringText", material_name_); //would LitTextured be the right thing? XXX \todo
    Ogre::MaterialManager &material_manager = Ogre::MaterialManager::getSingleton();
    Ogre::MaterialPtr material = material_manager.getByName(material_name_);
    OgreRenderer::SetTextureUnitOnMaterial(material, targettexture.Get().toStdString());    
}

void EC_RttTarget::SetAutoUpdated(bool val)
{
    Ogre::RenderTexture *render_texture = tex_->getBuffer()->getRenderTarget();
    if (render_texture)
    {
         tex_->getBuffer()->getRenderTarget()->setAutoUpdated(val);
    }
    else
        LogError("render target texture getting failed.");
}

/*void EC_RttTarget::ScheduleRender()
{
    framework_->GetFrame()->DelayedExecute(0.1f, this, SLOT(UpdateRtt()));
}
*/

void EC_RttTarget::AttributeUpdated(IAttribute* attribute)
{
}

/* needed if autoupdate is not good (is too heavy and doesn't provide fps config?)
void EC_RttTarget::UpdateRtt()
{
    LogInfo("Rtt update");

    // Get rendering texture and update it
    Ogre::RenderTexture *render_texture = tex_->getBuffer()->getRenderTarget();
    if (render_texture)
    {
        render_texture->update(false);
    }

    ScheduleRender();
}
*/
