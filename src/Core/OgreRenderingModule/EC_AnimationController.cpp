// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#define MATH_OGRE_INTEROP
#include "DebugOperatorNew.h"
#include "EC_Mesh.h"
#include "EC_AnimationController.h"
#include "Entity.h"
#include "FrameAPI.h"
#include "OgreRenderingModule.h"
#include "CoreStringUtils.h"
#include "Profiler.h"
#include "Scene/Scene.h"
#include "OgreWorld.h"

#include <Ogre.h>

#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"


using namespace OgreRenderer;

EC_AnimationController::EC_AnimationController(Scene* scene) :
    IComponent(scene),
    animationState(this, "Animation state", ""),
    drawDebug(this, "Draw debug", false),
    mesh(0)
{
    ResetState();
    
    QObject::connect(framework->Frame(), SIGNAL(Updated(float)), this, SLOT(Update(float)));
    QObject::connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()));
}

EC_AnimationController::~EC_AnimationController()
{
}

void EC_AnimationController::SetMeshEntity(EC_Mesh *new_mesh)
{
    mesh = new_mesh;
}

QStringList EC_AnimationController::GetAvailableAnimations()
{
    QStringList availableList;
    Ogre::Entity* entity = GetEntity();
    if (!entity) 
        return availableList;
    Ogre::AnimationStateSet* anims = entity->getAllAnimationStates();
    if (!anims) 
        return availableList;
    Ogre::AnimationStateIterator i = anims->getAnimationStateIterator();
    while(i.hasMoreElements()) 
    {
        Ogre::AnimationState *animstate = i.getNext();
        availableList << QString(animstate->getAnimationName().c_str());
    }
    return availableList;
}

QStringList EC_AnimationController::GetActiveAnimations() const
{
    QStringList activeList;
    for(AnimationMap::const_iterator i = animations_.begin(); i != animations_.end(); ++i)
    {
        if (i->second.phase_ != PHASE_STOP)
            activeList << i->first;
    }
    
    return activeList;
}

void EC_AnimationController::Update(float frametime)
{
    Ogre::Entity* entity = GetEntity();
    if (!entity) 
        return;
    
    PROFILE(EC_AnimationController_Update);

    std::vector<QString> erase_list;
    
    // Loop through all animations & update them as necessary
    for(AnimationMap::iterator i = animations_.begin(); i != animations_.end(); ++i)
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

                        float rewindpos = i->second.speed_factor_ >= 0.f ? (animstate->getTimePosition() - animstate->getLength()) : animstate->getLength();
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
            float advance = i->second.speed_factor_ * frametime;
            float new_weight = i->second.weight_ * i->second.weight_factor_;
            
            bool cycled = false;
            float oldtimepos = animstate->getTimePosition();
            float animlength = animstate->getLength();
            
            if (new_weight != animstate->getWeight())
                animstate->setWeight((float)i->second.weight_ * i->second.weight_factor_);
            if (advance != 0.0f)
                animstate->addTime((float)(i->second.speed_factor_ * frametime));
            if (!animstate->getEnabled())
                animstate->setEnabled(true);
            
            // Check if we should fire an "animation finished" signal
            float newtimepos = animstate->getTimePosition();
            if (advance > 0.0f)
            {
                if (!animstate->getLoop())
                {
                    if ((oldtimepos < animlength) && (newtimepos >= animlength))
                        cycled = true;
                }
                else
                {
                    if (newtimepos < oldtimepos)
                        cycled = true;
                }
            }
            else
            {
                if (!animstate->getLoop())
                {
                    if ((oldtimepos > 0.0f) && (newtimepos == 0.0f))
                        cycled = true;
                }
                else
                {
                    if (newtimepos > oldtimepos)
                        cycled = true;
                }
            }
            
            if (cycled)
            {
                if (animstate->getLoop())
                    emit AnimationCycled(QString::fromStdString(animstate->getAnimationName()));
                else
                    emit AnimationFinished(QString::fromStdString(animstate->getAnimationName()));
            }
        }
        else
        {
            // If stopped, disable & remove this animation from list
            animstate->setEnabled(false);
            erase_list.push_back(i->first);
        }
    }
    
    for(uint i = 0; i < erase_list.size(); ++i)
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

        for(uint i = 0; i < skel->getNumBones(); ++i)
        {
            highpriority_mask_[i] = 1.0;
            lowpriority_mask_[i] = 1.0;
        }

        // Loop through all high priority animations & update the lowpriority-blendmask based on their active tracks
        for(AnimationMap::iterator i = animations_.begin(); i != animations_.end(); ++i)
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
                while(it.hasMoreElements())
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
        for(AnimationMap::iterator i = animations_.begin(); i != animations_.end(); ++i)
        {
            Ogre::AnimationState* animstate = GetAnimationState(entity, i->first);
            if (!animstate)
                continue;    
            if (i->second.high_priority_ == false)
                animstate->_setBlendMaskData(&lowpriority_mask_[0]);
        }
    }
    if (getdrawDebug())
        DrawSkeleton(frametime);
}

void EC_AnimationController::DrawSkeleton(float frametime)
{
    Ogre::Entity *oEnt = GetEntity();
    if (oEnt && oEnt->hasSkeleton())
    {
        Scene *scene = ParentScene();
        if (!scene)
            return;

        OgreWorldPtr world = scene->GetWorld<OgreWorld>();
        if (!world)
            return;

        Ogre::Vector3 parentPos = Ogre::Vector3::ZERO;
        Ogre::Quaternion parentOrt = Ogre::Quaternion::IDENTITY;
        if (oEnt->getParentNode())
        {
            parentPos = oEnt->getParentNode()->_getDerivedPosition();
            parentOrt = oEnt->getParentNode()->_getDerivedOrientation();
        }

        Ogre::SkeletonInstance *skeleton = oEnt->getSkeleton();
        bool red = true;
        for(ushort i = 0; i < skeleton->getNumBones(); ++i)
        {
            Ogre::Bone *bone = skeleton->getBone(i);
            Ogre::Vector3 pos = parentPos + (parentOrt * bone->_getDerivedPosition());
            for(ushort ci = 0; ci < bone->numChildren(); ++ci)
            {
                Ogre::Vector3 childPos = parentPos + (parentOrt * bone->getChild(ci)->_getDerivedPosition());
                if (red)
                    world->DebugDrawLine(pos, childPos, 1.f, 0.f, 0.f, false);
                else
                    world->DebugDrawLine(pos, childPos, 0.f, 1.f, 0.f, false);
            }
            red = !red;
        }
    }
}

Ogre::Entity* EC_AnimationController::GetEntity()
{
    if (!mesh)
        AutoSetMesh();

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

void EC_AnimationController::ResetState()
{
    animations_.clear();
}

/// Finds an animation state from Ogre::AnimationStateSet by name, performing a case-insensitive name search.
/// @note This function is O(n), while normal set search would be O(logN) or O(1).
Ogre::AnimationState *OgreAnimStateSetFindNoCase(Ogre::AnimationStateSet *set, const QString &animState)
{
    if (!set)
        return 0;

    Ogre::AnimationStateIterator iter = set->getAnimationStateIterator();
    while(iter.hasMoreElements())
    {
        Ogre::AnimationState *s = iter.getNext();
        if (QString::compare(s->getAnimationName().c_str(), animState, Qt::CaseInsensitive) == 0)
            return s;
    }

    return 0;
}

Ogre::AnimationState* EC_AnimationController::GetAnimationState(Ogre::Entity* entity, const QString& name)
{
    if (!entity)
        return 0;
        
    return OgreAnimStateSetFindNoCase(entity->getAllAnimationStates(), name);
}

bool EC_AnimationController::EnableExclusiveAnimation(const QString& name, bool looped, float fadein, float fadeout, bool high_priority)
{
    // Disable all other active animations
    AnimationMap::iterator i = animations_.begin();
    while(i != animations_.end())
    {
        const QString& other_name = i->first;
        if (!(QString::compare(other_name, name, Qt::CaseInsensitive) == 0))
        {
            i->second.phase_ = PHASE_FADEOUT;
            i->second.fade_period_ = fadeout;
        }
        ++i;
    }

    // Then enable this
    return EnableAnimation(name, looped, fadein, high_priority);
}

bool EC_AnimationController::EnableAnimation(const QString& name, bool looped, float fadein, bool high_priority)
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
        // If animation is nonlooped and has already reached end, rewind to beginning
        if ((!looped) && (i->second.speed_factor_ > 0.0f))
        {
            if (animstate->getTimePosition() >= animstate->getLength())
                animstate->setTimePosition(0.0f);
        }
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

bool EC_AnimationController::HasAnimationFinished(const QString& name)
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

bool EC_AnimationController::IsAnimationActive(const QString& name, bool check_fadeout)
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

bool EC_AnimationController::SetAnimationAutoStop(const QString& name, bool enable)
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

bool EC_AnimationController::SetAnimationNumLoops(const QString& name, uint repeats)
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

bool EC_AnimationController::DisableAnimation(const QString& name, float fadeout)
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

void EC_AnimationController::DisableAllAnimations(float fadeout)
{
    AnimationMap::iterator i = animations_.begin();
    while(i != animations_.end())
    {
        i->second.phase_ = PHASE_FADEOUT;
        i->second.fade_period_ = fadeout;
        ++i;
    }
}

void EC_AnimationController::SetAnimationToEnd(const QString& name)
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

bool EC_AnimationController::SetAnimationSpeed(const QString& name, float speedfactor)
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

bool EC_AnimationController::SetAnimationWeight(const QString& name, float weight)
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

bool EC_AnimationController::SetAnimationPriority(const QString& name, bool high_priority)
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

bool EC_AnimationController::SetAnimationTimePosition(const QString& name, float newPosition)
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

bool EC_AnimationController::SetAnimationRelativeTimePosition(const QString& name, float newPosition)
{
    Ogre::Entity* entity = GetEntity();
    Ogre::AnimationState* animstate = GetAnimationState(entity, name);
    if (!animstate) 
        return false;
        
    // See if we find this animation in the list of active animations
    AnimationMap::iterator i = animations_.find(name);
    if (i != animations_.end())
    {
        animstate->setTimePosition(Clamp(newPosition, 0.0f, 1.0f) * animstate->getLength());
        return true;
    }
    // Animation not active
    return false;
}

float EC_AnimationController::GetAnimationLength(const QString& name)
{
    Ogre::Entity* entity = GetEntity();
    Ogre::AnimationState* animstate = GetAnimationState(entity, name);
    if (!animstate)
        return 0.0f;
    else
        return animstate->getLength();
}

float EC_AnimationController::GetAnimationTimePosition(const QString& name)
{
    Ogre::Entity* entity = GetEntity();
    Ogre::AnimationState* animstate = GetAnimationState(entity, name);
    if (!animstate)
        return 0.0f;
    
    // See if we find this animation in the list of active animations
    AnimationMap::iterator i = animations_.find(name);
    if (i != animations_.end())
        return animstate->getTimePosition();
    else return 0.0f;
}

float EC_AnimationController::GetAnimationRelativeTimePosition(const QString& name)
{
    Ogre::Entity* entity = GetEntity();
    Ogre::AnimationState* animstate = GetAnimationState(entity, name);
    if (!animstate)
        return 0.0f;
    
    // See if we find this animation in the list of active animations
    AnimationMap::iterator i = animations_.find(name);
    if (i != animations_.end())
        return animstate->getTimePosition() / animstate->getLength();
    else return 0.0f;
}

void EC_AnimationController::UpdateSignals()
{
    Entity* parent = ParentEntity();
    if (parent)
    {
        parent->ConnectAction("PlayAnim", this, SLOT(PlayAnim(const QString &, const QString &, const QString &)));
        parent->ConnectAction("PlayLoopedAnim", this, SLOT(PlayLoopedAnim(const QString &, const QString &, const QString &)));
        parent->ConnectAction("PlayReverseAnim", this, SLOT(PlayReverseAnim(const QString &, const QString &, const QString &)));
        parent->ConnectAction("PlayAnimAutoStop", this, SLOT(PlayAnimAutoStop(const QString &, const QString &, const QString &)));
        parent->ConnectAction("StopAnim", this, SLOT(StopAnim(const QString &, const QString &)));
        parent->ConnectAction("StopAllAnims", this, SLOT(StopAllAnims(const QString &)));
        parent->ConnectAction("SetAnimSpeed", this, SLOT(SetAnimSpeed(const QString &, const QString &)));
        parent->ConnectAction("SetAnimWeight", this, SLOT(SetAnimWeight(const QString &, const QString &)));

        // Connect to ComponentRemoved signal of the parent entity, so we can check if the mesh gets removed
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), this, SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)));
    }
}

void EC_AnimationController::AutoSetMesh()
{
    if (!mesh)
    {
        Entity* parent = ParentEntity();
        if (parent)
        {
            EC_Mesh* mesh = parent->GetComponent<EC_Mesh>().get();
            if (mesh)
                SetMeshEntity(mesh);
        }
    }
}

void EC_AnimationController::OnComponentRemoved(IComponent* component, AttributeChange::Type change)
{
    if (component == mesh)
        SetMeshEntity(0);
}

void EC_AnimationController::PlayAnim(const QString &name, const QString &fadein, const QString &exclusive)
{
    if (!ViewEnabled())
        return;

    if (!name.length())
    {
        LogWarning("Empty animation name for PlayAnim");
        return;
    }
    
    float fadein_ = 0.0f;
    if (fadein.length())
        fadein_ = fadein.toFloat();
    bool exclusive_ = false;
    if (exclusive.length())
        exclusive_ = ParseBool(exclusive);

    bool success;
    if (exclusive_)
        success = EnableExclusiveAnimation(name, false, fadein_, false);
    else
        success = EnableAnimation(name, false, fadein_, false);
    if (!success)
    {
        QStringList anims = GetAvailableAnimations();
        void (*log)(const QString &) = LogDebug; if (anims.length() > 0) log = LogWarning;
        log("Failed to play animation \"" + name + "\" on entity " + ParentEntity()->Name());
        log("The entity has " + QString::number(anims.length()) + " animations available: " + anims.join(","));
    }
}

void EC_AnimationController::PlayLoopedAnim(const QString &name, const QString &fadein, const QString &exclusive)
{
    if (!ViewEnabled())
        return;

    if (!name.length())
    {
        LogWarning("Empty animation name for PlayLoopedAnim");
        return;
    }
    
    float fadein_ = 0.0f;
    if (fadein.length())
        fadein_ = fadein.toFloat();
    bool exclusive_ = false;
    if (exclusive.length())
        exclusive_ = ParseBool(exclusive);

    bool success;
    if (exclusive_)
        success = EnableExclusiveAnimation(name, true, fadein_, fadein_, false);
    else
        success = EnableAnimation(name, true, fadein_, false);
    if (!success)
    {
        QStringList anims = GetAvailableAnimations();
        void (*log)(const QString &) = LogDebug; if (anims.length() > 0) log = LogWarning;
        log("Failed to play looped animation \"" + name + "\" on entity " + ParentEntity()->Name());
        log("The entity has " + QString::number(anims.length()) + " animations available: " + anims.join(","));
    }
}

void EC_AnimationController::PlayReverseAnim(const QString &name, const QString &fadein, const QString &exclusive)
{
    if (!ViewEnabled())
        return;

    if (!name.length())
    {
        LogWarning("Empty animation name for PlayReverseAnim");
        return;
    }
    
    float fadein_ = 0.0f;
    if (fadein.length())
        fadein_ = fadein.toFloat();
    bool exclusive_ = false;
    if (exclusive.length())
        exclusive_ = ParseBool(exclusive);
    bool success;
    if (exclusive_)
        success = EnableAnimation(name, true, fadein_, false);
    else
        success = EnableExclusiveAnimation(name, true, fadein_, fadein_, false);
    if (!success)
    {
        QStringList anims = GetAvailableAnimations();
        void (*log)(const QString &) = LogDebug; if (anims.length() > 0) log = LogWarning;
        log("Failed to play animation \"" + name + "\" in reverse on entity " + ParentEntity()->Name());
        log("The entity has " + QString::number(anims.length()) + " animations available: " + anims.join(","));

        SetAnimationToEnd(name);
        SetAnimationSpeed(name, -1.0f);
    }
}

void EC_AnimationController::PlayAnimAutoStop(const QString &name, const QString &fadein, const QString &exclusive)
{
    if (!name.length())
    {
        LogWarning("Empty animation name for PlayAnimAutoStop");
        return;
    }
    
    float fadein_ = 0.0f;
    if (fadein.length())
        fadein_ = fadein.toFloat();
    bool exclusive_ = false;
    if (exclusive.length())
        exclusive_ = ParseBool(exclusive);
    bool success;
    if (exclusive_)
        success = EnableExclusiveAnimation(name, false, fadein_, false);
    else
        success = EnableAnimation(name, false, fadein_, false);

    if (!success)
    {
        QStringList anims = GetAvailableAnimations();
        void (*log)(const QString &) = LogDebug; if (anims.length() > 0) log = LogWarning;
        log("Failed to play animation \"" + name + "\" on entity " + ParentEntity()->Name());
        log("The entity has " + QString::number(anims.length()) + " animations available: " + anims.join(","));

        // Enable autostop, and start always from the beginning
        SetAnimationAutoStop(name, true);
        SetAnimationTimePosition(name, 0.0f);
    }
}

void EC_AnimationController::StopAnim(const QString &name, const QString &fadeout)
{
    if (!name.length())
    {
        LogWarning("Empty animation name for StopAnim");
        return;
    }
    
    float fadeout_ = 0.0f;
    if (fadeout.length())
        fadeout_ = fadeout.toFloat();
    DisableAnimation(name, fadeout_);
}

void EC_AnimationController::StopAllAnims(const QString &fadeout)
{
    float fadeout_ = 0.0f;
    if (fadeout.length())
        fadeout_ = fadeout.toFloat();
    DisableAllAnimations(fadeout_);
}

void EC_AnimationController::SetAnimSpeed(const QString &name, const QString &animspeed)
{
    if (!name.length())
    {
        LogWarning("Empty animation name for SetAnimSpeed");
        return;
    }
    if (!animspeed.length())
    {
        LogWarning("No animation speed specified for SetAnimSpeed");
        return;
    }
    
    float speed = animspeed.toFloat();
    SetAnimationSpeed(name, speed);
}

void EC_AnimationController::SetAnimWeight(const QString &name, const QString &animweight)
{
    if (!name.length())
    {
        LogWarning("Empty animation name for SetAnimWeight");
        return;
    }
    if (!animweight.length())
    {
        LogWarning("No animation weight specified for SetAnimWeight");
        return;
    }
    
    float weight = animweight.toFloat();
    SetAnimationWeight(name, weight);
}

