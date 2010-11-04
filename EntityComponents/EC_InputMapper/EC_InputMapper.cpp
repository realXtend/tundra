/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InputMapper.h
 *  @brief  Registers an InputContext from the Naali Input subsystem and uses it to translate
 *          given set of keys to Entity Actions on the entity the component is part of.
 */

#include "StableHeaders.h"
#include "EC_InputMapper.h"

#include "IAttribute.h"
#include "Input.h"
#include "Entity.h"

#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_InputMapper")

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

EC_InputMapper::EC_InputMapper(IModule *module):
    IComponent(module->GetFramework()),
    contextName(this, "Input context name", "EC_InputMapper"),
    contextPriority(this, "Input context priority", 90),
    takeKeyboardEventsOverQt(this, "Take keyboard events over Qt", false),
    takeMouseEventsOverQt(this, "Take mouse events over Qt", false),
    mappings(this, "Mappings"),
    executionType(this, "Action execution type", 1),
    modifiersEnabled(this, "Key modifiers enable", true),
    enabled(this, "Enable actions", true)
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
    
    connect(this, SIGNAL(OnAttributeChanged(IAttribute *, AttributeChange::Type)),
        SLOT(AttributeUpdated(IAttribute *, AttributeChange::Type)));

    input_ = GetFramework()->GetInput()->RegisterInputContext(contextName.Get().toStdString().c_str(), contextPriority.Get());
    input_->SetTakeKeyboardEventsOverQt(takeKeyboardEventsOverQt.Get());
    input_->SetTakeMouseEventsOverQt(takeMouseEventsOverQt.Get());
    connect(input_.get(), SIGNAL(OnKeyEvent(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
    connect(input_.get(), SIGNAL(OnMouseEvent(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));
}

void EC_InputMapper::AttributeUpdated(IAttribute *attribute, AttributeChange::Type change)
{
    if(attribute == &contextName || attribute == &contextPriority)
    {
        input_.reset();
        input_ = GetFramework()->GetInput()->RegisterInputContext(contextName.Get().toStdString().c_str(), contextPriority.Get());
        connect(input_.get(), SIGNAL(OnKeyEvent(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
        connect(input_.get(), SIGNAL(OnMouseEvent(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));
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
    
    Mappings_t::iterator it;
    if (modifiersEnabled.Get())
        it = mappings_.find(qMakePair(QKeySequence(e->keyCode | e->modifiers), e->eventType));
    else
        it = mappings_.find(qMakePair(QKeySequence(e->keyCode), e->eventType));
    if (it == mappings_.end())
        return;

    Scene::Entity *entity = GetParentEntity();
    if (!entity)
    {
        LogWarning("Parent entity not set. Cannot execute action.");
        return;
    }

    ActionInvocation& invocation = it.value();
    QString &action = invocation.name;
    int execType = invocation.executionType;
    // If zero executiontype, use default
    if (!execType)
        execType = executionType.Get();
    
//    LogDebug("Invoking action " + action.toStdString() + " for entity " + ToString(entity->GetId()));

    // If the action has parameters, parse them from the action string.
    int idx = action.indexOf('(');
    if (idx != -1)
    {
        QString act = action.left(idx);
        QString parsedAction = action.mid(idx + 1);
        parsedAction.remove('(');
        parsedAction.remove(')');
        QStringList parameters = parsedAction.split(',');
        entity->Exec((EntityAction::ExecutionType)execType, act, parameters);
    }
    else
        entity->Exec((EntityAction::ExecutionType)execType, action);
}

void EC_InputMapper::HandleMouseEvent(MouseEvent *e)
{
    if (!enabled.Get())
        return;
    
    if (!GetParentEntity())
        return;
    
    //! \todo this hardcoding of look button logic (similar to RexMovementInput) is not nice!
    if ((e->IsButtonDown(MouseEvent::RightButton)) && (!GetFramework()->GetInput()->IsMouseCursorVisible()))
    {
        if (e->relativeX != 0)
        {
            GetParentEntity()->Exec((EntityAction::ExecutionType)executionType.Get(), "MouseLookX" , QString::number(e->relativeX));
        }
        if (e->relativeY != 0)
        {
            GetParentEntity()->Exec((EntityAction::ExecutionType)executionType.Get(), "MouseLookY" , QString::number(e->relativeY));
        }
    }
}
