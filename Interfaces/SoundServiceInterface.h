// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_SoundServiceInterface_h
#define incl_Interfaces_SoundServiceInterface_h

#include "ServiceInterface.h"
#include "CoreTypes.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "ResourceInterface.h"

#include <QString>

namespace Foundation
{
    //! An interface for sound functionality.
    class SoundServiceInterface : public QObject, public ServiceInterface
    {
        Q_OBJECT
        Q_ENUMS(SoundState)
        Q_ENUMS(SoundType)

    public:
        //! States of sound channels
        enum SoundState
        {
            Stopped = 0,
            Pending,
            Playing
        };
       
        //! Types of sound channels, for adjusting master volume individually
        enum SoundType
        {
            Triggered = 0,
            Ambient,
            Voice
        };
        
        //! Sound buffer description, used for streamed sound playback and in the sound resource class.
        /*! Note: the data is copied into internal structures, so it is not needed after a call returns.
         */
        struct SoundBuffer
        {
            //! Sound data
            std::vector<u8> data_;
            //! Frequency
            uint frequency_;
            //! Sixteenbit flag
            bool sixteenbit_;
            //! Stereo flag
            bool stereo_;
        };
        
        SoundServiceInterface() {}
        virtual ~SoundServiceInterface() {}

        //! (Re)initializes playback with specified device. Empty name uses default device.
        /*! \param name Playback device name
            \return true if successful
         */
        virtual bool Initialize(const std::string& name = std::string()) = 0;

    public slots:
        //! Gets playback device names
        virtual QStringList GetPlaybackDevices() = 0;
        
        //! Sets listener position & orientation
        /*! \param position Position
            \param orientation Orientation as quaternion
         */
        virtual void SetListener(const Vector3df& position, const Quaternion& orientation) = 0;
        
        //! Sets master gain of whole sound system
        /*! \param master_gain New master gain, in range 0.0 - 1.0
         */
        virtual void SetMasterGain(float master_gain) = 0;
        
        //! Sets master gain of certain sound types
        /*! \param type Sound channel type to adjust
            \param master_gain New master gain, in range 0.0 - 1.0
         */
        virtual void SetSoundMasterGain(SoundType type, float master_gain) = 0;
        
        //! Gets master gain of whole sound system
        virtual float GetMasterGain() = 0;
        
        //! Sets master gain of certain sound types
        virtual float GetSoundMasterGain(SoundType type) = 0;
                
        //! Plays non-positional sound
        /*! \param name Sound file name or asset id
            \param type Sound channel type, decides which mastervolume to use for the channel 
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later)
         */           
        virtual sound_id_t PlaySound(const QString& name, SoundType type = Triggered, bool local = false, sound_id_t channel = 0) = 0;
        
        //! Plays positional sound. Returns sound id to adjust parameters
        /*! \param name Sound file name or asset id
            \param type Sound channel type, decides which mastervolume to use for the channel 
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param position Position of sound
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later)
         */     
        virtual sound_id_t PlaySound3D(const QString& name, SoundType type = Triggered, bool local = false, Vector3df position = Vector3df(0.0f, 0.0f, 0.0f), sound_id_t channel = 0) = 0;

        //! Buffers sound data into a non-positional channel
        /*! Note: use the returned channel id for continuing to feed the sound stream.
            Call StopSound() with channel id to free the channel, when done.
            \param buffer Sound buffer structure
            \param type Sound channel type, decides which mastervolume to use for the channel 
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful
         */
        virtual sound_id_t PlaySoundBuffer(const SoundBuffer& buffer, SoundType type = Triggered, sound_id_t channel = 0) = 0;
        
        //! Buffers sound data into a positional channel
        /*! Note: use the returned channel id for continuing to feed the sound stream.
            Call StopSound() with channel id to free the channel, when done.
            \param buffer Sound buffer structure
            \param type Sound channel type, decides which mastervolume to use for the channel 
            \param position Position of sound
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero channel id, if successful
         */     
        virtual sound_id_t PlaySoundBuffer3D(const SoundBuffer& buffer, SoundType type = Triggered, Vector3df position = Vector3df(0.0f, 0.0f, 0.0f), sound_id_t channel = 0) = 0;

        //! Gets state of channel
        /*! \param id Channel id
            \return Current state (stopped, pending & loading sound asset, playing)
         */
        virtual Foundation::SoundServiceInterface::SoundState GetSoundState(sound_id_t id) const = 0;
        
        //! Gets all non-stopped channels id's
        virtual std::vector<sound_id_t> GetActiveSounds() const = 0;
        
        //! Gets type of sound played/pending on channel (triggered/ambient etc.)
        /*! \param id Channel id
            \return Sound type
         */
        virtual Foundation::SoundServiceInterface::SoundType GetSoundType(sound_id_t id) const = 0;

        //! Gets name of sound played/pending on channel
        /*! \param id Channel id
            \return Sound name, or empty if no sound
         */
        virtual const QString& GetSoundName(sound_id_t id) const = 0;
        
        //! Stops sound that's playing & destroys the channel
        /*! \param id Channel id
         */
        virtual void StopSound(sound_id_t id) = 0;
        
        //! Adjusts pitch of channel
        /*! \param id Channel id
            \param pitch Pitch relative to sound's original pitch (1.0 = original)
         */
        virtual void SetPitch(sound_id_t id, float pitch) = 0;

        //! Get sound channel pitch.
        /*! \param id Channel id
         *  \return Channel's pitch value.
         */
        virtual float GetPitch(sound_id_t id) const = 0;
        
        //! Adjusts gain of channel
        /*! \param id Channel id
            \param gain New gain value, 1.0 = full volume, 0.0 = silence
         */
        virtual void SetGain(sound_id_t id, float gain) = 0;

        //! Get gain of channel
        /*! \param id Channel id
         *  \return Channel's gain.
         */
        virtual float GetGain(sound_id_t id) const = 0;
        
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
        virtual void SetRange(sound_id_t id, float inner_radius, float outer_radius, float rolloff) = 0;

    public:

        //! Get recording device names
        virtual QStringList GetRecordingDevices() = 0;
        
        //! Open sound recording device & start recording
        /*! \param name Device name, empty for default
            \param frequency Sound frequency
            \param sixteenbit Whether to use sixteen bit audio
            \param stereo Whether to use stereo
            \param buffer_size Buffer size in bytes. Should be multiple of sample size.
            \return true if successful
         */
        virtual bool StartRecording(const QString& name, uint frequency, bool sixteenbit, bool stereo, uint buffer_size) = 0;
        
        //! Stop recording & close sound recording device
        virtual void StopRecording() = 0;
        
        //! Get amount of sound currently in recording buffer, in bytes
        virtual uint GetRecordedSoundSize() = 0;
        
        //! Get sound data from recording buffer
        /*! \param buffer Buffer to receive data
            \param size How many bytes to receive
            \return Amount of bytes returned
         */
        virtual uint GetRecordedSoundData(void* buffer, uint size) = 0;
        
        //! Request decoded sound resource. Note: this is strictly for inspecting the sound data, not needed for playback
        /*! \param assetid sound asset id. Assumed to be ogg format
            \return Request tag; a matching RESOURCE_READY event with this tag will be sent once sound has been decoded
         */ 
        virtual request_tag_t RequestSoundResource(const QString& assetid) = 0;
    };

    //! A sound resource
    class SoundResource : public ResourceInterface
    {
    public:
        SoundResource(const std::string& id, const SoundServiceInterface::SoundBuffer& buffer) :
            ResourceInterface(id),
            buffer_(buffer)
        {
        }
        
        virtual ~SoundResource()
        {
        }
        
        virtual bool IsValid() const { return buffer_.data_.size() != 0; }
        
        virtual const std::string& GetType() const
        { 
            static const std::string type_name("Sound");
            return type_name;
        }
        
        const SoundServiceInterface::SoundBuffer& GetBuffer() { return buffer_; }
        
    private:
        SoundServiceInterface::SoundBuffer buffer_;
    };
}

#endif

