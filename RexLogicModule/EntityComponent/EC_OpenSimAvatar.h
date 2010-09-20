// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_OpenSimAvatar_h
#define incl_EC_OpenSimAvatar_h

#include "ComponentInterface.h"
#include "RexUUID.h"
#include "RexLogicModuleApi.h"
#include "Declare_EC.h"

namespace RexLogic
{
	/**
<table class="header">
<tr>
<td>
<h2>OpenSimAvatar</h2>
This component is present on all avatars when connected to an OpenSim world. 

Registered by RexLogic::RexLogicModule.

<b>No Attributes</b>.

<b>Exposes the following scriptable functions:</b>
<ul>
<li>..
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Doesn't depend on any components</b>.
</table>
*/
    //! This component is present on all avatars when connected to an
    //! OpenSim world. 
    class REXLOGIC_MODULE_API EC_OpenSimAvatar : public Foundation::ComponentInterface
    {
        Q_OBJECT
            
        DECLARE_EC(EC_OpenSimAvatar);
       
    public:
        //! Avatar states
        //! \todo Do we need combinations of states?
        enum State
        {
            Stand,
            Walk,
            Fly,
            Hover,
            Sit
        };

        virtual ~EC_OpenSimAvatar();

        //! set appearance address 
        void SetAppearanceAddress(const std::string &address, bool overrideappearance);
        // get appearance address that is used, return override if that is defined, otherwise the default address
        const std::string& GetAppearanceAddress() const;

        //! current control flags for the avatar. If you modify this variable, you are responsible for sending update to server.
        uint32_t controlflags;

        //! cached control flags for the avatar, from previous update.
        uint32_t cached_controlflags;

        //! yaw the avatar (-1 for left, 1 for right)
		//NOTE: changed experimentally to float to support 'analog' rotating, via e.g. joystick?
		float yaw;

        //! set state
        void SetState(State state);
        //! get state
        State GetState() const;

    private:
        //! appearance address
        std::string avatar_address_;
        //! appearance override address
        std::string avatar_override_address_;
        //! state
        State state_;

        EC_OpenSimAvatar(Foundation::ModuleInterface* module);
    };
}

#endif
