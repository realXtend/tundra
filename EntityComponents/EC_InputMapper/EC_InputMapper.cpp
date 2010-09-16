/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InputMapper.h
 *  @brief  Registers an InputContext from the Naali Input subsystem and uses it to translate
 *          given set of keys to Entity Actions on the entity the component is part of.
 */

#include "StableHeaders.h"
#include "EC_InputMapper.h"

#include "AttributeInterface.h"
#include "InputServiceInterface.h"
#include "Entity.h"

#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_InputMapper")

EC_InputMapper::~EC_InputMapper()
{
    input_.reset();
}

void EC_InputMapper::RegisterMapping(const QKeySequence &keySeq, const QString &action)
{
    mappings_[keySeq] = action;
}

EC_InputMapper::EC_InputMapper(Foundation::ModuleInterface *module):
    Foundation::ComponentInterface(module->GetFramework()),
    contextName(this, "Input context name", "EC_InputMapper"),
    contextPriority(this, "Input context priority", 90),
    takeKeyboardEventsOverQt(this, "Take keyboard events over Qt", false),
    takeMouseEventsOverQt(this, "Take mouse events over Qt", false),
    mappings(this, "Mappings")
{
    connect(this, SIGNAL(OnAttributeChanged(AttributeInterface *, AttributeChange::Type)),
        SLOT(AttributeUpdated(AttributeInterface *, AttributeChange::Type)));

    input_ = GetFramework()->Input().RegisterInputContext(contextName.Get().toStdString().c_str(), contextPriority.Get());
    input_->SetTakeKeyboardEventsOverQt(takeKeyboardEventsOverQt.Get());
    input_->SetTakeMouseEventsOverQt(takeMouseEventsOverQt.Get());
    connect(input_.get(), SIGNAL(OnKeyEvent(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
    connect(input_.get(), SIGNAL(OnMouseEvent(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));

    RegisterMapping(Qt::Key_I, "Move(Forward)");
    RegisterMapping(Qt::Key_K, "Move(Backward)");
    RegisterMapping(Qt::Key_J, "Move(Left)");
    RegisterMapping(Qt::Key_L, "Move(Right)");
    RegisterMapping(Qt::Key_U, "Rotate(Left)");
    RegisterMapping(Qt::Key_O, "Rotate(Right)");
}

void EC_InputMapper::AttributeUpdated(AttributeInterface *attribute, AttributeChange::Type change)
{
    const std::string &name = attribute->GetNameString();
    if(name == contextName.GetNameString() || name == contextPriority.GetNameString())
    {
        input_.reset();
        input_ = GetFramework()->Input().RegisterInputContext(contextName.Get().toStdString().c_str(), contextPriority.Get());
        connect(input_.get(), SIGNAL(OnKeyEvent(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
        connect(input_.get(), SIGNAL(OnMouseEvent(MouseEvent *)), SLOT(HandleMouseEvent(MouseEvent *)));
    }
    else if(name == takeKeyboardEventsOverQt.GetNameString())
    {
        input_->SetTakeKeyboardEventsOverQt(takeKeyboardEventsOverQt.Get());
    }
    else if(name == takeMouseEventsOverQt.GetNameString())
    {
        input_->SetTakeMouseEventsOverQt(takeMouseEventsOverQt.Get());
    }
    else if(name == mappings.GetNameString())
    {
    }
}

void EC_InputMapper::HandleKeyEvent(KeyEvent *e)
{
    Mappings_t::iterator it = mappings_.find(QKeySequence(e->keyCode | e->modifiers));
    if (it == mappings_.end())
        return;

    Scene::Entity *entity = GetParentEntity();
    if (!entity)
    {
        LogWarning("Parent entity not set. Cannot execute action.");
        return;
    }

    QString &action = it.value();
//    LogDebug("Invoking action " + action.toStdString() + " for entity " + ToString(entity->GetId()));

    // If the action has parameters, parse them from the action string.
    int idx = action.indexOf('(');
    if (idx != -1)
    {
        QString act = action.left(idx);
        QString parsedAction = action.mid(idx + 1);
        parsedAction.remove('(');
        parsedAction.remove(')');
        QStringVector parameters = parsedAction.split(',').toVector();
        entity->Exec(act, parameters);
    }
    else
        entity->Exec(action);
}

void EC_InputMapper::HandleMouseEvent(MouseEvent *e)
{
    if (!GetParentEntity())
        return;

    if (e->IsButtonDown(MouseEvent::MiddleButton))
    {
        std::cout << e->relativeX << " " << e->relativeY << " " << e->relativeZ << std::endl;
        if (e->relativeX > 0 && abs(e->relativeX) >= 1)
        {
            GetParentEntity()->Exec("Move" ,"Right");
        }
        if (e->relativeX < 0 && abs(e->relativeX) >= 1)
        {
            GetParentEntity()->Exec("Move", "Left");
        }
        if (e->relativeY > 0 && abs(e->relativeY) >= 1)
        {
            GetParentEntity()->Exec("Move" ,"Backward");
        }
        if (e->relativeY < 0 && abs(e->relativeY) >= 1)
        {
            GetParentEntity()->Exec("Move", "Forward");
        }
    }
}
