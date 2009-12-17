// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OgreRenderingModule_CompositionHandler_h
#define incl_OgreRenderingModule_CompositionHandler_h

#include <Foundation.h>
#include <QObject>
#include <Ogre.h>
#include "OgreModuleApi.h"

namespace OgreRenderer
{   
    /////
    //This class and it's implementation is taken from the Ogre samples
    /////
    class HDRListener: public Ogre::CompositorInstance::Listener
	{
	protected:
		int mVpWidth, mVpHeight;
		int mBloomSize;
		// Array params - have to pack in groups of 4 since this is how Cg generates them
		// also prevents dependent texture read problems if ops don't require swizzle
		float mBloomTexWeights[15][4];
		float mBloomTexOffsetsHorz[15][4];
		float mBloomTexOffsetsVert[15][4];
	public:
		HDRListener();
		virtual ~HDRListener();
		void notifyViewportSize(int width, int height);
		void notifyCompositor(Ogre::CompositorInstance* instance);
		virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
		virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
	};
	//---------------------------------------------------------------------------
      /////
    //This class and it's implementation is taken from the Ogre samples
    /////
	class GaussianListener: public Ogre::CompositorInstance::Listener
	{
	protected:
		int mVpWidth, mVpHeight;
		// Array params - have to pack in groups of 4 since this is how Cg generates them
		// also prevents dependent texture read problems if ops don't require swizzle
		float mBloomTexWeights[15][4];
		float mBloomTexOffsetsHorz[15][4];
		float mBloomTexOffsetsVert[15][4];
	public:
		GaussianListener();
		virtual ~GaussianListener();
		void notifyViewportSize(int width, int height);
		virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
		virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
	};




    //! This class handles the post-processing effects
	class OGRE_MODULE_API CompositionHandler
	{
	private:
        //! effects available
        typedef std::vector<std::string> Postprocesses;


		//Used to specify postprocessing effects currently available. Number is needed to map server requests to the actual effect name.
		Postprocesses postprocess_effects;
        //! Ogres compositormanager
		Ogre::CompositorManager* c_manager_;
        //! Default viewport. This will be used if no other viewport is specified when enabling an effect
        Ogre::Viewport* default_vp;
        //! Framelistener for HDR effect
        HDRListener hdr_listener_;
        //! Framelistener for gaussian blur
        GaussianListener gaussian_listener_;
        //! handle to framework
		Foundation::Framework* framework_;



        


	public:
		
        //! Everything Initialized to 0.
		CompositionHandler();


        //! Destroys the ui_
		virtual ~CompositionHandler();
        
        //! Initialize the composition handler. This has to be called before trying to enable/disable effects
		bool Initialize(Foundation::Framework* framework, Ogre::Viewport *vp);

        
        //! map number to effect name (used to interpret server request since server requests composite activation with a number)
        std::string MapNumberToEffectName( std::string const &number);

        //! Adds specified compositor for the viewport if it's found. Effect is appended last in the chain if position is not specified (Except HDR)
		bool AddCompositorForViewport( std::string  const &compositor, Ogre::Viewport *vp, int position = -1);
        //! Remove specified compositor effect from viewport
		void RemoveCompositorFromViewport( std::string  const &compositor, Ogre::Viewport *vp);
        //! translates the message (RexPostP) 
        void ExecuteServersShaderRequest( StringVector const &parameters);
        //! Returns list of available post-processing effects
        std::vector<std::string>& GetAvailableCompositors();

        //! Convenience function that will add specified compositor for the default viewport given in initialization
		bool AddCompositorForViewport( std::string  const &compositor, int position = -1);
        //! Convenience funtion to remove specified  compositor from the default viewport
		void RemoveCompositorFromViewport( std::string  const &compositor);
	};

    
}

#endif