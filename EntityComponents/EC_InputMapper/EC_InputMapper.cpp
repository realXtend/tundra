/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InputMapper.h
 *  @brief  Registers an InputContext from the Naali Input subsystem and uses it to translate
 *          given set of keys to Entity Actions on the entity the component is part of.
 */

#include "StableHeaders.h"
#include "EC_InputMapper.h"

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
    Foundation::ComponentInterface(module->GetFramework())
{
    input_ = GetFramework()->Input().RegisterInputContext("EC_InputMapper", 90);
    input_->SetTakeKeyboardEventsOverQt(true);
    connect(input_.get(), SIGNAL(KeyPressed(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));

    // Register some hardcoded mappings for testing purposes;
    RegisterMapping(Qt::Key_I, "Move(Forward)");
    RegisterMapping(Qt::Key_K, "Move(Backward)");
    RegisterMapping(Qt::Key_J, "Move(Left)");
    RegisterMapping(Qt::Key_L, "Move(Right)");
    RegisterMapping(Qt::Key_U, "Rotate(Left)");
    RegisterMapping(Qt::Key_O, "Rotate(Right)");
}

void EC_InputMapper::HandleKeyEvent(KeyEvent *key)
{
    Mappings_t::iterator it = mappings_.find(QKeySequence(key->keyCode | key->modifiers));
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
    if (action.contains('(') || action.contains(')') || action.contains(','))
    {
        ///\todo Better protection agains malformed action strings?
        int idx = action.indexOf('(');
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

