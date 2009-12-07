// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenALAudio_SoundSystem_h
#define incl_OpenALAudio_SoundSystem_h

#include "SoundServiceInterface.h"
#include "Sound.h"
#include "SoundChannel.h"

#include <al.h>
#include <alc.h>

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
            \param orientation Orientation as quaternion.
         */
        void SetListener(const Core::Vector3df& position, const Core::Quaternion& orientation);       
        
        //! Plays non-positional sound
        /*! \param name Sound file name or asset id
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero sound id, if successful (in case of loading from asset, actual sound may start later)
         */           
        Core::sound_id_t PlaySound(const std::string& name, bool local, Core::sound_id_t channel = 0);
        
        //! Plays positional sound. Returns sound id to adjust parameters
        /*! \param name Sound file name or asset id
            \param local If true, name is interpreted as filename. Otherwise asset id
            \param position Position of sound
            \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
            \return nonzero sound id, if successful (in case of loading from asset, actual sound will start later)            
         */     
        Core::sound_id_t PlaySound3D(const std::string& name, bool localsemp, Core::Vector3df& position, Core::sound_id_t channel = 0);

        //! Gets state of channel
        Foundation::SoundServiceInterface::SoundState GetSoundState(Core::sound_id_t id);        
        //! Stops sound that's playing & destroys the channel
        void StopSound(Core::sound_id_t id);        
        //! Adjusts pitch of sound
        void SetPitch(Core::sound_id_t id, Core::Real pitch);
        //! Adjusts gain of sound
        void SetGain(Core::sound_id_t id, Core::Real gain);
        //! Adjusts looping status of sound 
        void SetLooped(Core::sound_id_t id, bool looped);        
        //! Adjusts positional status of sound
        void SetPositional(Core::sound_id_t id, bool positional);        
        //! Adjusts position of sound
        void SetPosition(Core::sound_id_t id, Core::Vector3df position);
        //! Adjusts range of sound
        void SetRange(Core::sound_id_t id, Core::Real radius, Core::Real rolloff);        

        //! Update. Cleans up channels not playing anymore, and checks sound cache. Called from OpenALAudioModule
        void Update(Core::f64 frametime);
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
        /*! Creates new if necessary. Initiates asset download as necessary.
         */ 
        SoundPtr GetSound(const std::string& name, bool local);
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
    };

    typedef boost::shared_ptr<SoundSystem> SoundSystemPtr;
}

#endif
