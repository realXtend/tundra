// For conditions of distribution and use, see copyright notice in license.txt
// Author: Nathan Letwory <nathan@letworyinteractive.com>

#pragma once

#include "IComponent.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"

/// Ogre selection box component
/**
<table class="header">
<tr>
<td>
<h2>SelectionBox</h2>

Used to draw a selection box in the overlay layer, using an Ogre::ManualObject

Registered by OgreRenderer::OgreRenderingModule.

\ingroup OgreRenderingModuleClient

<b>No Attributes</b>.

<b>Exposes scriptable functions:</b>
<ul>
<li>Show</li>
<li>Hide</li>
<li>SetBoundingBox</li>

<b>Doesn't react on actions</b>

Does not emit any actions.

</table>
*/
class OGRE_MODULE_API EC_SelectionBox : public IComponent
{
    Q_OBJECT
    
public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_SelectionBox(Scene* scene);
    
    virtual ~EC_SelectionBox();

    COMPONENT_NAME("EC_SelectionBox", 22)
public slots:

    /// Set the selection box dimensions.
    /// \param view screen coordinates
    void SetBoundingBox(QRect &view);

    /// Show the selection box, clearing.
    void Show();
    /// Hide the selection box, clearing.
    void Hide();

    
private:
    /// The object to draw selection box with
    Ogre::ManualObject *selectionBox_;
    
    /// Ogre world ptr
    OgreWorldWeakPtr world_;
    
};

