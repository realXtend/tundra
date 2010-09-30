/*
-----------------------------------------------------------------------------
This source is part of the Stereoscopy manager for OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
-----------------------------------------------------------------------------
* Copyright (c) 2008, Mathieu Le Ber, AXYZ-IMAGES
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the AXYZ-IMAGES nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Mathieu Le Ber ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Mathieu Le Ber BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------------
*/

// Last update jan 19 2010
#include "StableHeaders.h"
#include "StereoManager.h"
#include <vector>
#include <limits>


namespace CAVEStereo
    {
    //-------------- Stereo Camera listener -------------------------------
    void StereoManager::StereoCameraListener::init(StereoManager *stereoMgr, Ogre::Viewport *viewport, bool isLeftEye)	
    {
        shutter_flip_left_=false;
        stereo_mngr_ = stereoMgr;
        camera_ = NULL;
        is_lefteye_ = isLeftEye;
        viewport_ = viewport;
    }

    //void StereoManager::StereoCameraListener::preRenderTargetUpdate(const RenderTargetEvent& evt)
    void StereoManager::StereoCameraListener::preViewportUpdate (const Ogre::RenderTargetViewportEvent& evt)
    {
        if(evt.source != viewport_)
            return;
        camera_ = viewport_->getCamera();
        assert(camera_);
        stereo_mngr_->setCamera(camera_);

        Ogre::SceneManager *sceneMgr = camera_->getSceneManager();
        old_vis_mask_ = sceneMgr->getVisibilityMask();

        if(stereo_mngr_->getStereoMode() == StereoManager::SM_SHUTTER)
        {
            is_lefteye_ = shutter_flip_left_;
            shutter_flip_left_ = !shutter_flip_left_;
        }


        if(is_lefteye_)
        {
            sceneMgr->setVisibilityMask(stereo_mngr_->left_mask_ & old_vis_mask_);
        }
        else
        {
            sceneMgr->setVisibilityMask(stereo_mngr_->right_mask_ & old_vis_mask_);
        }

        // update the frustum offset
        Ogre::Real offset = (is_lefteye_ ? -0.5f : 0.5f) * stereo_mngr_->getEyesSpacing();
        offset += (is_lefteye_ ? -1:1) * stereo_mngr_->getPixelOffset() * (1.f/camera_->getViewport()->getActualWidth());

        if(!stereo_mngr_->is_custom_projection_)
        {
            old_offset_ = camera_->getFrustumOffset();
            if(!stereo_mngr_->isFocalLengthInfinite())
                camera_->setFrustumOffset(old_offset_ - Ogre::Vector2(offset,0));
        }
        else
        {
            if(is_lefteye_)
            {
                camera_->setCustomProjectionMatrix(true, stereo_mngr_->left_custom_projection_);
            }
            else
            {
                camera_->setCustomProjectionMatrix(true, stereo_mngr_->right_custom_projection_);
            }
        }

        // update position
        old_pos_ = camera_->getPosition();
        Ogre::Vector3 pos = old_pos_;
        
        pos += offset * camera_->getRight();
        camera_->setPosition(pos);

        stereo_mngr_->updateAllDependentRenderTargets(is_lefteye_);
        stereo_mngr_->chooseDebugPlaneMaterial(is_lefteye_);
    }
    //void StereoManager::StereoCameraListener::postRenderTargetUpdate(const RenderTargetEvent& evt)
    void StereoManager::StereoCameraListener::postViewportUpdate (const Ogre::RenderTargetViewportEvent& evt)
    {
        if(evt.source != viewport_)
            return;

        if(!stereo_mngr_->is_custom_projection_)
            camera_->setFrustumOffset(old_offset_);
        else
            camera_->setCustomProjectionMatrix(false);

        camera_->setPosition(old_pos_);

        camera_->getSceneManager()->setVisibilityMask(old_vis_mask_);
    }

    //-------------- Device Lost listener -------------------------------

    void StereoManager::DeviceLostListener::init(StereoManager *stereoMgr)
    {
        stereo_mngr_ = stereoMgr;
    }

    void StereoManager::DeviceLostListener::eventOccurred (const Ogre::String &eventName, const Ogre::NameValuePairList *parameters)
    {
        if(eventName == "DeviceRestored")
        {
            if(stereo_mngr_->compositor_instance_)
            {
                Ogre::Viewport *leftViewport, *rightViewport;
                stereo_mngr_->shutdownListeners();
                leftViewport = stereo_mngr_->compositor_instance_->getRenderTarget("Stereo/Left")->getViewport(0);
                rightViewport = stereo_mngr_->compositor_instance_->getRenderTarget("Stereo/Right")->getViewport(0);
                stereo_mngr_->initListeners(leftViewport, rightViewport);
            }
        }
    }

    //------------------------ init Stereo Manager --------------------------
    StereoManager::StereoManager(void)
    {
        stereo_mode_ = SM_NONE;
        debug_plane_ = NULL;
        debug_plane_node_ = NULL;
        left_viewport_ = NULL;
        right_viewport_ = NULL;
        camera_ = NULL;
        compositor_instance_ = NULL;
        focal_plane_fixed_ = false;
        scrn_width_ = 1.0f;
        eye_spacing_ = 0.06f;
        pix_offset_ = 0.0f;
        focal_length_ = 10.0f;
        infinite_focal_length_ = false;
        is_inversed_ = false;
        is_custom_projection_ = false;
        left_custom_projection_ = Ogre::Matrix4::IDENTITY;
        right_custom_projection_ = Ogre::Matrix4::IDENTITY;
        right_mask_ = ~((Ogre::uint32)0);
        left_mask_ = ~((Ogre::uint32)0);

        available_nodes_[SM_ANAGLYPH] = StereoModeDescription("ANAGLYPH_RED_CYAN", "Stereo/Anaglyph");
        //available_nodes_[SM_INTERLACED_H] = StereoModeDescription("INTERLACED_HORIZONTAL", "Stereo/HorizontalInterlace");
        //available_nodes_[SM_INTERLACED_V] = StereoModeDescription("INTERLACED_VERTICAL", "Stereo/VerticalInterlace");
        //available_nodes_[SM_INTERLACED_CB] = StereoModeDescription("INTERLACED_CHECKBOARD", "Stereo/CheckboardInterlace");

        available_nodes_[SM_DUALOUTPUT] = StereoModeDescription("DUALOUTPUT");
        available_nodes_[SM_SHUTTER] = StereoModeDescription("SHUTTER");
        available_nodes_[SM_NONE] = StereoModeDescription("NONE");
        }

    StereoManager::~StereoManager(void)
    {
        shutdown();
        destroyDebugPlane();
    }

    void StereoManager::init(Ogre::Viewport* leftViewport, Ogre::Viewport* rightViewport, StereoMode mode)
    {
        if(stereo_mode_ != SM_NONE)
            return;
        stereo_mode_ = mode;
        init(leftViewport, rightViewport);
    }

    void StereoManager::init(Ogre::Viewport* leftViewport, Ogre::Viewport* rightViewport, const Ogre::String &fileName)
    {
        if(stereo_mode_ != SM_NONE)
            return;
        stereo_mode_ = loadConfig(fileName);
        init(leftViewport, rightViewport);
    }

    void StereoManager::init(Ogre::Viewport* leftViewport, Ogre::Viewport* rightViewport)
    {
        if(stereo_mode_ == SM_NONE)
            return;
        if(!leftViewport)
            OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "At least left viewport must be provided",
            "StereoManager::init");

        camera_ = leftViewport->getCamera();
        if(!camera_ && rightViewport && !rightViewport->getCamera())
            OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "Viewports must have cameras associated",
            "StereoManager::init");

        if(rightViewport && camera_ != rightViewport->getCamera())
            OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "Left and right viewports must have the same camera associated",
            "StereoManager::init");

        if(available_nodes_[stereo_mode_].uses_compositor_)
        {
            Ogre::Viewport *newLeft, *newRight;
            initCompositor(leftViewport, available_nodes_[stereo_mode_].mat_name_, newLeft, newRight);
            leftViewport = newLeft;
            rightViewport = newRight;
        }

        initListeners(leftViewport, rightViewport);

        RenderTargetList::iterator it;
        RenderTargetList::iterator end = rendertarget_list_.end();
        for(it = rendertarget_list_.begin(); it != end; ++it)
        {
            it->first->setAutoUpdated(false);
        }

        bool infinite = infinite_focal_length_;
        setFocalLength(focal_length_); // setFocalLength will erase the infinite focal length option, so backup it and restore it
        setFocalLengthInfinite(infinite);

        if(focal_plane_fixed_)
            updateCamera(0);
    }

    void StereoManager::initListeners(Ogre::Viewport* leftViewport, Ogre::Viewport* rightViewport)
    {
        if(leftViewport)
        {
            left_cam_listener_.init(this, leftViewport, !is_inversed_);
            leftViewport->getTarget()->addListener(&left_cam_listener_);
            left_viewport_ = leftViewport;
        }

        if(rightViewport)
        {
            right_cam_listener_.init(this, rightViewport, is_inversed_);
            rightViewport->getTarget()->addListener(&right_cam_listener_);
            right_viewport_ = rightViewport;
        }
    }

    void StereoManager::shutdownListeners(void)
    {
        if(left_viewport_)
        {
            left_viewport_->getTarget()->removeListener(&left_cam_listener_);
            left_viewport_ = NULL;
        }
        if(right_viewport_)
        {
            right_viewport_->getTarget()->removeListener(&right_cam_listener_);
            right_viewport_ = NULL;
        }
    }

    void StereoManager::initCompositor(Ogre::Viewport *viewport, const Ogre::String &materialName, Ogre::Viewport *&out_left, Ogre::Viewport *&out_right)
    {
        compositor_viewport_ = viewport;
        compositor_instance_ = Ogre::CompositorManager::getSingleton().addCompositor(viewport, "Stereo/BaseCompositor");
        if(!compositor_instance_)
            OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "Cannot create compositor, missing StereoManager resources",
            "StereoManager::initCompositor");
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(viewport, "Stereo/BaseCompositor", true);

        Ogre::MaterialPtr mat = static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingleton().getByName(materialName));
        if(mat.isNull())
            OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, materialName + " not found, missing StereoManager resources",
            "StereoManager::initCompositor");

        compositor_instance_->getTechnique()->getOutputTargetPass()->getPass(0)->setMaterial(mat);
        out_left = compositor_instance_->getRenderTarget("Stereo/Left")->getViewport(0);
        out_right = compositor_instance_->getRenderTarget("Stereo/Right")->getViewport(0);


        device_list_listener_.init(this);
        Ogre::Root::getSingleton().getRenderSystem()->addListener(&device_list_listener_);
    }
    void StereoManager::shutdownCompositor()
    {
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(compositor_viewport_, "Stereo/BaseCompositor", false);
        Ogre::CompositorManager::getSingleton().removeCompositor(compositor_viewport_, "Stereo/BaseCompositor");


        Ogre::Root::getSingleton().getRenderSystem()->removeListener(&device_list_listener_);
        compositor_instance_ = NULL;
        compositor_viewport_ = NULL;
    }

    void StereoManager::shutdown(void)
    {
        if(stereo_mode_ == SM_NONE)
            return;

        shutdownListeners();
        if(available_nodes_[stereo_mode_].uses_compositor_)
            shutdownCompositor();

        RenderTargetList::iterator it;
        RenderTargetList::iterator end = rendertarget_list_.end();
        for(it = rendertarget_list_.begin(); it != end; ++it)
        {
            it->first->setAutoUpdated(it->second);
        }

        stereo_mode_ = SM_NONE;
    }
    //-------------------------- misc --------------

    void StereoManager::setVisibilityMask(Ogre::uint32 leftMask, Ogre::uint32 rightMask)
    {
        right_mask_ = rightMask;
        left_mask_ = leftMask;
    }

    void StereoManager::getVisibilityMask(Ogre::uint32 &outLeftMask, Ogre::uint32 &outRightMask) const
    {
        outRightMask = right_mask_;
        outLeftMask = left_mask_;
    }

    void StereoManager::addRenderTargetDependency(Ogre::RenderTarget *renderTarget)
    {
        if(rendertarget_list_.find(renderTarget) != rendertarget_list_.end())
            return;
        rendertarget_list_[renderTarget] = renderTarget->isAutoUpdated();
        renderTarget->setAutoUpdated(false);
    }

    void StereoManager::removeRenderTargetDependency(Ogre::RenderTarget *renderTarget)
    {
        if(rendertarget_list_.find(renderTarget) == rendertarget_list_.end())
            return;
        renderTarget->setAutoUpdated(rendertarget_list_[renderTarget]);
        rendertarget_list_.erase(renderTarget);
    }

    void StereoManager::updateAllDependentRenderTargets(bool isLeftEye)
    {
        Ogre::uint32 mask;
        if(isLeftEye)
        {
            mask = left_mask_;
        }
        else
        {
            mask = right_mask_;
        }

        RenderTargetList::iterator itarg, itargend;
        itargend = rendertarget_list_.end();
        for( itarg = rendertarget_list_.begin(); itarg != itargend; ++itarg )
        {
            Ogre::RenderTarget *rt = itarg->first;

            int n = rt->getNumViewports();
            std::vector<int> maskVector(n); // VS2005 gives a warning if I declare the vector as uint32 but not with int

            for(int i = 0; i<n ; ++i)
            {
                maskVector[i] = rt->getViewport(i)->getVisibilityMask();
                rt->getViewport(i)->setVisibilityMask(maskVector[i] & mask);
            }

            rt->update();

            for(int i = 0; i<n ; ++i)
            {
                rt->getViewport(i)->setVisibilityMask(maskVector[i]);
            }
        }
    }

    //---------------------------- Stereo tuning  ------------------------
    void StereoManager::setFocalLength(Ogre::Real l)
    {
        if(l == std::numeric_limits<Ogre::Real>::infinity())
        {
            setFocalLengthInfinite(true);
        }
        else
        {
            setFocalLengthInfinite(false);

            Ogre::Real old = focal_length_;
            focal_length_ = l;
            if( camera_ )
            {
                camera_->setFocalLength(focal_length_);
                if(focal_plane_fixed_)
                    updateCamera(focal_length_ - old);
                else if(debug_plane_)
                    updateDebugPlane();
            }
        }
    }

    Ogre::Real StereoManager::getFocalLength(void) const
    {
        if(infinite_focal_length_)
            return std::numeric_limits<Ogre::Real>::infinity();
        else
            return focal_length_;
    }

    void StereoManager::setFocalLengthInfinite(bool isInfinite)
    {
        infinite_focal_length_ = isInfinite;
        if(isInfinite)
            focal_plane_fixed_ = false;
    }

    void StereoManager::useScreenWidth(Ogre::Real w)
    {
        scrn_width_ = w;
        focal_plane_fixed_ = true;
        if( camera_ )
            updateCamera(0);
    }

    void StereoManager::updateCamera(Ogre::Real delta)
    {
        camera_->moveRelative(-delta * Ogre::Vector3::UNIT_Z);
        Ogre::Radian a = 2 * Ogre::Math::ATan(scrn_width_/(2 * focal_length_ * camera_->getAspectRatio()));
        camera_->setFOVy(a);
    }

    void StereoManager::inverseStereo(bool inverse)
    {
        is_inversed_ = inverse;
        left_cam_listener_.is_lefteye_ = !is_inversed_;
        right_cam_listener_.is_lefteye_ = is_inversed_;
    }

    void StereoManager::setCustomProjectonMatrices(bool enable, const Ogre::Matrix4 &leftMatrix, const Ogre::Matrix4 &rightMatrix)
    {
        is_custom_projection_ = enable;
        left_custom_projection_ = leftMatrix;
        right_custom_projection_ = rightMatrix;
    }

    void StereoManager::getCustomProjectonMatrices(bool &enabled, Ogre::Matrix4 &leftMatrix, Ogre::Matrix4 &rightMatrix) const
    {
        enabled = is_custom_projection_;
        leftMatrix = left_custom_projection_;
        rightMatrix = right_custom_projection_;
    }

    //------------------------------------ Debug focal plane ---------------------------------
    void StereoManager::enableDebugPlane(bool enable)
    {
        if(debug_plane_)
            debug_plane_->setVisible(enable);
    }

    void StereoManager::toggleDebugPlane(void)
    {
        if(debug_plane_)
            debug_plane_->setVisible(!debug_plane_->isVisible());
    }

    void StereoManager::createDebugPlane(Ogre::SceneManager *sceneMgr, const Ogre::String &leftMaterialName, const Ogre::String &rightMaterialName)
    {
        if(debug_plane_)
            return;

        sceneMngr_ = sceneMgr;
        Ogre::Plane screenPlane;
        screenPlane.normal = Ogre::Vector3::UNIT_Z;
        Ogre::MeshManager::getSingleton().createPlane("Stereo/Plane",
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            screenPlane,1,1,10,10);
        debug_plane_ = sceneMgr->createEntity( "Stereo/DebugPlane", "Stereo/Plane" );

        if(leftMaterialName == "")
        {
            left_mat_name_ = "Stereo/Wireframe";
        }
        else
        {
            left_mat_name_ = leftMaterialName;
        }

        if(rightMaterialName == "")
        {
            right_mat_name_ = "Stereo/Wireframe";
        }
        else
        {
            right_mat_name_ = rightMaterialName;
        }


        debug_plane_node_ = static_cast<Ogre::SceneNode*>(sceneMgr->getRootSceneNode()->createChild("Stereo/DebugPlaneNode"));
        debug_plane_node_->attachObject(debug_plane_);

        enableDebugPlane(true);
        updateDebugPlane();
    }

    void StereoManager::destroyDebugPlane(void)
    {
        if(debug_plane_)
        {
            Ogre::SceneNode *parent = static_cast<Ogre::SceneNode*>(debug_plane_node_->getParent());
            parent->removeAndDestroyChild("Stereo/DebugPlaneNode");
            debug_plane_node_ = NULL;
            sceneMngr_->destroyEntity("Stereo/DebugPlane");
            debug_plane_ = NULL;
            Ogre::MeshManager::getSingleton().remove("Stereo/Plane");
        }
    }

    void StereoManager::updateDebugPlane(void)
    {
        if(debug_plane_node_ && camera_)
        {
            Ogre::Real actualFocalLength = infinite_focal_length_ ? camera_->getFarClipDistance() * 0.99f : focal_length_;
            Ogre::Vector3 pos = camera_->getDerivedPosition();
            pos += camera_->getDerivedDirection() * actualFocalLength;
            debug_plane_node_->setPosition(pos);
            debug_plane_node_->setOrientation(camera_->getDerivedOrientation());
            Ogre::Vector3 scale;
            Ogre::Real height = actualFocalLength * Ogre::Math::Tan(camera_->getFOVy()/2)*2;
            scale.z = 1;
            scale.y = height;
            scale.x = height * camera_->getAspectRatio();
            debug_plane_node_->setScale(scale);
        }
    }

    void StereoManager::chooseDebugPlaneMaterial(bool isLeftEye)
    {
        if(debug_plane_)
        {
            if(isLeftEye)
                debug_plane_->setMaterialName(left_mat_name_);
            else
                debug_plane_->setMaterialName(right_mat_name_);
        }
    }


    //-------------------------------------- config ------------------------------------
    void StereoManager::saveConfig(const Ogre::String &filename) const
    {
        std::ofstream of(filename.c_str());
        if (!of)
            OGRE_EXCEPT(Ogre::Exception::ERR_CANNOT_WRITE_TO_FILE, "Cannot create settings file.",
            "StereoManager::saveConfig");

        of << "[Stereoscopy]" << std::endl;
        of << "# Available Modes: ";

        const StereoModeList::const_iterator end = available_nodes_.end();
        for(StereoModeList::const_iterator it = available_nodes_.begin(); it != end; ++it)
        {
            of << it->second.name_ << " ";
        }
        of << std::endl;

        StereoModeList::const_iterator it = available_nodes_.find(stereo_mode_);
        if(it != available_nodes_.end())
            of << "Stereo mode = "  << it->second.name_ << std::endl;
        else
            of << "Stereo mode = " << "NONE # wrong enum value, defaults to NONE" << std::endl;

        of << "Eyes spacing = " << eye_spacing_ << std::endl;

        of << "# Set to inf for parallel frustrum stereo." << std::endl;
        if(infinite_focal_length_)
            of << "Focal length = " << "inf"  << std::endl;
        else
            of << "Focal length = " << focal_length_ << std::endl;

        of << "Inverse stereo = " << (is_inversed_ ? "true" : "false") << std::endl;

        of << std::endl << "# For advanced use. See StereoManager.h for details." << std::endl;
        of << "Fixed screen = " << (focal_plane_fixed_ ? "true" : "false") << std::endl;
        of << "Screen width = " << scrn_width_ << std::endl;

        of.close();
    }

    StereoManager::StereoMode StereoManager::loadConfig(const Ogre::String &filename)
    {
        Ogre::ConfigFile cf;
        cf.load(filename.c_str());

        StereoMode mode;

        const Ogre::String &modeName = cf.getSetting("Stereo mode","Stereoscopy");
        const StereoModeList::const_iterator end = available_nodes_.end();
        StereoModeList::iterator it;
        for(it = available_nodes_.begin(); it != end; ++it)
        {
            if(it->second.name_ == modeName)
            {
                mode = it->first;
                break;
            }
        }
        if(it == available_nodes_.end())
            mode = SM_NONE;

        fixFocalPlanePos(Ogre::StringConverter::parseBool(cf.getSetting("Fixed screen","Stereoscopy")));

        if(cf.getSetting("Focal length","Stereoscopy") == "inf")
            infinite_focal_length_ = true;
        else
            setFocalLength(Ogre::StringConverter::parseReal(cf.getSetting("Focal length","Stereoscopy")));

        setEyesSpacing(Ogre::StringConverter::parseReal(cf.getSetting("Eyes spacing","Stereoscopy")));
        setScreenWidth(Ogre::StringConverter::parseReal(cf.getSetting("Screen width","Stereoscopy")));
        inverseStereo(Ogre::StringConverter::parseBool(cf.getSetting("Inverse stereo","Stereoscopy")));
        
        return mode;
    }

}