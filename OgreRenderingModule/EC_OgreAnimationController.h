// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreAnimationController_h
#define incl_OgreRenderer_EC_OgreAnimationController_h

#include "ComponentInterface.h"
#include "OgreModuleApi.h"
#include "Declare_EC.h"

namespace Ogre
{
    class Entity;
}

#include <OgreAnimationState.h>
#include <QStringList>

namespace OgreRenderer
{

	class EC_OgreMesh;

    //! Ogre-specific mesh entity animation controller
    /*! Needs to be told of an EC_OgreMesh component to be usable
        \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API EC_OgreAnimationController : public Foundation::ComponentInterface
    {
        Q_OBJECT
        
        DECLARE_EC(EC_OgreAnimationController);

    public slots:
	//! Qt wrappers for py&js access. unnencessary if we switch to qstring etc.
        bool EnableAnimation(const QString &name) { return EnableAnimation(name.toStdString(), false); }
        bool SetAnimationTimePosition(const QString name, float new_position) { return SetAnimationTimePosition(name.toStdString(), (Real)new_position); }
        QStringList GetAvailableAnimations();

        //! Gets mesh entity component
        OgreRenderer::EC_OgreMesh *GetMeshEntity() const { return mesh; }
        
        //! Gets mesh entity component
        void SetMeshEntity(OgreRenderer::EC_OgreMesh *new_mesh);

    public:
        
        //! Enumeration of animation phase
        enum AnimationPhase
        {
            PHASE_FADEIN,
            PHASE_PLAY,
            PHASE_FADEOUT,
            PHASE_STOP,
            PHASE_FREE //in external control. for dynamiccomponent testing now
        };

        //! Structure for an ongoing animation
        struct Animation
        {
            //! Autostop at end (default false)
            bool auto_stop_;

            //! Time in milliseconds it takes to fade in/out an animation completely
            Real fade_period_;
            
            //! Weight of an animation in animation blending, maximum 1.0
            Real weight_;

            //! Weight adjust
            Real weight_factor_;

            //! How an animation is sped up or slowed down, default 1.0 (original speed)
            Real speed_factor_;

            //! loop animation through num_repeats times, or loop if zero
            uint num_repeats_;

            //! priority. high priority will reduce the weight of low priority animations, if exists on the same bone tracks
            bool high_priority_;
            
            //! current phase            
            AnimationPhase phase_;

            Animation() :
                auto_stop_(false),
                fade_period_(0.0),
                weight_(0.0),
                weight_factor_(1.0),
                speed_factor_(1.0),
                num_repeats_(0),
                high_priority_(false),
                phase_(PHASE_STOP)
            {
            }
        };
        
        typedef std::map<std::string, Animation> AnimationMap;

        virtual ~EC_OgreAnimationController();
        
        //! Updates animation(s) by elapsed time
        void Update(f64 frametime);
        
        //! Enables animation, with optional fade-in period. Returns true if success (animation exists)
        bool EnableAnimation(const std::string& name, bool looped = true, Real fadein = 0.0f, bool high_priority = false);

        //! Enables an exclusive animation (fades out all other animations with fadeOut parameter)
        bool EnableExclusiveAnimation(const std::string& name, bool looped, Real fadein = 0.0f, Real fadeout = 0.0f, bool high_priority = false);

        //! Checks whether non-looping animation has finished
        /*! If looping, returns always false
        */
        bool HasAnimationFinished(const std::string& name);

        //! Checks whether animation is active
        //! \param check_fade_out if true, also fade-out (until totally faded) phase is interpreted as "active"
        bool IsAnimationActive(const std::string& name, bool check_fadeout = true);

        //! Disables animation, with optional fade-out period. Returns true if success (animation exists)
        bool DisableAnimation(const std::string& name, Real fadeout = 0.0f);

        //! Disables all animations with the same fadeout period
        void DisableAllAnimations(Real fadeout = 0.0f);

        //! Forwards animation to end, useful if animation is played in reverse
        void SetAnimationToEnd(const std::string& name);

        //! Sets relative speed of active animation. Once disabled, the speed is forgotten! Returns true if success (animation exists)
        bool SetAnimationSpeed(const std::string& name, Real speedfactor);

        //! Changes weight of an active animation (default 1.0). Return false if the animation doesn't exist or isn't active
        bool SetAnimationWeight(const std::string& name, Real weight);
        
        //! Changes animation priority. Can lead to fun visual effects, but provided for completeness
        bool SetAnimationPriority(const std::string& name, bool high_priority);

        //! Sets time position of an active animation.
        bool SetAnimationTimePosition(const std::string& name, Real new_position);

        //! Sets autostop on animation
        bool SetAnimationAutoStop(const std::string& name, bool enable);

        //! Sets number of times the animation is repeated (0 = repeat indefinitely)
        bool SetAnimationNumLoops(const std::string& name, uint repeats);
        
        //! Returns all running animations
        const AnimationMap& GetRunningAnimations() const { return animations_; }
        
    private:
        //! Constructor
        /*! \param module renderer module
         */
        EC_OgreAnimationController(Foundation::ModuleInterface* module);
        
        //! Gets Ogre entity from the mesh entity component and checks if it has changed; in that case resets internal state
        Ogre::Entity* GetEntity();

        //! Gets animationstate from Ogre entity safely
        /*! \param entity Ogre entity
            \param name Animation name
            \return animationstate, or null if not found
         */
        Ogre::AnimationState* GetAnimationState(Ogre::Entity* entity, const std::string& name);
        
        //! Resets internal state
        void ResetState();
        
        //! Mesh entity component 
        EC_OgreMesh *mesh;
        
        //! Current mesh name
        std::string mesh_name_;
        
        //! Current animations
        AnimationMap animations_;
        
    	//! Bone blend mask of high-priority animations
    	Ogre::AnimationState::BoneBlendMask highpriority_mask_;

    	//! Bone blend mask of low-priority animations
    	Ogre::AnimationState::BoneBlendMask lowpriority_mask_;        
    };
}

#endif
