// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenALAudio_SoundSystem_h
#define incl_OpenALAudio_SoundSystem_h

#include "SoundServiceInterface.h"
#include "Sound.h"
#include "SoundChannel.h"

#include <AL/al.h>
#include <AL/alc.h>

namespace Foundation
{
    class Framework;
}

namespace OpenALAudio
{
    typedef std::map<Core::sound_id_t, SoundChannelPtr> SoundChannelMap;
    typedef std::map<std::string, SoundPtr> SoundMap;
      
    class SoundSystem : public Foundation::SoundServiceInterface
    {
	public:
        SoundSystem(Foundation::Framework *framework);
		virtual ~SoundSystem();

        //! Sets listener position & orientation
        /*! \param position Position
            \param orientation Orientation as quaternion
         */
        virtual void SetListener(const Core::Vector3df& position, const Core::Quaternion& orientation);       
        
        //! Plays non-positional sound
        /*! \param name Sound file name or asset id
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later)
         */           
        virtual Core::sound_id_t PlaySound(const std::string& name, bool local = false, Core::sound_id_t channel = 0);
        
        //! Plays positional sound. Returns sound id to adjust parameters
        /*! \param name Sound file name or asset id
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param position Position of sound
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later)            
         */     
        virtual Core::sound_id_t PlaySound3D(const std::string& name, bool local = false, Core::Vector3df position = Core::Vector3df(0.0f, 0.0f, 0.0f), Core::sound_id_t channel = 0);

        //! Play raw audio data from buffer
        /*! \param buffer pointer to buffer where playable audio data is stored
            \buffer_size Size of buffer
            \sample_rate Sample rate of audio data
            \sample_width sample widh of audio data in bits. Currently only 8 and 16 are supported values
            \stereo If true then audio data is stero otherwise it's mono
            \channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
        */
        virtual Core::sound_id_t PlayAudioData(Core::u8* buffer, int buffer_size, int sample_rate, int sample_width, bool stereo, Core::sound_id_t channel = 0);

        //! Gets state of channel
        /*! \param id Channel id
            \return Current state (stopped, loading sound asset, playing)
         */
        virtual Foundation::SoundServiceInterface::SoundState GetSoundState(Core::sound_id_t id) const;
        
        //! Gets all non-stopped channels id's
        virtual std::vector<Core::sound_id_t> GetActiveSounds() const;
                
        //! Gets name of sound played/pending on channel
        /*! \param id Channel id
            \return Sound name, or empty if no sound
         */
        virtual const std::string& GetSoundName(Core::sound_id_t id) const;
        
        //! Stops sound that's playing & destroys the channel
        /*! \param id Channel id
         */
        virtual void StopSound(Core::sound_id_t id);
        
        //! Adjusts pitch of channel
        /*! \param id Channel id
            \param pitch Pitch relative to sound's original pitch (1.0 = original)
         */
        virtual void SetPitch(Core::sound_id_t id, Core::Real pitch);
        
        //! Adjusts gain of channel
        /*! \param id Channel id
            \param gain New gain value, 1.0 = full volume, 0.0 = silence
         */        
        virtual void SetGain(Core::sound_id_t id, Core::Real gain);
        
        //! Adjusts looping status of channel
        /*! \param id Channel id
            \param looped Whether to loop
         */           
        virtual void SetLooped(Core::sound_id_t id, bool looped);
        
        //! Adjusts positional status of channel
        /*! \param id Channel id
            \param positional Positional status
         */               
        virtual void SetPositional(Core::sound_id_t id, bool positional);
        
        //! Sets position of channel
        /*! \param id Channel id
            \param position New position
         */   
        virtual void SetPosition(Core::sound_id_t id, Core::Vector3df position);
        
        //! Adjusts range parameters of positional sound channel.
        /*! \param id Channel id
            \param inner_radius Within inner radius, sound will be played at gain
            \param outer_radius Outside outer radius, sound will be silent
            \param rolloff Rolloff power factor. 1.0 = linear, 2.0 = distance squared 
            Between radiuses, attenuation will be interpolated and raised to power of rolloff
            If outer_radius is 0, there will be no attenuation (sound is always played at gain)
            Also, for non-positional channels the range parameters have no effect.
         */
        virtual void SetRange(Core::sound_id_t id, Core::Real inner_radius, Core::Real outer_radius, Core::Real rolloff); 

        //! Update. Cleans up channels not playing anymore, and checks sound cache. Called from OpenALAudioModule.
        void Update(Core::f64 frametime);
        
        //! Handles an asset event. Called from OpenALAudioModule.
        bool HandleAssetEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);
        
        //! Handles a thread task event. Called from OpenALAudioModule.
        bool HandleTaskEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);
                
        //! Returns initialized status
        bool IsInitialized() const { return initialized_; }

    private:
        //! Initialize OpenAL sound
		void Initialize();
        //! Uninitialize OpenAL sound
        void Uninitialize();
        //! Return next sound channel ID
        Core::sound_id_t GetNextSoundChannelID();  
        //! Get sound
        /*! Creates new if necessary. Initiates asset decode/download as necessary.
         */ 
        SoundPtr GetSound(const std::string& name, bool local);
        //! Posts request for local decode of ogg file
        /* \return true if file could be found & decode initiated. This does not yet tell if the data is valid, though
         */
        bool DecodeLocalOggFile(Sound* sound, const std::string& name);
        
        //! Update sound cache. Ages sounds and removes oldest if cache too big
        void UpdateCache(Core::f64 frametime);
        
        //! Framework
        Foundation::Framework* framework_;
        //! Initialized flag
        bool initialized_;        
        //! OpenAL context
        ALCcontext* context_;        
        //! OpenAL device
        ALCdevice* device_;        
        //! Active channels
        SoundChannelMap channels_;        
        //! Currently loaded sounds
        SoundMap sounds_;      
        //! Sound cache size
        Core::uint sound_cache_size_;  
        //! Update timer (for cache)
        Core::f64 update_time_;
        //! Next channel id
        Core::sound_id_t next_channel_id_;
        
        //! Listener position
        Core::Vector3df listener_position_;
        //! Listener orientation
        Core::Quaternion listener_orientation_;

        boost::mutex mutex;
    };

    typedef boost::shared_ptr<SoundSystem> SoundSystemPtr;
}

#endif
