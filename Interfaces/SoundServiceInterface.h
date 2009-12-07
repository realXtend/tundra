// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_SoundServiceInterface_h
#define incl_Interfaces_SoundServiceInterface_h

#include "ServiceInterface.h"

namespace Foundation
{
    //! An interface for sound functionality.
    class SoundServiceInterface : public ServiceInterface
    {
    public:
        enum SoundState
        {
            Stopped = 0,
            Loading,
            Playing
        };
       
        SoundServiceInterface() {}
        virtual ~SoundServiceInterface() {}

        //! Sets listener position & orientation
        /*! \param position Position
            \param orientation Orientation as quaternion.
         */
        virtual void SetListener(const Core::Vector3df& position, const Core::Quaternion& orientation) = 0;       
        
        //! Plays non-positional sound
        /*! \param name Sound file name or asset id
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero sound id, if successful (in case of loading from asset, actual sound may start later)
         */           
        virtual Core::sound_id_t PlaySound(const std::string& name, bool local, Core::sound_id_t channel = 0) = 0;
        
        //! Plays positional sound. Returns sound id to adjust parameters
        /*! \param name Sound file name or asset id
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param position Position of sound
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero sound id, if successful (in case of loading from asset, actual sound will start later)            
         */     
        virtual Core::sound_id_t PlaySound3D(const std::string& name, bool local, Core::Vector3df& position, Core::sound_id_t channel = 0) = 0;

        //! Gets state of channel
        virtual SoundState GetSoundState(Core::sound_id_t id) = 0;
        
        //! Stops sound that's playing & destroys the channel
        virtual void StopSound(Core::sound_id_t id) = 0;
        
        //! Adjusts pitch of sound
        virtual void SetPitch(Core::sound_id_t id, Core::Real pitch) = 0;
        
        //! Adjusts gain of sound
        virtual void SetGain(Core::sound_id_t id, Core::Real gain) = 0;
        
        //! Adjusts looping status of sound 
        virtual void SetLooped(Core::sound_id_t id, bool looped) = 0;
        
        //! Adjusts positional status of sound
        virtual void SetPositional(Core::sound_id_t id, bool positional) = 0;
        
        //! Adjusts position of sound
        virtual void SetPosition(Core::sound_id_t id, Core::Vector3df position) = 0;
        
        //! Adjusts range of sound
        virtual void SetRange(Core::sound_id_t id, Core::Real radius, Core::Real rolloff) = 0;
    };
}

#endif

