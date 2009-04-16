// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "Mapper.h"
#include "InputEvents.h"

namespace Input
{
    Mapper::Mapper(InputModuleOIS *module) : 
        module_(module)
    {
        assert (module_);

        SetDefaultMappings();
    }

    Mapper::~Mapper()
    {
    }

    void Mapper::SetDefaultMappings()
    {
        module_->AddUnbufferedKeyEvent(OIS::KC_W, Events::MOVE_FORWARD_PRESSED,     Events::MOVE_FORWARD_RELEASED);
        module_->AddUnbufferedKeyEvent(OIS::KC_S, Events::MOVE_BACK_PRESSED,        Events::MOVE_BACK_RELEASED);
        module_->AddUnbufferedKeyEvent(OIS::KC_A, Events::MOVE_LEFT_PRESSED,        Events::MOVE_LEFT_RELEASED);
        module_->AddUnbufferedKeyEvent(OIS::KC_D, Events::MOVE_RIGHT_PRESSED,       Events::MOVE_RIGHT_RELEASED);
        module_->AddUnbufferedKeyEvent(OIS::KC_UP, Events::MOVE_FORWARD_PRESSED,    Events::MOVE_FORWARD_RELEASED);
        module_->AddUnbufferedKeyEvent(OIS::KC_DOWN, Events::MOVE_BACK_PRESSED,     Events::MOVE_BACK_RELEASED);
        module_->AddUnbufferedKeyEvent(OIS::KC_LEFT, Events::MOVE_LEFT_PRESSED,     Events::MOVE_LEFT_RELEASED);
        module_->AddUnbufferedKeyEvent(OIS::KC_RIGHT, Events::MOVE_RIGHT_PRESSED,   Events::MOVE_RIGHT_RELEASED);

    }
}

