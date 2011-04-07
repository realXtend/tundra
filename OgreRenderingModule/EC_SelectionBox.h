// For conditions of distribution and use, see copyright notice in license.txt
// Author: Nathan Letwory <nathan@letworyinteractive.com>

#ifndef incl_OgreRenderer_EC_SelectionBox_h
#define incl_OgreRenderer_EC_SelectionBox_h

#include "IComponent.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "Declare_EC.h"

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
    
    DECLARE_EC(EC_SelectionBox);
public:
    
    /// Destructor.
    virtual ~EC_SelectionBox();

public slots:
    /// Set the selection box dimensions.
    /// \param view screen coordinates
    void SetBoundingBox(QRect &view);

    /// Show the selection box, clearing.
    void Show();
    /// Hide the selection box, clearing.
    void Hide();

    
private:
    /// constructor
    /// \param module renderer module
    EC_SelectionBox(IModule* module);

    /// The object to draw selection box with
    Ogre::ManualObject *selectionBox_;
    
    /// renderer
    OgreRenderer::RendererWeakPtr renderer_;
    
};

#endif
