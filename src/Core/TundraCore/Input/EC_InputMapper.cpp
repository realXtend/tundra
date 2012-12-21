/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_InputMapper.cpp
    @brief  Translates given set of key and mouse sequences to Entity Actions on the entity the component is part of. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_InputMapper.h"
#include "InputAPI.h"

#include "Framework.h"
#include "IAttribute.h"
#include "AttributeMetadata.h"
#include "Entity.h"
#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

EC_InputMapper::EC_InputMapper(Scene* scene):
    IComponent(scene),
    contextName(this, "Input context name", "EC_InputMapper"),
    contextPriority(this, "Input context priority", 90),
    takeKeyboardEventsOverQt(this, "Take keyboard events over Qt", false),
    takeMouseEventsOverQt(this, "Take mouse events over Qt", false),
    executionType(this, "Action execution type", 1),
    modifiersEnabled(this, "Key modifiers enable", true),
    enabled(this, "Enable actions", true),
    keyrepeatTrigger(this, "Trigger on keyrepeats", true),
    suppressKeyEvents(this, "Suppress used keyboard events", false),
    suppressMouseEvents(this, "Suppress used mouse events", false)
{
    static AttributeMetadata executionAttrData;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        executionAttrData.enums[EntityAction::Local] = "Local";
        executionAttrData.enums[EntityAction::Server] = "Server";
        executionAttrData.enums[EntityAction::Server | EntityAction::Local] = "Local+Server";
        executionAttrData.enums[EntityAction::Peers] = "Peers";
        executionAttrData.enums[EntityAction::Peers | EntityAction::Local] = "Local+Peers";
        executionAttrData.enums[EntityAction::Peers | EntityAction::Server] = "Local+Server";
        executionAttrData.enums[EntityAction::Peers | EntityAction::Server | EntityAction::Local] = "Local+Server+Peers";
        metadataInitialized = true;
    }
    executionType.SetMetadata(&executionAttrData);

    inputContext = GetFramework()->Input()->RegisterInputContext(contextName.Get().toStdString().c_str(), contextPriority.Get());
    inputContext->SetTakeKeyboardEventsOverQt(takeKeyboardEventsOverQt.Get());
    inputContext->SetTakeMouseEventsOverQt(takeMouseEventsOverQt.Get());
    connect(inputContext.get(), SIGNAL(KeyEventReceived(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
    connect(inputContext.get(), SIGNAL(MouseEventReceived(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));
}

EC_InputMapper::~EC_InputMapper()
{
    inputContext.reset();
}

void EC_InputMapper::RegisterMapping(const QKeySequence &keySeq, const QString &action, int eventType, int executionType)
{
    ActionInvocation invocation;
    invocation.name = action;
    invocation.executionType = executionType;
    actionInvokationMappings[qMakePair(keySeq, (KeyEvent::EventType)eventType)] = invocation;
}

void EC_InputMapper::RegisterMapping(const QString &keySeq, const QString &action, int eventType, int executionType)
{
    ActionInvocation invocation;
    invocation.name = action;
    invocation.executionType = executionType;
    QKeySequence key(keySeq);
    if (!key.isEmpty())
        actionInvokationMappings[qMakePair(key, (KeyEvent::EventType)eventType)] = invocation;
}

void EC_InputMapper::RemoveMapping(const QKeySequence &keySeq, int eventType)
{
    ActionInvocationMap::iterator it = actionInvokationMappings.find(qMakePair(keySeq, (KeyEvent::EventType)eventType));
    if (it != actionInvokationMappings.end())
        actionInvokationMappings.erase(it);
}

void EC_InputMapper::RemoveMapping(const QString &keySeq, int eventType)
{
    ActionInvocationMap::iterator it = actionInvokationMappings.find(qMakePair(QKeySequence(keySeq), (KeyEvent::EventType)eventType));
    if (it != actionInvokationMappings.end())
        actionInvokationMappings.erase(it);
}

void EC_InputMapper::AttributesChanged()
{
    if (contextName.ValueChanged())
        inputContext->SetName(contextName.Get());
    if (contextPriority.ValueChanged())
        inputContext->SetPriority(contextPriority.Get());
    if(takeKeyboardEventsOverQt.ValueChanged())
        inputContext->SetTakeKeyboardEventsOverQt(takeKeyboardEventsOverQt.Get());
    if(takeMouseEventsOverQt.ValueChanged())
        inputContext->SetTakeMouseEventsOverQt(takeMouseEventsOverQt.Get());
}

void EC_InputMapper::HandleKeyEvent(KeyEvent *e)
{
    if (!enabled.Get())
        return;

    // Do not act on already handled key events.
    if (!e || e->handled)
        return;
    
    if (!keyrepeatTrigger.Get())
    {
        // Now we do not repeat key pressed events.
        if (e != 0 && e->eventType == KeyEvent::KeyPressed &&  e->keyPressCount > 1)
            return;
    }

    ActionInvocationMap::iterator it;
    // If 'modifiers are enabled', then it means we distinguish 'E', 'Shift+E' and 'Ctrl+E' as separate key sequences...
    // But this separation can only be done for pressed and hold-down keyboard events. If 'E' is released, it needs to be detected as released individual of
    // whether any modifiers are down.
    if (modifiersEnabled.Get() && e->eventType != KeyEvent::KeyReleased) 
        it = actionInvokationMappings.find(qMakePair(QKeySequence(e->keyCode | e->modifiers), e->eventType));
    else // .. otherwise, we treat 'E', 'Shift+E' and 'Ctrl+E' all just simply as 'E'.
        it = actionInvokationMappings.find(qMakePair(QKeySequence(e->keyCode), e->eventType));
    if (it == actionInvokationMappings.end())
        return;

    Entity *entity = ParentEntity();
    if (!entity)
    {
        LogWarning("Parent entity not set. Cannot execute action.");
        return;
    }

    ActionInvocation& invocation = it.value();
    QString &action = invocation.name;
    int execType = invocation.executionType;
    // If zero execution type, use default
    if (!execType)
        execType = executionType.Get();
    
    // If the action has parameters, parse them from the action string.
    int idx = action.indexOf('(');
    if (idx != -1)
    {
        QString act = action.left(idx);
        QString parsedAction = action.mid(idx + 1);
        parsedAction.remove('(');
        parsedAction.remove(')');
        QStringList parameters = parsedAction.split(',');
        entity->Exec((EntityAction::ExecTypeField)execType, act, parameters);
    }
    else
        entity->Exec((EntityAction::ExecTypeField)execType, action);
    if (suppressKeyEvents.Get())
        e->Suppress();
}

void EC_InputMapper::HandleMouseEvent(MouseEvent *e)
{
    if (!enabled.Get())
        return;
    if (!ParentEntity())
        return;
    
    /// \todo this hard coding of look button logic is not nice! Android implements left mouse move (touch drag) as mouselook currently.
#ifdef ANDROID
    if (e->IsButtonDown(MouseEvent::LeftButton))
#else
    if ((e->IsButtonDown(MouseEvent::RightButton)) && (!GetFramework()->Input()->IsMouseCursorVisible()))
#endif
    {
        if (e->relativeX != 0)
        {
            ParentEntity()->Exec((EntityAction::ExecTypeField)executionType.Get(), "MouseLookX" , QString::number(e->relativeX));
            if (suppressMouseEvents.Get())
                e->Suppress();
        }
        if (e->relativeY != 0)
        {
            ParentEntity()->Exec((EntityAction::ExecTypeField)executionType.Get(), "MouseLookY" , QString::number(e->relativeY));
            if (suppressMouseEvents.Get())
                e->Suppress();
        }
    }
    if (e->eventType == MouseEvent::MouseScroll && e->relativeZ != 0)
    {
        ParentEntity()->Exec((EntityAction::ExecTypeField)executionType.Get(), "MouseScroll" , QString::number(e->relativeZ));
        if (suppressMouseEvents.Get())
            e->Suppress();
    }
}
