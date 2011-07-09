/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InputMapper.h
 *  @brief  Registers an InputContext from the Input API and uses it to translate
 *          given set of keys to Entity Actions on the entity the component is part of.
 */

#include "DebugOperatorNew.h"
#include "EC_InputMapper.h"
#include "Framework.h"

#include "IAttribute.h"
#include "AttributeMetadata.h"
#include "InputAPI.h"
#include "Entity.h"

#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

EC_InputMapper::~EC_InputMapper()
{
    input_.reset();
}

void EC_InputMapper::RegisterMapping(const QKeySequence &keySeq, const QString &action, int eventType, int executionType)
{
    ActionInvocation invocation;
    invocation.name = action;
    invocation.executionType = executionType;
    
    mappings_[qMakePair(keySeq, (KeyEvent::EventType)eventType)] = invocation;
}

void EC_InputMapper::RegisterMapping(const QString &keySeq, const QString &action, int eventType, int executionType)
{
    ActionInvocation invocation;
    invocation.name = action;
    invocation.executionType = executionType;
    
    QKeySequence key(keySeq);
    if(!key.isEmpty())
    {
        mappings_[qMakePair(key, (KeyEvent::EventType)eventType)] = invocation;
    }
}

void EC_InputMapper::RemoveMapping(const QKeySequence &keySeq, int eventType)
{
    Mappings_t::iterator it = mappings_.find(qMakePair(keySeq, (KeyEvent::EventType)eventType));
    if (it != mappings_.end())
        mappings_.erase(it);
}

void EC_InputMapper::RemoveMapping(const QString &keySeq, int eventType)
{
    Mappings_t::iterator it = mappings_.find(qMakePair(QKeySequence(keySeq), (KeyEvent::EventType)eventType));
    if (it != mappings_.end())
        mappings_.erase(it);
}

EC_InputMapper::EC_InputMapper(Scene* scene):
    IComponent(scene),
    contextName(this, "Input context name", "EC_InputMapper"),
    contextPriority(this, "Input context priority", 90),
    takeKeyboardEventsOverQt(this, "Take keyboard events over Qt", false),
    takeMouseEventsOverQt(this, "Take mouse events over Qt", false),
    mappings(this, "Mappings"),
    executionType(this, "Action execution type", 1),
    modifiersEnabled(this, "Key modifiers enable", true),
    enabled(this, "Enable actions", true),
    keyrepeatTrigger(this, "Trigger on keyrepeats", true)
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
    
    connect(this, SIGNAL(AttributeChanged(IAttribute *, AttributeChange::Type)),
        SLOT(HandleAttributeUpdated(IAttribute *, AttributeChange::Type)));

    input_ = GetFramework()->Input()->RegisterInputContext(contextName.Get().toStdString().c_str(), contextPriority.Get());
    input_->SetTakeKeyboardEventsOverQt(takeKeyboardEventsOverQt.Get());
    input_->SetTakeMouseEventsOverQt(takeMouseEventsOverQt.Get());
    connect(input_.get(), SIGNAL(KeyEventReceived(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
    connect(input_.get(), SIGNAL(MouseEventReceived(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));
}

void EC_InputMapper::HandleAttributeUpdated(IAttribute *attribute, AttributeChange::Type change)
{
    if(attribute == &contextName || attribute == &contextPriority)
    {
        input_.reset();
        input_ = GetFramework()->Input()->RegisterInputContext(contextName.Get().toStdString().c_str(), contextPriority.Get());
        connect(input_.get(), SIGNAL(KeyEventReceived(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
        connect(input_.get(), SIGNAL(MouseEventReceived(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));
    }
    else if(attribute == &takeKeyboardEventsOverQt)
    {
        input_->SetTakeKeyboardEventsOverQt(takeKeyboardEventsOverQt.Get());
    }
    else if(attribute == &takeMouseEventsOverQt)
    {
        input_->SetTakeMouseEventsOverQt(takeMouseEventsOverQt.Get());
    }
    else if(attribute == &mappings)
    {
    }
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

    Mappings_t::iterator it;
    if (modifiersEnabled.Get())
        it = mappings_.find(qMakePair(QKeySequence(e->keyCode | e->modifiers), e->eventType));
    else
        it = mappings_.find(qMakePair(QKeySequence(e->keyCode), e->eventType));
    if (it == mappings_.end())
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
        entity->Exec((EntityAction::ExecType)execType, act, parameters);
    }
    else
        entity->Exec((EntityAction::ExecType)execType, action);
}

void EC_InputMapper::HandleMouseEvent(MouseEvent *e)
{
    if (!enabled.Get())
        return;
    if (!ParentEntity())
        return;
    
    /// \todo this hard coding of look button logic is not nice!
    if ((e->IsButtonDown(MouseEvent::RightButton)) && (!GetFramework()->Input()->IsMouseCursorVisible()))
    {
        if (e->relativeX != 0)
            ParentEntity()->Exec((EntityAction::ExecType)executionType.Get(), "MouseLookX" , QString::number(e->relativeX));
        if (e->relativeY != 0)
            ParentEntity()->Exec((EntityAction::ExecType)executionType.Get(), "MouseLookY" , QString::number(e->relativeY));
    }
    if (e->relativeZ != 0 && e->relativeZ != -1) // For some reason this is -1 without scroll
        ParentEntity()->Exec((EntityAction::ExecType)executionType.Get(), "MouseScroll" , QString::number(e->relativeZ));
}
