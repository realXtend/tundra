/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ECBillboard.h
 *  @brief  EC_Billboard shows a billboard (3D sprite) that is attached to an entity.
 *  @note   The entity must have EC_OgrePlaceable component available in advance.
*/

#ifndef incl_EC_Billboard_EC_BillBoard_h
#define incl_EC_Billboard_EC_BillBoard_h

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

class EC_Billboard : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_Billboard);

private:
    /// Constuctor.
    /// @param module Module.
    explicit EC_Billboard(Foundation::ModuleInterface *module);

public:
    /// Destructor.
    ~EC_Billboard();

    /// @param offset
    /// @param timeToShow How long this billboard is visible. Note: not used yet.
    /// @param imageName Name of the image file. Note the image must be located in the OGRE resource group.
    void Show(const Vector3df &offset, float timeToShow, const char *imageName);
};

#endif
