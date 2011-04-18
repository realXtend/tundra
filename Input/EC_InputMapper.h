/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InputMapper.h
 *  @brief  Registers an InputContext from the Naali Input subsystem and uses it to translate
 *          given set of key and mouse sequences to Entity Actions on the entity the component is part of.
 */

#ifndef incl_EC_InputMapper_EC_InputMapper_h
#define incl_EC_InputMapper_EC_InputMapper_h

#include "IComponent.h"
#include "Declare_EC.h"
#include "InputFwd.h"
#include "KeyEvent.h"

#include <QMap>
#include <QKeySequence>
#include <QVector>
#include <QVariant>

/// Registers an InputContext from the Naali Input subsystem and uses it to translate
/// given set of key and mouse sequences to Entity Actions on the entity the component is part of.
/**
<table class="header">
<tr>
<td>
<h2>InputMapper</h2>

Registers an InputContext from the Naali Input subsystem and uses it to translate
given set of key and mouse sequences to Entity Actions on the entity the component is part of.

<b>Attributes</b>:
<ul>
<li>QString: contextName.
<div>This input mapper's input context priority.</div> 
<li>int: contextPriority.
<div>This input mapper's input context priority.</div> 
<li>bool: takeKeyboardEventsOverQt.
<div>Does the mapper receive keyboard input events even when a Qt widget has focus.</div> 
<li>bool: takeMouseEventsOverQt.
<div>Does the mapper receive mouse input events even when a Qt widget has focus.</div>
<li>int: executionType.
<div>Action execution type that is used for the Entity Actions.</div>
<li>bool: modifiersEnabled.
<div>Whether modifiers are checked for in the key events. Default true.</div>
<li>bool: enabled.
<div>Whether the input mapper is active (ie. actions will be invoked.) Default true.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"RegisterMapping": Register new key sequence - action mapping for this input mapper.
        @param keySeq Key sequence.
        @param action Name of the action. If you want to use parameters the string should look the following: 
        "More(Forward)" or "Move(Forward,100)" etc.
        @note If registering key sequence with modifier keys, don't use Qt::Key enum - use Qt::Modifer enum instead.
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>

</td>
</tr>

<b>Can emit anykind of user-defined/registered actions.</b>

<b>Doesn't depend on any components</b>

</table>
*/
class EC_InputMapper : public IComponent
{
    DECLARE_EC(EC_InputMapper);
    Q_OBJECT

public:
    /// Destructor.
    ~EC_InputMapper();

    /// This input mapper's input context priority.
    Q_PROPERTY(QString contextName READ getcontextName WRITE setcontextName);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, contextName);

    /// This input mapper's input context priority.
    Q_PROPERTY(int contextPriority READ getcontextPriority WRITE setcontextPriority);
    DEFINE_QPROPERTY_ATTRIBUTE(int, contextPriority);

    /// Does the mapper receive keyboard input events even when a Qt widget has focus.
    Q_PROPERTY(bool takeKeyboardEventsOverQt READ gettakeKeyboardEventsOverQt WRITE settakeKeyboardEventsOverQt);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, takeKeyboardEventsOverQt);

    /// Does the mapper receive mouse input events even when a Qt widget has focus.
    Q_PROPERTY(bool takeMouseEventsOverQt READ gettakeMouseEventsOverQt WRITE settakeMouseEventsOverQt);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, takeMouseEventsOverQt);

    /// Key sequence - action name mappings.
    Q_PROPERTY(QVariantList mappings READ getmappings WRITE setmappings);
    DEFINE_QPROPERTY_ATTRIBUTE(QVariantList, mappings);
    
    /// Execution type for actions
    Q_PROPERTY(int executionType READ getexecutionType WRITE setexecutionType)
    DEFINE_QPROPERTY_ATTRIBUTE(int, executionType);
    
    /// Modifier mode for key events. Default true. If false, modifiers are not checked for key events
    Q_PROPERTY(bool modifiersEnabled READ getmodifiersEnabled WRITE setmodifiersEnabled)
    DEFINE_QPROPERTY_ATTRIBUTE(int, modifiersEnabled);
    
    /// Is the input mapper enabled
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);
    
    /// Trigger on keyrepeats, if it is off input mapper does not repeat keypressed actions. Default is on.
    Q_PROPERTY(bool keyrepeatTrigger READ getkeyrepeatTrigger WRITE setkeyrepeatTrigger)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, keyrepeatTrigger);

    //Attribute<QVariantList > mappings;

    struct ActionInvocation
    {
        ActionInvocation() : executionType(0) {}
        QString name;
        int executionType;
    };
    
    typedef QMap<QPair<QKeySequence, KeyEvent::EventType>, ActionInvocation> Mappings_t;

public slots:
    /// Register new key sequence - action mapping for this input mapper.
    /** @param keySeq Key sequence.
        @param action Name of the action. If you want to use parameters the string should look the following: 
        "More(Forward)" or "Move(Forward,100)" etc.
        @param eventType Event type (press, release), default press (1)
        @param executionType Execution type override. If 0 (default), uses the InputMapper's executionType attribute
        @note If registering key sequence with modifier keys, don't use Qt::Key enum - use Qt::Modifer enum instead.
    */
    void RegisterMapping(const QKeySequence &keySeq, const QString &action, int eventType = 1, int executionType = 0);

    /** Register new key sequence - action mapping for this input mapper.
        @param keySeq Key sequence as in string. example Qt::CTRL+Qt::Key_O sequence eguals "Ctrl+O" string.
        @param action Name of the action. If you want to use parameters the string should look the following: 
        @param executionType Execution type override. If 0 (default), uses the InputMapper's executionType attribute
        "More(Forward)" or "Move(Forward,100)" etc.
    */
    void RegisterMapping(const QString &keySeqString, const QString &action, int eventType = 1, int executionType = 0);

    /// Remove a key mapping
    void RemoveMapping(const QKeySequence &keySeq, int eventType = 1);
    /// Remove a key mapping
    void RemoveMapping(const QString &keySeqString, int eventType = 1);
    
    /// Returns the input context of this input mapper.
    InputContext *GetInputContext() const { return input_.get(); }

private:
    /// Constructor.
    /** @param module Declaring module.
    */
    explicit EC_InputMapper(IModule *module);

    boost::shared_ptr<InputContext> input_; ///< Input context for this EC.
    Mappings_t mappings_; ///< List of registered key sequence - action mappings.

private slots:
    /// Alters input context's parameters when attributes are changed.
    /** @param attribute Changed attribute.
        @param change Change type.
    */
    void HandleAttributeUpdated(IAttribute *, AttributeChange::Type change);

    /// Handles key events from input service.
    /** Performs entity action for for the parent entity if action mapping is registered for the key event.
        @param e Key event.
    */
    void HandleKeyEvent(KeyEvent *e);

    /// Handles mouse events from input service.
    /** Performs entity action for for the parent entity if action mapping is registered for the mouse event.
        @param e Mouse event.
    */
    void HandleMouseEvent(MouseEvent *e);
};

#endif
