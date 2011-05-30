// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AR_ARRenderer_h
#define incl_AR_ARRenderer_h

#include "ARFwd.h"

#include <QObject>

#include <OgreRenderQueueListener.h>
#include <OgreEntity.h>
#include <OgreMaterialManager.h>
#include <OgreMaterial.h>
#include <OgreTextureManager.h>
#include <OgreTexture.h>
#include <OgreRectangle2D.h>

#include <QImage>

namespace AR
{
    /**
    <table class="header"><tr><td>
    <h2>ARRenderer</h2>

    This class is responsible of managing the renderable objects. Basically it will just adjust the rendering options of materials and move renderable items to their proper rendering queue groups.
    ARRenderer will also render a fullscreen image to the screen, this image is supposed to be an image from the video stream.

    </td></tr></table>
    */
    class ARRenderer : public QObject, public Ogre::RenderQueueListener
    {

    Q_OBJECT

    public:
        /// ARRenderer ctor.
        ARRenderer(OgreRenderer::Renderer *renderer);

        /// ARRenderer dtor.
        ~ARRenderer();

        /// Ogre::RenderQueueListener override.
        virtual void preRenderQueues();

        /// Ogre::RenderQueueListener override.
        virtual void postRenderQueues();

        /// Ogre::RenderQueueListener override.
        virtual void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String &invocation, bool &skipThisInvocation);

        /// Ogre::RenderQueueListener override.
        virtual void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String &invocation, bool &repeatThisInvocation);

    private slots:
        /// Initialize the renderer.
        void Initialize();

    public slots:
        /// Adds entity to the queue where visible objects are (also the material is modified). Please note that the material is modified, so all the objects sharing this material 
        /// will receive the modifications. Use separate materials if this is not acceptable.
        void AddEntityToVisibleQueue(Ogre::Entity* ent);

        ///Adds entity to the queue where invisible objects are (also the material is modified). Please note that the material is modified, so all the objects sharing this material 
        /// will receive the modifications. Use separate materials if this is not acceptable.
        void AddEntityToInvisibleQueue(Ogre::Entity* ent);

        /// Blit the QImage to our full screen quad texture.
        /// \param QImage Image to be blitted.
        void BlitRealityToTexture(QImage frame);

        /// Enable or disable full screen texture.
        /// \param bool Enabled.
        void SetRealityFeedEnabled(bool enabled);

    private:
        /// Ogre renderer ptr.
        OgreRenderer::Renderer* renderer_;

        /// Render queue groups ID.
        int realityFeedQueueId_;
        
        /// Render queue groups ID.
        int invisibleQueueId_;
        
        /// Render queue groups ID.
        int visibleQueueId_;

        /// Full screen quad.
        Ogre::Rectangle2D* renderQuad_;

        /// Material for the full screen quad.
        Ogre::MaterialPtr quadMaterial_;

        /// Material name for the full screen quad.
        std::string materialName_;

        /// Texture for the full screen quad.
        Ogre::TexturePtr quadTexture_;

        /// Texture for the full screen quad.
        std::string textureName_;

        /// Boolean to track if full screen texture is enabled.
        bool realityFeedEnabled_;
    };
}

#endif