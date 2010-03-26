// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SoundServiceInterface.h"
#include "ModuleInterface.h"
#include "ServiceManager.h"
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
	
	void EC_AttachedSound::Update(f64 frametime)
	{
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem)
            return;
            	
	    // Check if any of the sounds have stopped, remove from list in that case
	    for (uint i = 0; i < sounds_.size(); ++i)
	    {
	        if (sounds_[i])
	        {
	            if (soundsystem->GetSoundState(sounds_[i]) == Foundation::SoundServiceInterface::Stopped)
	                sounds_[i] = 0;
	        }
	    }
	}
	
	void EC_AttachedSound::SetPosition(Vector3df position)
	{
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem)
            return;
            
        for (uint i = 0; i < sounds_.size(); ++i)
        {
            if (sounds_[i])
                soundsystem->SetPosition(sounds_[i], position);
        }
	}
	
	void EC_AttachedSound::RemoveSound(sound_id_t sound)
    {
        if (sound == 0)
            return;
                
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
      
        for (uint i = 0; i < sounds_.size(); ++i)
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

        uint i = (uint)slot;
        if (i < (uint)Other)
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
            for (uint i = 0; i < sounds_.size(); ++i)    
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
        sounds_.resize((uint)Other);    
        sounds_[OpenSimAttachedSound] = 0;
        sounds_[RexAmbientSound] = 0;
    }   
    
    void EC_AttachedSound::AddSound(sound_id_t sound, SoundSlot slot)
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
            for (uint i = (uint)Other; i < sounds_.size(); ++i)
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