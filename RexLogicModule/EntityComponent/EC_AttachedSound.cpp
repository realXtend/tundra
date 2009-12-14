// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "SoundServiceInterface.h"
#include "EC_AttachedSound.h"

namespace RexLogic
{
    EC_AttachedSound::EC_AttachedSound(Foundation::ModuleInterface *module) : Foundation::ComponentInterface(module->GetFramework())
	{
        framework_ = module->GetFramework(); 
        
        InitSoundVector();  
	}

    EC_AttachedSound::~EC_AttachedSound()
	{
	    RemoveAllSounds();        
	}
	
	void EC_AttachedSound::Update(Core::f64 frametime)
	{
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem)
            return;
            	
	    // Check if any of the sounds have stopped, remove from list in that case
	    for (Core::uint i = 0; i < sounds_.size(); ++i)
	    {
	        if (sounds_[i])
	        {
	            if (soundsystem->GetSoundState(sounds_[i]) == Foundation::SoundServiceInterface::Stopped)
	                sounds_[i] = 0;
	        }
	    }
	}
	
	void EC_AttachedSound::SetPosition(Core::Vector3df position)
	{
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem)
            return;
            
        for (Core::uint i = 0; i < sounds_.size(); ++i)
        {
            if (sounds_[i])
                soundsystem->SetPosition(sounds_[i], position);
        }
	}
	
	void EC_AttachedSound::RemoveSound(Core::sound_id_t sound)
    {
        if (sound == 0)
            return;
                
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
      
        for (Core::uint i = 0; i < sounds_.size(); ++i)
        {
            if (sounds_[i] == sound)
            {
                if (soundsystem)
                    soundsystem->StopSound(sounds_[i]);
                sounds_[i] = 0;
            }
        }
    }
    
    void EC_AttachedSound::RemoveSound(SoundSlot slot)
    {
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();

        Core::uint i = (Core::uint)slot;
        if (i < (Core::uint)Other)
        {
            if (sounds_[i])
            {
                if (soundsystem)
                    soundsystem->StopSound(sounds_[i]);
                sounds_[i] = 0;
            }
        }            
    }
         
    void EC_AttachedSound::RemoveAllSounds()
    {
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
 
        if (soundsystem)
        {    
            for (Core::uint i = 0; i < sounds_.size(); ++i)    
            {
                if (sounds_[i])
                    soundsystem->StopSound(sounds_[i]);
            }
        }        
        
        InitSoundVector(); 
    }
    
    void EC_AttachedSound::InitSoundVector()
    {
        // Have always room for the opensim & rex-style ambient sounds 
        sounds_.clear();
        sounds_.resize((Core::uint)Other);    
        sounds_[OpenSimAttachedSound] = 0;
        sounds_[RexAmbientSound] = 0;
    }   
    
    void EC_AttachedSound::AddSound(Core::sound_id_t sound, SoundSlot slot)
    {
        if (sound == 0)
            return;
                        
        if (slot != Other)
        {
            // Remove previous sound from slot first
            RemoveSound(slot);
            sounds_[slot] = sound;
        }
        else        
        {
            for (Core::uint i = (Core::uint)Other; i < sounds_.size(); ++i)
            {
                if (sounds_[i] == 0)
                {
                    sounds_[i] = sound;
                    return;
                }
            }
            
            sounds_.push_back(sound);
        }
    }
}