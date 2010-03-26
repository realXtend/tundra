// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenALAudio_EC_AttachedSound_h
#define incl_OpenALAudio_EC_AttachedSound_h

#include "ComponentInterface.h"
#include "RexLogicModuleApi.h"
#include "Vector3D.h"
#include "Declare_EC.h"

namespace RexLogic
{
    //! Component which holds references to attached spatial sounds.
    /*! Their positions will be updated automatically to match the position of the entity
        To use, first setup the sound usually using the sound service, then use AddSound()
        To be useful, the entity also needs the EC_OgrePlaceable component.
     */
    class REXLOGIC_MODULE_API EC_AttachedSound : public Foundation::ComponentInterface
    {
        Q_OBJECT

        DECLARE_EC(EC_AttachedSound);
    public:
        //! Sound slot definitions. 
        /*! When a new sound is added to either OpenSimAttachedSound or RexAmbientSound slot, the previous will be killed.
            "Other" slot on the other hand allows as many sounds to be added as desired
         */
        enum SoundSlot
        {
            OpenSimAttachedSound = 0,
            RexAmbientSound,
            Other
        };

        virtual ~EC_AttachedSound();
        
        //! Add sound to entity. 
        /*! \param sound Channel id from SoundServiceInterface
            \param slot Sound slot definition
         */
        void AddSound(sound_id_t sound, SoundSlot slot = Other);

        //! Stop and remove sound from entity by channel id
        /*! \param sound Channel id from SoundServiceInterface
         */
        void RemoveSound(sound_id_t sound);

        //! Stop and remove sound from entity by sound slot
        /*! \param slot Sound slot definition, only OpenSimAttachedSound or RexAmbientSound will have effect
         */
        void RemoveSound(SoundSlot slot);

        //! Stop and remove all sounds from entity.
        /*! Also called automatically upon destruction of this EntityComponent.
         */
        void RemoveAllSounds();

        //! Get channel ids of currently playing sounds. Note that some may be zero (stopped)
        const std::vector<sound_id_t>& GetSounds() const { return sounds_; }

        //! Performs per-frame update, if necessary. Called from RexLogicModule.
        void Update(f64 frametime);

        //! Syncs position. Called from RexLogicModule.
        void SetPosition(const Vector3df position);

    private:
        EC_AttachedSound(Foundation::ModuleInterface *module);

        //! Init sound slot vector
        void InitSoundVector();

        Foundation::Framework* framework_;

        //! Vector of current attached sounds
        std::vector<sound_id_t> sounds_;
    };
}

#endif
