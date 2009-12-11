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
	    std::vector<Core::sound_id_t>::iterator i = sounds_.begin();
	    while (i != sounds_.end())
	    {
	        if (soundsystem->GetSoundState(*i) == Foundation::SoundServiceInterface::Stopped)
	            i = sounds_.erase(i);
	        else
	            ++i;
	    }
	}
	
	void EC_AttachedSound::SetPosition(Core::Vector3df position)
	{
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
        if (!soundsystem)
            return;
            
        for (Core::uint i = 0; i < sounds_.size(); ++i)
            soundsystem->SetPosition(sounds_[i], position);
	}
	
	void EC_AttachedSound::RemoveSound(Core::sound_id_t sound)
    {
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();


        std::vector<Core::sound_id_t>::iterator i = sounds_.begin();
        while (i != sounds_.end())
        {
            if ((*i) == sound)
            {
                if (soundsystem)
                    soundsystem->StopSound((*i));
                sounds_.erase(i);
                break;
            }
            
            ++i;           
        }
    }
    
    void EC_AttachedSound::RemoveAllSounds()
    {
        boost::shared_ptr<Foundation::SoundServiceInterface> soundsystem = framework_->GetServiceManager()->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();
 
        if (soundsystem)
        {    
            for (Core::uint i = 0; i < sounds_.size(); ++i)      
                soundsystem->StopSound(sounds_[i]);
        }        
        sounds_.clear();        
    }
    
    void EC_AttachedSound::AddSound(Core::sound_id_t sound)
    {
        if (sound == 0)
            return;
            
        sounds_.push_back(sound);
    }
}