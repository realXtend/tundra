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

void EC_InputMapper::RegisterMapping(const QString &action, const QKeySequence &keySeq)
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
    RegisterMapping("MoveForward", Qt::Key_I);
    RegisterMapping("MoveBackward", Qt::Key_K);
    RegisterMapping("MoveLeft", Qt::Key_J);
    RegisterMapping("MoveRight", Qt::Key_L);
}

void EC_InputMapper::HandleKeyEvent(KeyEvent *key)
{
    Mappings_t::iterator it = mappings_.find(QKeySequence(key->keyCode | key->modifiers));
    if (it == mappings_.end())
        return;

    const QString &action = it.value();
    if (action.isEmpty())
    {
        LogWarning("");
        return;
    }

    Scene::Entity *entity = GetParentEntity();
    if (!entity)
    {
        LogWarning("Parent entity not set. Cannot execute action.");
        return;
    }

    LogDebug("Invoking action " + action.toStdString() + " for entity " + ToString(entity->GetId()));
    entity->Exec(action);
}

