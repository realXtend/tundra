// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ConsoleManager.h"
#include "InputEventsOIS.h"

namespace Console
{
    bool ConsoleManager::HandleKeyDown(OIS::KeyCode code, unsigned int text)
    {
        return (checked_static_cast<OgreOverlay*>(ogre_.get())->HandleKeyDown(code, text));
    }
}
