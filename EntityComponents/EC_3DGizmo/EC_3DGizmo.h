/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_3DGizmo.h
 *  @brief  EC_3DGizmo enables visual sound properties effect for scene entity.
 *  @note   The entity must have EC_Placeable and EC_OpenSimPrim 
 *  @author Nathan Letwory | http://www.letworyinteractive.com
 */

#ifndef incl_EC_3DGizmo_EC_3DGizmo_h
#define incl_EC_3DGizmo_EC_3DGizmo_h

#include "IComponent.h"
#include "Declare_EC.h"

namespace OgreRenderer
{
    class Renderer;
}

namespace Ogre
{
    class SceneNode;
    class ManualObject;
}



class EC_OpenSimPrim;

/**

<table class="header">
<tr>
<td>
<h2>3DGizmo</h2>
3DGizmo enables visual sound properties effect for scene entity.

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>
<li>float: radiusAttr_
<div>Sound radius</div> 
<li>float: volumeAttr_
<div>Sound volume</div> 
<li>float: segmentsAttr_
<div> Segments to use for radius circle</div> 
</ul>


<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Show": Shows the highlighting effect. 
<li>"Hide": Hides the highlighting effect.
<li>"SetVolume": Set volume to show
<li>"SetRadius": Set radius to show
<li>"IsVisible": Returns if the ruler component is visible or not.
		@true If the rule component is visible, false if it's hidden or not initialized properly.
<li>"Update3DGizmo": Callback for OnChanged from ECEditor
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on Placeable, OgreMesh, OgreCustomObject and OpenSimPrim</b>. @author Nathan Letwory | http://www.letworyinteractive.com
</table>

*/
class EC_3DGizmo : public IComponent
{


    Q_OBJECT
    DECLARE_EC(EC_3DGizmo);
    

public:
    /// Destructor.
    ~EC_3DGizmo();
    
public slots:
	void Update3DGizmo();
    
private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_3DGizmo(IModule *module);

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;
};

#endif
