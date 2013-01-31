/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_InputMapper.h
    @brief  Translates given set of key and mouse sequences to Entity Actions on the entity the component is part of. */

#pragma once

#include "TundraCoreApi.h"
#include "IComponent.h"
#include "InputFwd.h"
#include "KeyEvent.h"
#include "MouseEvent.h"

#include <QMap>
#include <QKeySequence>
#include <QVariant>

/// Translates given set of key and mouse sequences to Entity Actions on the entity the component is part of.
/** <table class="header">
    <tr>
    <td>
    <h2>InputMapper</h2>

    Registers an InputContext from the Input API and uses it to translate
    given set of key and mouse sequences to Entity Actions on the entity the component is part of.

    <b>Attributes</b>:
    <ul>
    <li>QString: contextName
    <div> @copydoc contextName </div>
    <li>int: contextPriority
    <div> @copydoc contextPriority </div>
    <li>bool: takeKeyboardEventsOverQt
    <div @copydoc takeKeyboardEventsOverQt </div>
    <li>bool: takeMouseEventsOverQt
    <div> @copydoc takeMouseEventsOverQt </div>
    <li>int: executionType
    <div> @copydoc executionType </div>
    <li>bool: modifiersEnabled
    <div> @copydoc modifiersEnabled </div>
    <li>bool: enabled
    <div> @copydoc enabled </div>
    <li>bool: keyrepeatTrigger
    <div> @copydoc keyrepeatTrigger </div>
    <li>bool: suppressKeyEvents
    <div> @copydoc suppressKeyEvents </div>
    <li>bool: suppressMouseEvents
    <div> @copydoc suppressMouseEvents </div>
    </ul>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li>"RegisterMapping": @copydoc RegisterMapping
    <li>"RemoveMapping": @copydoc RemoveMapping
    <li>"GetInputContext": @copydoc GetInputContext
    </ul>

    <b>Reacts on the following actions:</b>
    <ul>
    <li>...
    </ul>

    </td>
    </tr>

    <b>Can emit anykind of user-defined/registered actions.</b>

    <b>Doesn't depend on any components</b>

    </table> */
class TUNDRACORE_API EC_InputMapper : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_InputMapper", 13)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_InputMapper(Scene* scene);
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

    /// Execution type for actions
    Q_PROPERTY(int executionType READ getexecutionType WRITE setexecutionType)
    DEFINE_QPROPERTY_ATTRIBUTE(int, executionType);

    /// Modifier mode for key events. Default true. If false, modifiers are not checked for key events
    Q_PROPERTY(bool modifiersEnabled READ getmodifiersEnabled WRITE setmodifiersEnabled)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, modifiersEnabled);

    /// Is the input mapper enabled
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

    /// Trigger on keyrepeats, if it is off input mapper does not repeat keypressed actions. Default is on.
    ///\todo Rename to keyRepeatTrigger
    Q_PROPERTY(bool keyrepeatTrigger READ getkeyrepeatTrigger WRITE setkeyrepeatTrigger)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, keyrepeatTrigger);

    /// If true, the input mapper suppressed all the keyboard input events it handles.
    /// This allows prohibiting the signals from being handled further. Be aware that this also suppresses
    /// keyboard events from being sent to Qt if takeKeyboardEventsOverQt is also enabled.
    /// Default: false.
    Q_PROPERTY(bool suppressKeyEvents READ getsuppressKeyEvents WRITE setsuppressKeyEvents)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, suppressKeyEvents);

    /// If true, the input mapper suppressed all the mouse input events it handles.
    /// This allows prohibiting the signals from being handled further. Be aware that this also suppresses
    /// mouse events from being sent to Qt if takeMouseEventsOverQt is also enabled.
    /// Default: false.
    Q_PROPERTY(bool suppressMouseEvents READ getsuppressMouseEvents WRITE setsuppressMouseEvents)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, suppressMouseEvents);

public slots:
    /// Registers new key sequence - action mapping for this input mapper.
    /** @param keySeq Key sequence.
        @param action Name of the action. If you want to use parameters the string should look the following: 
        "More(Forward)" or "Move(Forward,100)" etc.
        @param eventType Event type (press, release), default press (1)
        @param executionType Execution type override. If 0 (default), uses the InputMapper's executionType attribute
        @note If registering key sequence with modifier keys, don't use Qt::Key enum - use Qt::Modifer enum instead. */
    void RegisterMapping(const QKeySequence &keySeq, const QString &action, int eventType = 1, int executionType = 0);
    /// @overload
    /** @param keySeq Key sequence as in string. example Qt::CTRL+Qt::Key_O sequence eguals "Ctrl+O" string. */
    void RegisterMapping(const QString &keySeqString, const QString &action, int eventType = 1, int executionType = 0);

    /// Removes a key mapping
    void RemoveMapping(const QKeySequence &keySeq, int eventType = 1);
    void RemoveMapping(const QString &keySeqString, int eventType = 1); /**< @overload */

    /// Returns the input context of this input mapper.
    InputContext *GetInputContext() const { return inputContext.get(); }

private:
    struct ActionInvocation
    {
        ActionInvocation() : executionType(0) {}
        QString name;
        int executionType;
    };

    typedef QMap<QPair<QKeySequence, KeyEvent::EventType>, ActionInvocation> ActionInvocationMap;
    ActionInvocationMap actionInvokationMappings; ///< List of registered key sequence - action mappings.
    shared_ptr<InputContext> inputContext; ///< Input context for this EC.

    /// Alters input context's parameters when attributes are changed.
    /** @param attribute Changed attribute.
     @param change Change type. */
    void AttributesChanged();

private slots:
    /// Handles key events from the input system.
    /** Performs entity action for for the parent entity if action mapping is registered for the key event.
        @param e Key event. */
    void HandleKeyEvent(KeyEvent *e);

    /// Handles mouse events from the input system.
    /** Performs entity action for for the parent entity if action mapping is registered for the mouse event.
        @param e Mouse event. */
    void HandleMouseEvent(MouseEvent *e);
};
