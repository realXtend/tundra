// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_RttTarget.h"
#include "EC_Camera.h"
#include "OgreMaterialUtils.h"

#include "Scene/Scene.h"
#include "FrameAPI.h"
#include "Entity.h"
#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

EC_RttTarget::EC_RttTarget(Scene* scene) :
    IComponent(scene),
    textureName(this, "Texture name", "RttTex"),
    width(this, "Texture width", 400),
    height(this, "Texture height", 300)
{
    //can't do immediately here, 'cause getcomponent crashes
    //.. is not allowed to get other components in the creation of a component. ok?
    //framework->Frame()->DelayedExecute(0.1f, this, SLOT(PrepareRtt()));
    //.. resorting to manual call to PrepareRtt now
}

EC_RttTarget::~EC_RttTarget()
{
    // Cannot use ViewEnabled() here, the parent entity is already null,
    // which means it will return true. After that we will crash below calling Ogre.
    if (framework->IsHeadless())
        return;

  //XXX didn't have a ref to renderer here yet. is this really required?
  //if(renderer_.expired())
  //      return;

    //if (!image_rendering_texture_name_.empty())
    Ogre::TextureManager::getSingleton().remove(textureName.Get().toStdString());
    //does this remove also the rendertarget with the viewports etc? seems so?
    
    Ogre::MaterialManager::getSingleton().remove(material_name_);
}

void EC_RttTarget::PrepareRtt()
{
    if (!ViewEnabled())
        return;

    //\todo XXX reconfig via AttributeUpdated when these change
    int x = width.Get();
    int y = height.Get();

    // Get the camera ec
    EC_Camera *ec_camera = ParentEntity()->GetComponent<EC_Camera>().get();
    if (!ec_camera)
    {
        LogInfo("No camera for rtt.");
        return; //XXX note: doesn't reschedule, so won't start working if cam added afterwards
    }

    ec_camera->GetCamera()->setAspectRatio(Ogre::Real(x) / Ogre::Real(y));

    Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().getByName(textureName.Get().toStdString());
    if (tex.isNull())
    {
        tex = Ogre::TextureManager::getSingleton().createManual(textureName.Get().toStdString(),
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, x, y, 0,
            Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET);
    }

    Ogre::RenderTexture *render_texture = tex->getBuffer()->getRenderTarget();
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
        tex->getBuffer()->getRenderTarget()->setAutoUpdated(false); 
    }
    else
        LogError("render target texture getting failed.");

    //create material to show the texture
    material_name_ = textureName.Get().toStdString() + "_mat"; //renderer_.lock()->GetUniqueObjectName("EC_BillboardWidget_mat");
    OgreRenderer::CloneMaterial("HoveringText", material_name_); //would LitTextured be the right thing? XXX \todo
    Ogre::MaterialManager &material_manager = Ogre::MaterialManager::getSingleton();
    Ogre::MaterialPtr material = material_manager.getByName(material_name_);
    OgreRenderer::SetTextureUnitOnMaterial(material, textureName.Get().toStdString());
}

void EC_RttTarget::SetAutoUpdated(bool val)
{
    if (!ViewEnabled())
        return;

    Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().getByName(textureName.Get().toStdString());
    if (tex.isNull())
    {
        LogError("render target texture getting failed.");
        return;
    }

    Ogre::RenderTexture *render_texture = tex->getBuffer()->getRenderTarget();
    if (!render_texture)
    {
        LogError("Render target texture getting failed.");
        return;
    }

    tex->getBuffer()->getRenderTarget()->setAutoUpdated(val);
}

/*void EC_RttTarget::ScheduleRender()
{
    framework->Frame()->DelayedExecute(0.1f, this, SLOT(UpdateRtt()));
}
*/

void EC_RttTarget::AttributesChanged()
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
