// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OgreRenderingModule_CompositionHandler_h
#define incl_OgreRenderingModule_CompositionHandler_h

#include "OgreModuleApi.h"

#include <OgreMaterial.h>
#include <OgreCompositorInstance.h>
#include "CoreTypes.h"

namespace Foundation
{
    class Framework;
}

namespace OgreRenderer
{
    ///\note This class and its implementation is taken from the Ogre samples
    class HDRListener: public Ogre::CompositorInstance::Listener
    {
    public:
        HDRListener();
        virtual ~HDRListener();
        void notifyViewportSize(int width, int height);
        void notifyCompositor(Ogre::CompositorInstance* instance);
        virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
        ///\todo Is this needed? Does nothing atm.
        virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat) {}

    protected:
        int mVpWidth, mVpHeight;
        int mBloomSize;
        // Array params - have to pack in groups of 4 since this is how Cg generates them
        // also prevents dependent texture read problems if ops don't require swizzle
        float mBloomTexWeights[15][4];
        float mBloomTexOffsetsHorz[15][4];
        float mBloomTexOffsetsVert[15][4];
    };

    ///\note This class and its implementation is taken from the Ogre samples
    class GaussianListener: public Ogre::CompositorInstance::Listener
    {
    public:
        GaussianListener();
        virtual ~GaussianListener();
        void notifyViewportSize(int width, int height);
        virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
        ///\todo Is this needed? Does nothing atm.
        virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat) {}

    protected:
        int mVpWidth, mVpHeight;
        // Array params - have to pack in groups of 4 since this is how Cg generates them
        // also prevents dependent texture read problems if ops don't require swizzle
        float mBloomTexWeights[15][4];
        float mBloomTexOffsetsHorz[15][4];
        float mBloomTexOffsetsVert[15][4];
    };

    ///\note This class handles the post-processing effects
    class OGRE_MODULE_API CompositionHandler
    {
    public:
        //! Default constructor.
        CompositionHandler();

        //! Destructor.
        virtual ~CompositionHandler();

        //! Initialize the composition handler. This has to be called before trying to enable/disable effects
        bool Initialize(Foundation::Framework* framework, Ogre::Viewport *vp);

        //! map number to effect name (used to interpret server request since server requests composite activation with a number)
        std::string MapNumberToEffectName(const std::string &number);

        //! Adds specified compositor for the viewport if it's found. Effect is appended last in the chain if position is not specified (Except HDR)
        bool AddCompositorForViewport(const std::string &compositor, Ogre::Viewport *vp, int position = -1);

        //! Remove specified compositor effect from viewport
        void RemoveCompositorFromViewport(const std::string &compositor, Ogre::Viewport *vp);

        //! translates the message (RexPostP) 
        void ExecuteServersShaderRequest(const StringVector &parameters);

        //! Returns list of available post-processing effects
        std::vector<std::string> &GetAvailableCompositors() { return postprocess_effects_; }

        //! Convenience function that will add specified compositor for the default viewport given in initialization
        bool AddCompositorForViewport(const std::string &compositor, int position = -1);

        //! Convenience funtion to remove specified  compositor from the default viewport
        void RemoveCompositorFromViewport(const std::string &compositor);

    private:
        //Used to specify postprocessing effects currently available. Number is needed to map server requests to the actual effect name.
        std::vector<std::string> postprocess_effects_;

        //! Compositor manager
        Ogre::CompositorManager* c_manager_;

        //! Ogre viewport.
        Ogre::Viewport* viewport_;

        //! Framelistener for HDR effect
        HDRListener hdr_listener_;

        //! Framelistener for gaussian blur
        GaussianListener gaussian_listener_;

        //! handle to framework
        Foundation::Framework* framework_;
    };
}

#endif
