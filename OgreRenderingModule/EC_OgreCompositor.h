// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreCompositor_h
#define incl_OgreRenderer_EC_OgreCompositor_h

#include "IComponent.h"
#include "Declare_EC.h"
#include "Core.h"


namespace OgreRenderer { class OgreRenderingModule; class CompositionHandler; };

/// Ogre compositor component
/**
<table class="header">
<tr>
<td>
<h2>OgreCompositor</h2>
Can be used to enable Ogre postprocessing effects.
Registered by OgreRenderer::OgreRenderingModule.

<b>Attributes</b>:
<ul>
<li>QString: compositorref
<div>Name of the compositor (Ogre resource name), f.ex. "HDR"</div>
<li>int: priority
<div>Priority for the compositor. Lower values mean the compositor is rendered earlier. Use -1 to auto order. If there are more
than one compositor in the scene with the same priority, the order of the compositors is arbitrary.</div>
</ul>

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

<b>Doesn't depend on any components</b>.
</table>
*/
class EC_OgreCompositor : public IComponent
{
    Q_OBJECT
    
    DECLARE_EC(EC_OgreCompositor);
public:
    Q_PROPERTY(QString compositorref READ getcompositorref WRITE setcompositorref);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, compositorref);

    Q_PROPERTY(int priority READ getpriority WRITE setpriority);
    DEFINE_QPROPERTY_ATTRIBUTE(int, priority);

    virtual ~EC_OgreCompositor();

private slots:
    void AttributeUpdated(IAttribute* attribute);

private:
    void UpdateCompositor();

    //! constructor
    /*! \param module Ogre module
     */
    EC_OgreCompositor(IModule* module);

    //! Owner module of this component
    OgreRenderer::OgreRenderingModule *owner_;
    //! Compositor handler. Used to actually add / remove post process effects.
    OgreRenderer::CompositionHandler *handler_;
    //! Stored compositor ref for internal use
    QString previous_ref_;
};

#endif
