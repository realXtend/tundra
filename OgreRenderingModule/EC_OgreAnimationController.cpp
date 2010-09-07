// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_OgreMesh.h"
#include "EC_OgreAnimationController.h"
#include "OgreRenderingModule.h"

#include <Ogre.h>

namespace OgreRenderer
{
    EC_OgreAnimationController::EC_OgreAnimationController(Foundation::ModuleInterface* module) :
        Foundation::ComponentInterface(module->GetFramework())
    {
        ResetState();
    }
    
    EC_OgreAnimationController::~EC_OgreAnimationController()
    {
    }
    
    void EC_OgreAnimationController::SetMeshEntity(EC_OgreMesh *new_mesh)
    {
        mesh = new_mesh;
    }
    
    QStringList EC_OgreAnimationController::GetAvailableAnimations()
    {
        QStringList availableList;
        Ogre::Entity* entity = GetEntity();
        if (!entity) return availableList;
            
        Ogre::AnimationStateSet* anims = entity->getAllAnimationStates();
        Ogre::AnimationStateIterator i = anims->getAnimationStateIterator();
        while(i.hasMoreElements()) {
            Ogre::AnimationState *animstate = i.getNext();
            availableList << QString(animstate->getAnimationName().c_str());
        }
        
        return availableList;
    }
    
    void EC_OgreAnimationController::Update(f64 frametime)
    {
        Ogre::Entity* entity = GetEntity();
        if (!entity) return;
        
        std::vector<std::string> erase_list;
        
        // Loop through all animations & update them as necessary
        for (AnimationMap::iterator i = animations_.begin(); i != animations_.end(); ++i)
        {
            Ogre::AnimationState* animstate = GetAnimationState(entity, i->first);
            if (!animstate)
                continue;
                
            switch(i->second.phase_)
            {
            case PHASE_FADEIN:
                // If period is infinitely fast, skip to full weight & PLAY status
                if (i->second.fade_period_ == 0.0f)
                {
                    i->second.weight_ = 1.0f;
                    i->second.phase_ = PHASE_PLAY;
                }   
                else
                {
                    i->second.weight_ += (1.0f / i->second.fade_period_) * frametime;
                    if (i->second.weight_ >= 1.0f)
                    {
                        i->second.weight_ = 1.0f;
                        i->second.phase_ = PHASE_PLAY;
                    }
                }
                break;

            case PHASE_PLAY:
                if (i->second.auto_stop_ || i->second.num_repeats_ != 1)
                {
                    if ((i->second.speed_factor_ >= 0.f && animstate->getTimePosition() >= animstate->getLength()) ||
                        (i->second.speed_factor_ < 0.f && animstate->getTimePosition() <= 0.f))
                    {
                        if (i->second.num_repeats_ != 1)
                        {
                            if (i->second.num_repeats_ > 1)
                                i->second.num_repeats_--;

                            Ogre::Real rewindpos = i->second.speed_factor_ >= 0.f ? (animstate->getTimePosition() - animstate->getLength()) : animstate->getLength();
                            animstate->setTimePosition(rewindpos);
                        }
                        else
                        {
                            i->second.phase_ = PHASE_FADEOUT;
                        }
                    }
                }
                break;    

            case PHASE_FADEOUT:
                // If period is infinitely fast, skip to disabled status immediately
                if (i->second.fade_period_ == 0.0f)
                {
                    i->second.weight_ = 0.0f;
                    i->second.phase_ = PHASE_STOP;
                }
                else
                {
                    i->second.weight_ -= (1.0f / i->second.fade_period_) * frametime;
                    if (i->second.weight_ <= 0.0f)
                    {
                        i->second.weight_ = 0.0f;
                        i->second.phase_ = PHASE_STOP;
                    }
                }
                break;
            }

            // Set weight & step the animation forward
            if (i->second.phase_ != PHASE_STOP)
            {
                Ogre::Real advance = i->second.speed_factor_ * frametime;
                Ogre::Real new_weight = i->second.weight_ * i->second.weight_factor_;
                
                if (new_weight != animstate->getWeight())
                    animstate->setWeight((Ogre::Real)i->second.weight_ * i->second.weight_factor_);
                if (advance != 0.0f)
                    animstate->addTime((Ogre::Real)(i->second.speed_factor_ * frametime));
                if (!animstate->getEnabled())
                    animstate->setEnabled(true);
            }
            else
            {
                // If stopped, disable & remove this animation from list
                animstate->setEnabled(false);
                erase_list.push_back(i->first);
            }
        }
        
        for (uint i = 0; i < erase_list.size(); ++i)
        {
            animations_.erase(erase_list[i]);
        }
        
        // High-priority/low-priority blending code
        if (entity->hasSkeleton())
        {
            Ogre::SkeletonInstance* skel = entity->getSkeleton();
            if (!skel)
                return;
                
            if (highpriority_mask_.size() != skel->getNumBones())
                highpriority_mask_.resize(skel->getNumBones());
            if (lowpriority_mask_.size() != skel->getNumBones())
                lowpriority_mask_.resize(skel->getNumBones());

            for (uint i = 0; i < skel->getNumBones(); ++i)
            {
                highpriority_mask_[i] = 1.0;
                lowpriority_mask_[i] = 1.0;
            }

            // Loop through all high priority animations & update the lowpriority-blendmask based on their active tracks
            for (AnimationMap::iterator i = animations_.begin(); i != animations_.end(); ++i)
            {
                Ogre::AnimationState* animstate = GetAnimationState(entity, i->first);
                if (!animstate)
                    continue;            
                // Create blend mask if animstate doesn't have it yet
                if (!animstate->hasBlendMask())
                    animstate->createBlendMask(skel->getNumBones());

                if ((i->second.high_priority_) && (i->second.weight_ > 0.0))
                {
                    // High-priority animations get the full weight blend mask
                    animstate->_setBlendMaskData(&highpriority_mask_[0]);
                    if (!skel->hasAnimation(animstate->getAnimationName()))
                        continue;
                        
                    Ogre::Animation* anim = skel->getAnimation(animstate->getAnimationName());
                    
                    Ogre::Animation::NodeTrackIterator it = anim->getNodeTrackIterator();
                    while (it.hasMoreElements())
                    {
                        Ogre::NodeAnimationTrack* track = it.getNext();
                        unsigned id = track->getHandle();
                        // For each active track, reduce corresponding bone weight in lowpriority-blendmask 
                        // by this animation's weight
                        if (id < lowpriority_mask_.size())
                        {
                            lowpriority_mask_[id] -= i->second.weight_;
                            if (lowpriority_mask_[id] < 0.0) lowpriority_mask_[id] = 0.0;
                        }
                    }
                }
            }

            // Now set the calculated blendmask on low-priority animations
            for (AnimationMap::iterator i = animations_.begin(); i != animations_.end(); ++i)
            {
                Ogre::AnimationState* animstate = GetAnimationState(entity, i->first);
                if (!animstate)
                    continue;    
                if (i->second.high_priority_ == false)
                    animstate->_setBlendMaskData(&lowpriority_mask_[0]);
            }
        }
    }
    
    Ogre::Entity* EC_OgreAnimationController::GetEntity()
    {
        if (!mesh)
            return 0;
        
        Ogre::Entity* entity = mesh->GetEntity();
        if (!entity)
            return 0;
        
        if (entity->getMesh()->getName() != mesh_name_)
        {
            mesh_name_ = entity->getMesh()->getName();
            ResetState();
        }
        
        return entity;
    }
    
    void EC_OgreAnimationController::ResetState()
    {
        animations_.clear();
    }
    
    Ogre::AnimationState* EC_OgreAnimationController::GetAnimationState(Ogre::Entity* entity, const std::string& name)
    {
        if (!entity)
            return 0;
            
        Ogre::AnimationStateSet* anims = entity->getAllAnimationStates();
        if (!anims)
            return 0;
            
        if (anims->hasAnimationState(name))
            return anims->getAnimationState(name);
        else
            return 0;
    }
    
    bool EC_OgreAnimationController::EnableExclusiveAnimation(const std::string& name, bool looped, Real fadein, Real fadeout, bool high_priority)
    {
        // Disable all other active animations
        AnimationMap::iterator i = animations_.begin();
        while (i != animations_.end())
        {
            const std::string& other_name = i->first;
            if (other_name != name)
            {
                i->second.phase_ = PHASE_FADEOUT;
                i->second.fade_period_ = fadeout;
            }
            ++i;
        }

        // Then enable this
        return EnableAnimation(name, looped, fadein, high_priority);
    }

    bool EC_OgreAnimationController::EnableAnimation(const std::string& name, bool looped, Real fadein, bool high_priority)
    {
        Ogre::Entity* entity = GetEntity();
        Ogre::AnimationState* animstate = GetAnimationState(entity, name);
        if (!animstate) 
            return false;

        animstate->setLoop(looped);

        // See if we already have this animation
        AnimationMap::iterator i = animations_.find(name);
        if (i != animations_.end())
        {
            i->second.phase_ = PHASE_FADEIN;
            i->second.num_repeats_ = (looped ? 0: 1);
            i->second.fade_period_ = fadein;
            i->second.high_priority_ = high_priority;
            return true;
        }
        
        // Start new animation from zero weight & speed factor 1, also reset time position
        animstate->setTimePosition(0.0f);
        
        Animation newanim;
        newanim.phase_ = PHASE_FADEIN;
        newanim.num_repeats_ = (looped ? 0: 1); // if looped, repeat 0 times (loop indefinetly) otherwise repeat one time.
        newanim.fade_period_ = fadein;
        newanim.high_priority_ = high_priority;

        animations_[name] = newanim;

        return true;
    }

    bool EC_OgreAnimationController::HasAnimationFinished(const std::string& name)
    {
        Ogre::Entity* entity = GetEntity();
        Ogre::AnimationState* animstate = GetAnimationState(entity, name);
        if (!animstate) 
            return false;

        AnimationMap::iterator i = animations_.find(name);
        if (i != animations_.end())
        {
            if ((!animstate->getLoop()) && ((i->second.speed_factor_ >= 0.f && animstate->getTimePosition() >= animstate->getLength()) ||
                (i->second.speed_factor_ < 0.f && animstate->getTimePosition() <= 0.f)))
                return true;
            else
                return false;
        }

        // Animation not listed, must be finished
        return true;
    }

    bool EC_OgreAnimationController::IsAnimationActive(const std::string& name, bool check_fadeout)
    {
        AnimationMap::iterator i = animations_.find(name);
        if (i != animations_.end())
        {
            if (check_fadeout)
                return true;
            else 
            {
                if (i->second.phase_ != PHASE_FADEOUT)
                    return true;
                else
                    return false;
            }
        }

        return false;
    }

    bool EC_OgreAnimationController::SetAnimationAutoStop(const std::string& name, bool enable)
    {
        AnimationMap::iterator i = animations_.find(name);
        if (i != animations_.end())
        {
            i->second.auto_stop_ = enable;
            return true;
        }

        // Animation not active
        return false;
    }
    
    bool EC_OgreAnimationController::SetAnimationNumLoops(const std::string& name, uint repeats)
    {
        AnimationMap::iterator i = animations_.find(name);
        if (i != animations_.end())
        {
            i->second.num_repeats_ = repeats;
            return true;
        }
        // Animation not active
        return false;
    }

    bool EC_OgreAnimationController::DisableAnimation(const std::string& name, Real fadeout)
    {
        AnimationMap::iterator i = animations_.find(name);
        if (i != animations_.end())
        {
            i->second.phase_ = PHASE_FADEOUT;
            i->second.fade_period_ = fadeout;
            return true;
        }
        // Animation not active
        return false;
    }

    void EC_OgreAnimationController::DisableAllAnimations(Real fadeout)
    {
        AnimationMap::iterator i = animations_.begin();
        while (i != animations_.end())
        {
            i->second.phase_ = PHASE_FADEOUT;
            i->second.fade_period_ = fadeout;
            ++i;
        }
    }

    void EC_OgreAnimationController::SetAnimationToEnd(const std::string& name)
    {
        Ogre::Entity* entity = GetEntity();
        Ogre::AnimationState* animstate = GetAnimationState(entity, name);
        if (!animstate) 
            return;
            
        if (animstate)
        {
            SetAnimationTimePosition(name, animstate->getLength());
        }
    }

    bool EC_OgreAnimationController::SetAnimationSpeed(const std::string& name, Real speedfactor)
    {
        AnimationMap::iterator i = animations_.find(name);
        if (i != animations_.end())
        {
            i->second.speed_factor_ = speedfactor;
            return true;
        }
        // Animation not active
        return false;
    }

    bool EC_OgreAnimationController::SetAnimationWeight(const std::string& name, Real weight)
    {
        AnimationMap::iterator i = animations_.find(name);
        if (i != animations_.end())
        {
            i->second.weight_factor_ = weight;
            return true;
        }
        // Animation not active
        return false;
    }

    bool EC_OgreAnimationController::SetAnimationPriority(const std::string& name, bool high_priority)
    {
        AnimationMap::iterator i = animations_.find(name);
        if (i != animations_.end())
        {
            i->second.high_priority_ = high_priority;
            return true;
        }
        // Animation not active
        return false;
    }    

    bool EC_OgreAnimationController::SetAnimationTimePosition(const std::string& name, Real newPosition)
    {
        Ogre::Entity* entity = GetEntity();
        Ogre::AnimationState* animstate = GetAnimationState(entity, name);
        if (!animstate) 
            return false;
            
        // See if we find this animation in the list of active animations
        AnimationMap::iterator i = animations_.find(name);
        if (i != animations_.end())
        {
            animstate->setTimePosition(newPosition);
            return true;
        }
        // Animation not active
        return false;
    }
}