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
        virtual void SetListener(const Vector3df& position, const Quaternion& orientation) = 0;       
        
        //! Plays non-positional sound
        /*! \param name Sound file name or asset id
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later)
         */           
        virtual sound_id_t PlaySound(const std::string& name, bool local = false, sound_id_t channel = 0) = 0;
        
        //! Plays positional sound. Returns sound id to adjust parameters
        /*! \param name Sound file name or asset id
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param position Position of sound
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later)            
         */     
        virtual sound_id_t PlaySound3D(const std::string& name, bool local = false, Vector3df position = Vector3df(0.0f, 0.0f, 0.0f), sound_id_t channel = 0) = 0;

        //! Play raw audio data from buffer
        /*! \param buffer pointer to buffer where playable audio data is stored
            \buffer_size Size of buffer
            \sample_rate Sample rate of audio data
            \sample_width sample widh of audio data in bits. Currently only 8 and 16 are supported values
            \stereo If true then audio data is stero otherwise it's mono
            \channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new

            \TODO: Support multiple SoundStream with sound stream id
        */
        virtual sound_id_t PlayAudioData(u8* buffer, int buffer_size, int sample_rate, int sample_width, bool stereo, sound_id_t channel = 0) = 0;

        //! Gets state of channel
        /*! \param id Channel id
            \return Current state (stopped, loading sound asset, playing)
         */
        virtual Foundation::SoundServiceInterface::SoundState GetSoundState(sound_id_t id) const = 0;
        
        //! Gets all non-stopped channels id's
        virtual std::vector<sound_id_t> GetActiveSounds() const = 0;
        
        //! Gets name of sound played/pending on channel
        /*! \param id Channel id
            \return Sound name, or empty if no sound
         */
        virtual const std::string& GetSoundName(sound_id_t id) const = 0;
        
        //! Stops sound that's playing & destroys the channel
        /*! \param id Channel id
         */
        virtual void StopSound(sound_id_t id) = 0;
        
        //! Adjusts pitch of channel
        /*! \param id Channel id
            \param pitch Pitch relative to sound's original pitch (1.0 = original)
         */
        virtual void SetPitch(sound_id_t id, Real pitch) = 0;
        
        //! Adjusts gain of channel
        /*! \param id Channel id
            \param gain New gain value, 1.0 = full volume, 0.0 = silence
         */        
        virtual void SetGain(sound_id_t id, Real gain) = 0;
        
        //! Adjusts looping status of channel
        /*! \param id Channel id
            \param looped Whether to loop
         */           
        virtual void SetLooped(sound_id_t id, bool looped) = 0;
        
        //! Adjusts positional status of channel
        /*! \param id Channel id
            \param positional Positional status
         */               
        virtual void SetPositional(sound_id_t id, bool positional) = 0;
        
        //! Sets position of channel
        /*! \param id Channel id
            \param position New position
         */   
        virtual void SetPosition(sound_id_t id, Vector3df position) = 0;
        
        //! Adjusts range parameters of positional sound channel.
        /*! \param id Channel id
            \param inner_radius Within inner radius, sound will be played at gain
            \param outer_radius Outside outer radius, sound will be silent
            \param rolloff Rolloff power factor. 1.0 = linear, 2.0 = distance squared 
            Between radiuses, attenuation will be interpolated and raised to power of rolloff
            If outer_radius is 0, there will be no attenuation (sound is always played at gain)
            Also, for non-positional channels the range parameters have no effect.
         */
        virtual void SetRange(sound_id_t id, Real inner_radius, Real outer_radius, Real rolloff) = 0;

        //! Sets position of channel
        /*! 
            \param position New position

            \todo: Merge with exist SetPosition method
         */   
        virtual void SetSoundStreamPosition(Vector3df position) = 0;

    };
}

#endif

