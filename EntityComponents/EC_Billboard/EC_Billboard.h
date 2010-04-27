/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Billboard.h
 *  @brief  EC_Billboard shows a billboard (3D sprite) that is attached to an entity.
 *  @note   The entity must have EC_OgrePlaceable component available in advance.
 */

#ifndef incl_EC_Billboard_EC_BillBoard_h
#define incl_EC_Billboard_EC_BillBoard_h

#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "Vector3D.h"

namespace Ogre
{
    class BillboardSet;
    class Billboard;
}

class EC_Billboard : public Foundation::ComponentInterface
{
    Q_OBJECT

    DECLARE_EC(EC_Billboard);

private:
    /// Constuctor.
    /// @param module Module.
    explicit EC_Billboard(Foundation::ModuleInterface *module);

public:
    /// Destructor.
    ~EC_Billboard();

    /// Sets postion for the billboard.
    /// @param position Position.
    /// @note The position is relative to the entity to which the billboard is attached.
    /// @note Call this after you have first called Show()
    void SetPosition(const Vector3df& position);

    /// Sets the width and height for billboard.
    /// @param w Width.
    /// @param h height.
    /// @note Call this after you have first called Show()
    void SetDimensions(float w, float h);

    /// Returns true if the billboard is created properly.
    bool IsCreated() const { return billboardSet_ != 0 && billboard_ != 0; }

public slots:
    /// Creates and shows billboard.
    /// @param imageName Name of the image file. Note the image must be located in the OGRE resource group.
    /// @param timeToShow How long this billboard is visible in milliseconds.
    /// Use -1 (default)if you want the billboard be visible always.
    void Show(const std::string &imageName, int timeToShow = -1);

    /// Shows the billboard if it exists with the texture.
    /// @param timeToShow How long this billboard is visible in milliseconds.
    /// Use -1 (default)if you want the billboard be visible always.
    void Show(int timeToShow = -1);

    /// Hides the billboard.
    void Hide();

private:
    /// Creates Ogre texture resource for image.
    /// @return True if creation was succesful, false otherwise.
    bool CreateOgreTextureResource(const std::string &imageName);

    /// Name of the material used for the billboard set.
    std::string materialName_;

    /// Ogre billboard set.
    Ogre::BillboardSet *billboardSet_;

    /// Ogre billboard.
    Ogre::Billboard *billboard_;
};

#endif
