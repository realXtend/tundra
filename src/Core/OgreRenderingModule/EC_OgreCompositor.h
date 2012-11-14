// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "CoreDefines.h"
#include "OgreModuleApi.h"

class OgreCompositionHandler;

/// Ogre compositor component
/** <table class="header">
    <tr>
    <td>
    <h2>OgreCompositor</h2>
    Can be used to enable Ogre post-processing effects.
    Registered by OgreRenderer::OgreRenderingModule.

    <b>Attributes</b>:
    <ul>
    <li>QString: compositorName
    <div> @copydoc compositorName </div>
    <li>bool: enabled
    <div> @copydoc enabled </div>
    <li>int: priority
    <div> @copydoc priority</div>
    <li>QVariantList: parameters
    <div> @copydoc parameters</div>
    </ul>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li>"AvailableCompositors": @copydoc AvailableCompositors
    <li>"ApplicableParameters": @copydoc ApplicableParameters
    </ul>

    <b>Reacts on the following actions:</b>
    <ul>
    </ul>
    </td>
    </tr>

    Does not emit any actions.

    <b>Doesn't depend on any components</b>.
    </table> */
class OGRE_MODULE_API EC_OgreCompositor : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_OgreCompositor", 18)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_OgreCompositor(Scene* scene);
    virtual ~EC_OgreCompositor();

    /// Enables or disables this compositor effect.
    /** Useful for when you don't want to recreate and delete the component just to enable/disable an effect. */
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

    /// Name of the compositor (Ogre resource name), f.ex. "HDR"
    Q_PROPERTY(QString compositorName READ getcompositorName WRITE setcompositorName);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, compositorName);

    /// Priority for the compositor.
    /** Lower values mean the compositor is rendered earlier. Use -1 to auto order. If there are more
        than one compositor in the scene with the same priority, the order of the compositors is arbitrary. */
    Q_PROPERTY(int priority READ getpriority WRITE setpriority);
    DEFINE_QPROPERTY_ATTRIBUTE(int, priority);

    /// Key-value pair for shader parameters, separated with a '='.
    /** The value supports up to 4 floats (Vector4) separated by spaces. F.ex. 'strength=1.2' or 'color=1 0 0 0.5' */
    Q_PROPERTY(QVariantList parameters READ getparameters WRITE setparameters);
    DEFINE_QPROPERTY_ATTRIBUTE(QVariantList, parameters);

public slots:
    /// Returns list of names for available compositors.
    QStringList AvailableCompositors() const;

    /// Returns applicable compositor parameter names and their current values in format "name=value" for the currently active compositor.
    QStringList ApplicableParameters() const;

private slots:
    void OneTimeRefresh();

private:
    void AttributesChanged();
    /// Enables or disables and sets the priority of the specified compositor based on the attributes
    void UpdateCompositor(const QString &compositor);

    /// Updates compositor shader parameters
    void UpdateCompositorParams(const QString &compositor);

    OgreCompositionHandler *compositionHandler; ///< Compositor handler used to actually add and remove post-process effects.
    QString previousRef; ///< Stored compositor ref for internal use
    int previousPriority; ///< Stored previous priority for internal use
};
