/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_SoundListener.h
 *  @brief  Only one entity be ative sound listener at a time.
 */

#ifndef incl_EC_SoundListener_EC_SoundListener_h
#define incl_EC_SoundListener_EC_SoundListener_h

#include "ComponentInterface.h"
#include "AttributeInterface.h"
#include "Declare_EC.h"

/**
 *
 */
class EC_SoundListener : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_SoundListener);
    Q_OBJECT

public:
    /// Destructor. Detaches placeable component from this entity.
    ~EC_SoundListener();

    /// Is this listener active.
    Foundation::Attribute<bool> active;

    /// Returns placeble component.
    Foundation::ComponentPtr GetPlaceable() const { return placeable_; }

private slots:
    /// Retrieves placeable component when parent entity is set.
    void RetrievePlaceable();

    /// When active attribute is changed and it's true, iterates the scene and
    /// disables all the other sound listeners.
    void DisableOtherSoundListeners();

private:
    /** Constructor.
     *  @param module Declaring module.
     */
    explicit EC_SoundListener(Foundation::ModuleInterface *module);

    /// Parent entity's placeable component.
    Foundation::ComponentPtr placeable_;
};

#endif
