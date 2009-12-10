// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenALAudio_EC_AttachedSound_h
#define incl_OpenALAudio_EC_AttachedSound_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace RexLogic
{
    //! Component which holds references to attached spatial sounds.
    /*! Their positions will be updated automatically to match the position of the entity
        To use, first setup the sound usually using the sound service, then use AddSound()
        To be useful, the entity also needs the EC_OgrePlaceable component.
     */
    class EC_AttachedSound : public Foundation::ComponentInterface
	{
		DECLARE_EC(EC_AttachedSound);
	public:
		virtual ~EC_AttachedSound();
		
		//! Add sound to entity. 
		/*! \param sound Sound tag from SoundServiceInterface
		 */
		void AddSound(Core::sound_id_t sound);
		//! Stop and remove sound from entity.
		/*! \param sound Sound tag from SoundServiceInterface
		 */
		void RemoveSound(Core::sound_id_t sound);
		
		//! Stop and remove all sounds from entity.
		/*! Also called automatically upon destruction of this EntityComponent.
		 */
		void RemoveAllSounds();

        //! Get sound tags of currently playing sounds.
		const std::vector<Core::sound_id_t>& GetSounds() const { return sounds_; }
		
		//! Performs per-frame update, if necessary. Called from RexLogicModule.
		void Update(Core::f64 frametime);	
			
		//! Syncs position. Called from RexLogicModule.
		void SetPosition(const Core::Vector3df position);				
		
	private:
		EC_AttachedSound(Foundation::ModuleInterface *module);
		
		Foundation::Framework* framework_;
		
		//! Vector of current attached sounds
		std::vector<Core::sound_id_t> sounds_;
	};
}

#endif
