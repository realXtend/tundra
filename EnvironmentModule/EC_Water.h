// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Water_h
#define incl_EC_Water_h

#include "ComponentInterface.h"
#include "RexUUID.h"
#include "Declare_EC.h"

namespace Ogre
{
    class SceneNode;
    class Entity;
}


namespace Environment
{
	/**

<table class="header">
<tr>
<td>
<h2>Water</h2>
Represents data related to water plane.

Registered by Environment::EnvironmentModule.

<b>No Attributes</b>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Doesn't Depend on any components</b>. 
</table>

*/

    /// Represents data related to water plane.
    /// \ingroup EnvironmentModuleClient.
    class EC_Water : public Foundation::ComponentInterface
    {
        Q_OBJECT

        DECLARE_EC(EC_Water);
    public:
        virtual ~EC_Water();

        /// Sets the new water plane height.
        void SetWaterHeight(float height);

        /// @return The currently active water plane height if water does not exist returns 0.0
        float GetWaterHeight() const;

    private:
        EC_Water(Foundation::ModuleInterface* module);

        void CreateOgreWaterObject();

        //! Ogre scene node
        Ogre::SceneNode *scene_node_;

        //! Ogre entity.
        Ogre::Entity *entity_;
    };
}

#endif
