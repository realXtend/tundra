// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ARRenderer.h"
#include "Renderer.h"

#include "OgreMaterialUtils.h"

#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreSubEntity.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreHardwarePixelBuffer.h>

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("ARRenderer")

namespace AR
{
    ARRenderer::ARRenderer(OgreRenderer::Renderer *renderer) :
        renderer_(renderer),
        realityFeedQueueId_(Ogre::RENDER_QUEUE_4),
        invisibleQueueId_(Ogre::RENDER_QUEUE_3),
        visibleQueueId_(Ogre::RENDER_QUEUE_MAIN),
        renderQuad_(0),
        quadMaterial_(0),
        quadTexture_(0),
        realityFeedEnabled_(false)
    {
        Initialize();
    }

    ARRenderer::~ARRenderer()
    {
    }

    // Private

    void ARRenderer::Initialize()
    {
        renderer_->GetSceneManager()->addRenderQueueListener(this);

        // Prepare render quad
        renderQuad_ = OGRE_NEW Ogre::Rectangle2D(true);
        Ogre::RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
        Ogre::Viewport* vp = renderer_->GetViewport();
        Ogre::Real hOffset = rs->getHorizontalTexelOffset() / (0.5f * vp->getActualWidth());
        Ogre::Real vOffset = rs->getVerticalTexelOffset() / (0.5f * vp->getActualHeight());
        renderQuad_->setCorners(-1 + hOffset, 1 - vOffset, 1 + hOffset, -1 - vOffset);
        renderQuad_->setCastShadows(false);

        // Prepare material
        materialName_ = renderer_->GetUniqueObjectName("ARrenderQuad_Material");
        Ogre::MaterialManager &matmngr = Ogre::MaterialManager::getSingleton();
        quadMaterial_ = matmngr.create(materialName_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        quadMaterial_->getTechnique(0)->setDepthCheckEnabled(false);
        quadMaterial_->getTechnique(0)->setDepthWriteEnabled(false);
        quadMaterial_->getTechnique(0)->getPass(0)->createTextureUnitState();
        quadMaterial_->setLightingEnabled(false);
        quadMaterial_->setReceiveShadows(false);
        renderQuad_->setMaterial(quadMaterial_->getName());

        renderQuad_->setRenderQueueGroup(realityFeedQueueId_);

        // Prepare texture
        textureName_ = renderer_->GetUniqueObjectName("ARRenderer_Texture");
        Ogre::TextureManager &texmngr = Ogre::TextureManager::getSingleton();
        quadTexture_ = texmngr.createManual(textureName_, 
                                               Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                               Ogre::TEX_TYPE_2D, 1, 1, 0, Ogre::PF_A8R8G8B8, 
                                               Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
        if (quadTexture_.isNull())
            textureName_ = "";
        else
            OgreRenderer::SetTextureUnitOnMaterial(quadMaterial_, textureName_, 0);

        // With this you can change the default visibility of objects
        renderer_->GetSceneManager()->getRenderQueue()->setDefaultQueueGroup(visibleQueueId_);
        renderer_->GetSceneManager()->getRenderQueue()->getQueueGroup(realityFeedQueueId_);
    }

    void ARRenderer::AddEntityToInvisibleQueue(Ogre::Entity* ent)
    {
        ent->setRenderQueueGroup(invisibleQueueId_);
        /*
        for(int i=0; i<ent->getNumSubEntities(); i++)
        {
            Ogre::SubEntity* sub = ent->getSubEntity(i);
            Ogre::Technique* tech = sub->getMaterial()->getTechnique(0);
            tech->setDepthCheckEnabled(true);
            tech->setDepthWriteEnabled(true);
            tech->setColourWriteEnabled(false);
        }
        */
    }

    void ARRenderer::AddEntityToVisibleQueue(Ogre::Entity* ent)
    {
        ent->setRenderQueueGroup(visibleQueueId_);
        /*
        for(int i=0; i<ent->getNumSubEntities(); i++)
        {
            Ogre::SubEntity* sub = ent->getSubEntity(i);
            Ogre::Technique* tech = sub->getMaterial()->getTechnique(0);
            tech->setDepthCheckEnabled(true);
            tech->setDepthWriteEnabled(true);
            tech->setColourWriteEnabled(true);
        }
        */
    }

    // Public

    void ARRenderer::BlitRealityToTexture(QImage frame)
    {
        if (quadTexture_.isNull())
            return;

        if (frame.format() != QImage::Format_RGB32)
            return;

        if ((int)quadTexture_->getWidth() != frame.width() || (int)quadTexture_->getHeight() != frame.height())
        {
            quadTexture_->freeInternalResources();
            quadTexture_->setWidth(frame.width());
            quadTexture_->setHeight(frame.height());
            quadTexture_->createInternalResources();
        }

        Ogre::Box updateBox(0,0, frame.width(), frame.height());
        Ogre::PixelBox pixelBox(updateBox, Ogre::PF_A8R8G8B8, (void*)frame.bits());
        if (!quadTexture_->getBuffer().isNull())
            quadTexture_->getBuffer()->blitFromMemory(pixelBox, updateBox);
    }

    void ARRenderer::SetRealityFeedEnabled(bool enabled)
    {
        realityFeedEnabled_ = enabled;
    }

    // Ogre::RenderQueueListener overrides

    void ARRenderer::preRenderQueues()
    {
        if (realityFeedEnabled_ && renderer_)
            renderer_->GetSceneManager()->getRenderQueue()->addRenderable(renderQuad_, realityFeedQueueId_);
    }

    void ARRenderer::postRenderQueues()
    {
    }

    void ARRenderer::renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String &invocation, bool &skipThisInvocation)
    {
        // Skip all the queue groups that we are not handling
        /* not needed anymore?
        if (queueGroupId != invisibleQueueId_ && 
            queueGroupId != realityFeedQueueId_ && 
            queueGroupId != visibleQueueId_)
        {
            skipThisInvocation = true;
        }
        if(queueGroupId == realityFeedQueueId_)
        {
            renderer_->GetSceneManager()->_injectRenderWithPass(quadMaterial_->getTechnique(0)->getPass(0), renderQuad_, false);
        }
        */

        if (invocation == "SHADOWS" && queueGroupId == realityFeedQueueId_)
            skipThisInvocation = true;
    }

    void ARRenderer::renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String &invocation, bool &repeatThisInvocation)
    {
    }
}