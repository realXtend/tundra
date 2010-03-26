// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"
#include "CompositionHandler.h"
#include "OgreRenderingModule.h"

#include <OgreCompositorManager.h>
#include <OgreTechnique.h>
#include <OgreCompositionTechnique.h>

namespace OgreRenderer
{
    CompositionHandler::CompositionHandler():
        c_manager_(0),
        viewport_(0),
        framework_(0)
    {
    }

    CompositionHandler::~CompositionHandler()
    {
    }

    bool CompositionHandler::Initialize(Foundation::Framework* framework, Ogre::Viewport *vp)
    {
        postprocess_effects_.push_back("Bloom");
        postprocess_effects_.push_back("UnderWater");
        postprocess_effects_.push_back("Glass");
        postprocess_effects_.push_back("B&W");
        postprocess_effects_.push_back("Embossed");
        postprocess_effects_.push_back("Sharpen Edges");
        postprocess_effects_.push_back("Invert");
        postprocess_effects_.push_back("Posterize");
        postprocess_effects_.push_back("Laplace");
        postprocess_effects_.push_back("Tiling");
        postprocess_effects_.push_back("HDR");
        postprocess_effects_.push_back("Strong HDR");
        postprocess_effects_.push_back("Gaussian Blur");
        postprocess_effects_.push_back("Motion Blur");
        postprocess_effects_.push_back("Radial Blur");
        postprocess_effects_.push_back("WetLens");

        framework_ = framework;
        viewport_ = vp;
        c_manager_ = Ogre::CompositorManager::getSingletonPtr();
        if (c_manager_)
            return true;
        else
            return false;
    }

    std::string CompositionHandler::MapNumberToEffectName(const std::string &number)
    {
        std::string effect_name;
        if(number == "12")
        {
            effect_name = "Strong HDR";
        }else if(number == "4"){
            effect_name = "UnderWater";
        }
        return effect_name;
    }

    void CompositionHandler::ExecuteServersShaderRequest(const StringVector &parameters)
    {
        std::string effect_number = parameters.at(0);
        std::string enable = parameters.at(1);
        std::string effect_name;

        if(enable == "True")
        {
            effect_name = MapNumberToEffectName(effect_number);
            if (!effect_name.empty())
                AddCompositorForViewport(effect_name);
        }
        else if(enable == "False")
        {
            effect_name = MapNumberToEffectName(effect_number);
            if (!effect_name.empty())
                RemoveCompositorFromViewport(effect_name);
        }

        //12 (default, bloom (?))
        //4 (water)
    }

    void CompositionHandler::RemoveCompositorFromViewport(const std::string &compositor, Ogre::Viewport *vp)
    {
        c_manager_->setCompositorEnabled(vp, compositor, false);
        c_manager_->removeCompositor(vp, compositor);
    }

    bool CompositionHandler::AddCompositorForViewport(const std::string &compositor, Ogre::Viewport *vp, int position)
    {
        bool succesfull = false;

        if (c_manager_!=0)
        {
            //HDR must be first compositor in the chain
            if (compositor == "HDR" || compositor == "Strong HDR")
                position = 0;

            Ogre::CompositorInstance* comp = c_manager_->addCompositor(vp, compositor, position);
            if(comp != 0)
            {
                if(compositor == "HDR" || compositor == "Strong HDR")
                {
                    comp->addListener(&hdr_listener_);
                    hdr_listener_.notifyViewportSize(vp->getActualWidth(), vp->getActualHeight());
                    hdr_listener_.notifyCompositor(comp);
                }
                else if(compositor == "Gaussian Blur")
                {
                    comp->addListener(&gaussian_listener_);
                    gaussian_listener_.notifyViewportSize(vp->getActualWidth(), vp->getActualHeight());
                }

                c_manager_->setCompositorEnabled(vp, compositor, true);
                succesfull=true;
            }
        }
        
        if (!succesfull)
            OgreRenderingModule::LogWarning("Failed to enable effect: " + compositor);

        return succesfull;
    }

    bool CompositionHandler::AddCompositorForViewport(const std::string &compositor, int position)
    {
        return AddCompositorForViewport(compositor, viewport_, position);
    }

    void CompositionHandler::RemoveCompositorFromViewport(const std::string &compositor)
    {
        return RemoveCompositorFromViewport(compositor, viewport_);
    }


    /*************************************************************************
    HDRListener Methods
    *************************************************************************/

    HDRListener::HDRListener()
    {
    }

    HDRListener::~HDRListener()
    {
    }

    void HDRListener::notifyViewportSize(int width, int height)
    {
        mVpWidth = width;
        mVpHeight = height;
    }

    void HDRListener::notifyCompositor(Ogre::CompositorInstance* instance)
    {
        // Get some RTT dimensions for later calculations
        Ogre::CompositionTechnique::TextureDefinitionIterator defIter =
            instance->getTechnique()->getTextureDefinitionIterator();
        while (defIter.hasMoreElements())
        {
            Ogre::CompositionTechnique::TextureDefinition* def =
                defIter.getNext();
            if(def->name == "rt_bloom0")
            {
                mBloomSize = (int)def->width; // should be square
                // Calculate gaussian texture offsets & weights
                float deviation = 3.0f;
                float texelSize = 1.0f / (float)mBloomSize;

                // central sample, no offset
                mBloomTexOffsetsHorz[0][0] = 0.0f;
                mBloomTexOffsetsHorz[0][1] = 0.0f;
                mBloomTexOffsetsVert[0][0] = 0.0f;
                mBloomTexOffsetsVert[0][1] = 0.0f;
                mBloomTexWeights[0][0] = mBloomTexWeights[0][1] =
                    mBloomTexWeights[0][2] = Ogre::Math::gaussianDistribution(0, 0, deviation);
                mBloomTexWeights[0][3] = 1.0f;

                // 'pre' samples
                for(int i = 1; i < 8; ++i)
                {
                    mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
                        mBloomTexWeights[i][2] = 1.25f * Ogre::Math::gaussianDistribution(i, 0, deviation);
                    mBloomTexWeights[i][3] = 1.0f;
                    mBloomTexOffsetsHorz[i][0] = i * texelSize;
                    mBloomTexOffsetsHorz[i][1] = 0.0f;
                    mBloomTexOffsetsVert[i][0] = 0.0f;
                    mBloomTexOffsetsVert[i][1] = i * texelSize;
                }
                // 'post' samples
                for(int i = 8; i < 15; ++i)
                {
                    mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
                        mBloomTexWeights[i][2] = mBloomTexWeights[i - 7][0];
                    mBloomTexWeights[i][3] = 1.0f;

                    mBloomTexOffsetsHorz[i][0] = -mBloomTexOffsetsHorz[i - 7][0];
                    mBloomTexOffsetsHorz[i][1] = 0.0f;
                    mBloomTexOffsetsVert[i][0] = 0.0f;
                    mBloomTexOffsetsVert[i][1] = -mBloomTexOffsetsVert[i - 7][1];
                }
            }
        }
    }

    void HDRListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
    {
        // Prepare the fragment params offsets
        switch(pass_id)
        {
        //case 994: // rt_lum4
        case 993: // rt_lum3
        case 992: // rt_lum2
        case 991: // rt_lum1
        case 990: // rt_lum0
            break;
        case 800: // rt_brightpass
            break;
        case 701: // rt_bloom1
        {
            // horizontal bloom
            mat->load();
            Ogre::GpuProgramParametersSharedPtr fparams =
                mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
            const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
            fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsHorz[0], 15);
            fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
            break;
        }
        case 700: // rt_bloom0
        {
            // vertical bloom
            mat->load();
            Ogre::GpuProgramParametersSharedPtr fparams =
                mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
            const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
            fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsVert[0], 15);
            fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
            break;
        }
        }
    }

    /*************************************************************************
    GaussianListener Methods
    *************************************************************************/

    GaussianListener::GaussianListener()
    {
    }

    GaussianListener::~GaussianListener()
    {
    }

    void GaussianListener::notifyViewportSize(int width, int height)
    {
        mVpWidth = width;
        mVpHeight = height;
        // Calculate gaussian texture offsets & weights
        float deviation = 3.0f;
        float texelSize = 1.0f / (float)std::min(mVpWidth, mVpHeight);

        // central sample, no offset
        mBloomTexOffsetsHorz[0][0] = 0.0f;
        mBloomTexOffsetsHorz[0][1] = 0.0f;
        mBloomTexOffsetsVert[0][0] = 0.0f;
        mBloomTexOffsetsVert[0][1] = 0.0f;
        mBloomTexWeights[0][0] = mBloomTexWeights[0][1] =
            mBloomTexWeights[0][2] = Ogre::Math::gaussianDistribution(0, 0, deviation);
        mBloomTexWeights[0][3] = 1.0f;

        // 'pre' samples
        for(int i = 1; i < 8; ++i)
        {
            mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
                mBloomTexWeights[i][2] = Ogre::Math::gaussianDistribution(i, 0, deviation);
            mBloomTexWeights[i][3] = 1.0f;
            mBloomTexOffsetsHorz[i][0] = i * texelSize;
            mBloomTexOffsetsHorz[i][1] = 0.0f;
            mBloomTexOffsetsVert[i][0] = 0.0f;
            mBloomTexOffsetsVert[i][1] = i * texelSize;
        }
        // 'post' samples
        for(int i = 8; i < 15; ++i)
        {
            mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
                mBloomTexWeights[i][2] = mBloomTexWeights[i - 7][0];
            mBloomTexWeights[i][3] = 1.0f;

            mBloomTexOffsetsHorz[i][0] = -mBloomTexOffsetsHorz[i - 7][0];
            mBloomTexOffsetsHorz[i][1] = 0.0f;
            mBloomTexOffsetsVert[i][0] = 0.0f;
            mBloomTexOffsetsVert[i][1] = -mBloomTexOffsetsVert[i - 7][1];
        }
    }

    void GaussianListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
    {
        // Prepare the fragment params offsets
        switch(pass_id)
        {
        case 701: // blur horz
        {
            // horizontal bloom
            mat->load();
            Ogre::GpuProgramParametersSharedPtr fparams =
                mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
            const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
            fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsHorz[0], 15);
            fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
            break;
        }
        case 700: // blur vert
        {
            // vertical bloom
            mat->load();
            Ogre::GpuProgramParametersSharedPtr fparams =
                mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
            const Ogre::String& progName = mat->getBestTechnique()->getPass(0)->getFragmentProgramName();
            fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsVert[0], 15);
            fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
            break;
        }
        }
    }
}
