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
<li>bool: enabled
<div>Enables or disables this compositor effect. Useful for when you don't want to recreate and delete the component just to enable / disable an effect.</div>
<li>int: priority
<div>Priority for the compositor. Lower values mean the compositor is rendered earlier. Use -1 to auto order. If there are more
than one compositor in the scene with the same priority, the order of the compositors is arbitrary.</div>
<li>QVariantList: parameters
<div>Key-value pair for shader parameters, separated with a '='. The value supports up to 4 floats (Vector4) separated by spaces.
F.ex. 'strength=1.2' or 'color=1 0 0 0.5'</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>AvailableCompositors
<div>Returns list of available compositor names.</div>
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
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

    Q_PROPERTY(QString compositorref READ getcompositorref WRITE setcompositorref);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, compositorref);

    Q_PROPERTY(int priority READ getpriority WRITE setpriority);
    DEFINE_QPROPERTY_ATTRIBUTE(int, priority);

    Q_PROPERTY(QVariantList parameters READ getparameters WRITE setparameters);
    DEFINE_QPROPERTY_ATTRIBUTE(QVariantList, parameters);

    virtual ~EC_OgreCompositor();

public slots:
    /// Returns list of available compositor names.
    QStringList AvailableCompositors() const;

private slots:
    void OnAttributeUpdated(IAttribute* attribute);

private:
    /// Enables or disables and sets the priority of the specified compositor based on the attributes
    void UpdateCompositor(const QString &compositor);

    /// Updates compositor shader parameters
    void UpdateCompositorParams(const QString &compositor);

    /// constructor
    /** \param module Ogre module
     */
    EC_OgreCompositor(IModule* module);

    /// Owner module of this component
    OgreRenderer::OgreRenderingModule *owner_;
    /// Compositor handler. Used to actually add / remove post process effects.
    OgreRenderer::CompositionHandler *handler_;
    /// Stored compositor ref for internal use
    QString previous_ref_;
};

#endif
