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

class IEventData;

namespace OpenALAudio
{
    typedef std::map<sound_id_t, SoundChannelPtr> SoundChannelMap;
    typedef std::map<std::string, SoundPtr> SoundMap;
      
    //! Sound service implementation. Owned by OpenALAudioModule.
    class SoundSystem : public Foundation::SoundServiceInterface
    {
    public:
        //! Constructor. Initializes OpenAL audio using default device.
        SoundSystem(Foundation::Framework* framework);
        
        //! Destructor.
        virtual ~SoundSystem();
        
        //! (Re)initializes playback with specified device. Empty name uses default device.
        /*! \param name Playback device name
            \return true if successful
         */
        virtual bool Initialize(const std::string& name = std::string());
        
    public slots:
        //! Gets playback device names
        virtual QStringList GetPlaybackDevices();
        
        //! Sets listener position & orientation
        /*! \param position Position
            \param orientation Orientation as quaternion
         */
        virtual void SetListener(const Vector3df& position, const Quaternion& orientation);
        
        //! Sets master gain of whole sound system
        /*! \param master_gain New master gain, in range 0.0 - 1.0
         */
        virtual void SetMasterGain(float master_gain);
        
        //! Sets master gain of certain sound types
        /*! \param type Sound channel type to adjust
            \param master_gain New master gain, in range 0.0 - 1.0
         */
        virtual void SetSoundMasterGain(Foundation::SoundServiceInterface::SoundType type, float master_gain);
        
        //! Gets master gain of whole sound system
        virtual float GetMasterGain();
        
        //! Sets master gain of certain sound types
        virtual float GetSoundMasterGain(Foundation::SoundServiceInterface::SoundType type);        
        
        //! Plays non-positional sound
        /*! \param name Sound file name or asset id
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later)
         */
        virtual sound_id_t PlaySound(const QString& name, Foundation::SoundServiceInterface::SoundType type = Triggered, bool local = false, sound_id_t channel = 0);
        
        //! Plays positional sound. Returns sound id to adjust parameters
        /*! \param name Sound file name or asset id
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param position Position of sound
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later)            
         */
        virtual sound_id_t PlaySound3D(const QString& name, Foundation::SoundServiceInterface::SoundType type = Triggered, bool local = false, Vector3df position = Vector3df::ZERO, sound_id_t channel = 0);

        //! Buffers sound data into a non-positional channel
        /*! Note: use the returned channel id for continuing to feed the sound stream.
            Call StopSound() with channel id to free the channel, when done.
            \param buffer Sound buffer structure
            \param type Sound channel type, decides which mastervolume to use for the channel 
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful
         */
        virtual sound_id_t PlaySoundBuffer(const Foundation::SoundServiceInterface::SoundBuffer& buffer, Foundation::SoundServiceInterface::SoundType type = Triggered, sound_id_t channel = 0);
        
        //! Buffers sound data into a positional channel
        /*! Note: use the returned channel id for continuing to feed the sound stream.
            Call StopSound() with channel id to free the channel, when done.
            \param buffer Sound buffer structure
            \param type Sound channel type, decides which mastervolume to use for the channel 
            \param position Position of sound
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful
         */
        virtual sound_id_t PlaySoundBuffer3D(const Foundation::SoundServiceInterface::SoundBuffer& buffer, Foundation::SoundServiceInterface::SoundType type = Triggered, Vector3df position = Vector3df(0.0f, 0.0f, 0.0f), sound_id_t channel = 0);

        //! Gets state of channel
        /*! \param id Channel id
            \return Current state (stopped, pending & loading sound asset, playing)
         */
        virtual Foundation::SoundServiceInterface::SoundState GetSoundState(sound_id_t id) const;
        
        //! Gets all non-stopped channels id's
        virtual std::vector<sound_id_t> GetActiveSounds() const;
        
        //! Gets name of sound played/pending on channel
        /*! \param id Channel id
            \return Sound name, or empty if no sound
         */
        virtual const QString& GetSoundName(sound_id_t id) const;
     
        //! Gets type of sound played/pending on channel (triggered/ambient etc.)
        /*! \param id Channel id
            \return Sound type
         */
        virtual Foundation::SoundServiceInterface::SoundType GetSoundType(sound_id_t id) const;
        
        //! Stops sound that's playing & destroys the channel
        /*! \param id Channel id
         */
        virtual void StopSound(sound_id_t id);
        
        //! Adjusts pitch of channel
        /*! \param id Channel id
            \param pitch Pitch relative to sound's original pitch (1.0 = original)
         */
        virtual void SetPitch(sound_id_t id, float pitch);

        //! Get sound channel pitch.
        /*! \param id Channel id
         *  \return Channel's pitch value.
         */
        virtual float GetPitch(sound_id_t id) const;
        
        //! Adjusts gain of channel
        /*! \param id Channel id
            \param gain New gain value, 1.0 = full volume, 0.0 = silence
         */
        virtual void SetGain(sound_id_t id, float gain);
        
        //! Get gain of channel. If channel wasn't found return -1.
        /*! \param id Channel id
         *  \return Channel's gain.
         */
        virtual float GetGain(sound_id_t id) const; 
        
        //! Adjusts looping status of channel
        /*! \param id Channel id
            \param looped Whether to loop
         */
        virtual void SetLooped(sound_id_t id, bool looped);
        
        //! Adjusts positional status of channel
        /*! \param id Channel id
            \param positional Positional status
         */
        virtual void SetPositional(sound_id_t id, bool positional);
        
        //! Sets position of channel
        /*! \param id Channel id
            \param position New position
         */
        virtual void SetPosition(sound_id_t id, Vector3df position);
        
        //! Adjusts range parameters of positional sound channel.
        /*! \param id Channel id
            \param inner_radius Within inner radius, sound will be played at gain
            \param outer_radius Outside outer radius, sound will be silent
            \param rolloff Rolloff power factor. 1.0 = linear, 2.0 = distance squared 
            Between radiuses, attenuation will be interpolated and raised to power of rolloff
            If outer_radius is 0, there will be no attenuation (sound is always played at gain)
            Also, for non-positional channels the range parameters have no effect.
         */
        virtual void SetRange(sound_id_t id, float inner_radius, float outer_radius, float rolloff); 

    public:
        //! Get recording device names
        virtual QStringList GetRecordingDevices();
        
        //! Open sound recording device & start recording
        /*! \param name Device name, empty for default
            \param frequency Sound frequency
            \param sixteenbit Whether to use sixteen bit audio
            \param stereo Whether to use stereo
            \param buffer_size Buffer size in bytes. Should be multiple of sample size.
            \return true if successful
         */
        virtual bool StartRecording(const QString& name, uint frequency, bool sixteenbit, bool stereo, uint buffer_size);
        
        //! Stop recording & close sound recording device
        virtual void StopRecording();
        
        //! Get amount of sound currently in recording buffer, in bytes
        virtual uint GetRecordedSoundSize();
        
        //! Get sound data from recording buffer
        /*! \param buffer Buffer to receive data
            \param size How many bytes to receive
            \return Amount of bytes returned
         */
        virtual uint GetRecordedSoundData(void* buffer, uint size);

        //! Request decoded sound resource. Note: this is strictly for inspecting the sound data, not needed for playback
        /*! \param assetid sound asset id. Assumed to be ogg format
            \return Request tag; a matching RESOURCE_READY event with this tag will be sent once sound has been decoded
         */ 
        virtual request_tag_t RequestSoundResource(const QString& assetid);
        
        //! Update. Cleans up channels not playing anymore, and checks sound cache. Called from OpenALAudioModule.
        void Update(f64 frametime);
        
        //! Handles an asset event. Called from OpenALAudioModule.
        bool HandleAssetEvent(event_id_t event_id, IEventData* data);
        
        //! Handles a thread task event. Called from OpenALAudioModule.
        bool HandleTaskEvent(event_id_t event_id, IEventData* data);
        
        //! Returns initialized status
        bool IsInitialized() const { return initialized_; }

    private:
        //! Uninitialize OpenAL sound
        void Uninitialize();
        
        //! Return next sound channel ID
        sound_id_t GetNextSoundChannelID();
        
        //! Reapply master gain to all existing channels
        void ApplyMasterGain();
        
        //! Get sound
        /*! Creates new if necessary. Initiates asset decode/download as necessary.
         */
        SoundPtr GetSound(const std::string& name, bool local);
        //! Posts request for local decode of ogg file
        /* \return true if file could be found & decode initiated. This does not yet tell if the data is valid, though
         */
        bool DecodeLocalOggFile(Sound* sound, const std::string& name);
        
        //! Update sound cache. Ages sounds and removes oldest if cache too big
        void UpdateCache(f64 frametime);
        
        //! Framework
        Foundation::Framework* framework_;
        //! Initialized flag
        bool initialized_;
        //! OpenAL context
        ALCcontext* context_;
        //! OpenAL device
        ALCdevice* device_;
        //! OpenAL capture device
        ALCdevice* capture_device_;
        //! Capture sample size
        uint capture_sample_size_;
        //! Active channels
        SoundChannelMap channels_;
        //! Currently loaded sounds
        SoundMap sounds_;
        //! Sound cache size
        uint sound_cache_size_;
        //! Update timer (for cache)
        f64 update_time_;
        //! Next channel id
        sound_id_t next_channel_id_;
        
        //! Request tag map for sound resource requests
        std::map<request_tag_t, std::string> sound_resource_requests_;
        
        //! Listener position
        Vector3df listener_position_;
        //! Listener orientation
        Quaternion listener_orientation_;
        
        //! Master gain for whole sound system
        float master_gain_;
        //! Master gain for individual sound types
        std::map<Foundation::SoundServiceInterface::SoundType, float> sound_master_gain_;

        boost::mutex mutex;
    };

    typedef boost::shared_ptr<SoundSystem> SoundSystemPtr;
}

#endif
