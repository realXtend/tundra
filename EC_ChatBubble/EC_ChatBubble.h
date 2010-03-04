/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_ChatBubble.h
 *  @brief  EC_ChatBubble 
*/

#ifndef incl_EC_ChatBubble_ECBillBoard_h
#define incl_EC_ChatBubble_ECBillBoard_h

#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "Vector3D.h"

/*
namespace Scene
{
    class Entity;
}

namespace Ogre
{
    class SceneManager;
}
*/

class EC_ChatBubble : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_ChatBubble);

    /// Constuctor.
    /// @param module Module.
    explicit EC_ChatBubble(Foundation::ModuleInterface *module);

public:
    /// Destructor.
    ~EC_ChatBubble();

    /// @param offset
    /// @param timeToShow How long this billboard is visible. Note: not used yet.
    /// @param imageName Name of the image file. Note the image must be located in the OGRE resource group.
    void Show(const Vector3df &offset, float timeToShow, const char *imageName);
};

#endif
