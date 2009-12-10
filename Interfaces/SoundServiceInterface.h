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
            \param orientation Orientation as quaternion
         */
        virtual void SetListener(const Core::Vector3df& position, const Core::Quaternion& orientation) = 0;       
        
        //! Plays non-positional sound
        /*! \param name Sound file name or asset id
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later)
         */           
        virtual Core::sound_id_t PlaySound(const std::string& name, bool local = false, Core::sound_id_t channel = 0) = 0;
        
        //! Plays positional sound. Returns sound id to adjust parameters
        /*! \param name Sound file name or asset id
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param position Position of sound
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later)            
         */     
        virtual Core::sound_id_t PlaySound3D(const std::string& name, bool local = false, Core::Vector3df position = Core::Vector3df(0.0f, 0.0f, 0.0f), Core::sound_id_t channel = 0) = 0;

        //! Gets state of channel
        /*! \param id Channel id
            \return Current state (stopped, loading sound asset, playing)
         */
        virtual Foundation::SoundServiceInterface::SoundState GetSoundState(Core::sound_id_t id) const = 0;
        
        //! Gets name of sound played/pending on channel
        /*! \param id Channel id
            \return Sound name, or empty if no sound
         */
        virtual const std::string& GetSoundName(Core::sound_id_t id) const = 0;
        
        //! Stops sound that's playing & destroys the channel
        /*! \param id Channel id
         */
        virtual void StopSound(Core::sound_id_t id) = 0;
        
        //! Adjusts pitch of channel
        /*! \param id Channel id
            \param pitch Pitch relative to sound's original pitch (1.0 = original)
         */
        virtual void SetPitch(Core::sound_id_t id, Core::Real pitch) = 0;
        
        //! Adjusts gain of channel
        /*! \param id Channel id
            \param gain New gain value, 1.0 = full volume, 0.0 = silence
         */        
        virtual void SetGain(Core::sound_id_t id, Core::Real gain) = 0;
        
        //! Adjusts looping status of channel
        /*! \param id Channel id
            \param looped Whether to loop
         */           
        virtual void SetLooped(Core::sound_id_t id, bool looped) = 0;
        
        //! Adjusts positional status of channel
        /*! \param id Channel id
            \param positional Positional status
         */               
        virtual void SetPositional(Core::sound_id_t id, bool positional) = 0;
        
        //! Sets position of channel
        /*! \param id Channel id
            \param position New position
         */   
        virtual void SetPosition(Core::sound_id_t id, Core::Vector3df position) = 0;
        
        //! Adjusts range parameters of positional sound channel.
        /*! \param id Channel id
            \param inner_radius Within inner radius, sound will be played at gain
            \param outer_radius Outside outer radius, sound will be silent
            \param rolloff Rolloff power factor. 1.0 = linear, 2.0 = distance squared 
            Between radiuses, attenuation will be interpolated and raised to power of rolloff
            If outer_radius is 0, there will be no attenuation (sound is always played at gain)
            Also, for non-positional channels the range parameters have no effect.
         */
        virtual void SetRange(Core::sound_id_t id, Core::Real inner_radius, Core::Real outer_radius, Core::Real rolloff) = 0;
    };
}

#endif

