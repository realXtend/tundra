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

/*
std::vector<ProtocolUtilities::MultiObjectUpdateInfo> updates;
EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
OgreRenderer::EC_OgrePlaceable *ogre_pos = entity->GetComponent<OgreRenderer::EC_OgrePlaceable >().get();
if (!prim && !ogre_pos)
    return;

ProtocolUtilities::MultiObjectUpdateInfo update;
update.local_id_ = prim->LocalId;
update.position_ = ogre_pos->GetPosition();
update.orientation_ = ogre_pos->GetOrientation();
update.scale_ = ogre_pos->GetScale();

updates.push_back(new_info);

worldStream->GetServerConnection()->SendMultipleObjectUpdatePacket(update_info_list);
*/
EC_InputMapper::~EC_InputMapper()
{
    input_.reset();
}

void EC_InputMapper::RegisterMapping(const QString &action, const QKeySequence &keySeq)
{
//    QString inputActionName = input_->Name() + '.' + action;
//    const QKeySequence &keyBinding = framework_->Input().KeyBinding(inputActionName, keySeq);
    mappings_[keySeq] = action;
//    KeyEventSignal *signal = &input_->RegisterKeyEvent(keySeq);
//    connect(signal, SIGNAL(SequencePressed(KeyEvent &)), SLOT(test()));
}

EC_InputMapper::EC_InputMapper(Foundation::ModuleInterface *module):
    Foundation::ComponentInterface(module->GetFramework())
{
    ///\todo Generate random/unique name for input context?
    input_ = GetFramework()->Input().RegisterInputContext("EC_InputMapper", 900);
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
    LogDebug("HandleKeyEvent");
    // We only act on key presses that are not repeats.
//    if (key->eventType != KeyEvent::KeyPressed || key->keyPressCount > 1)
//        return;

    Mappings_t::iterator it = mappings_.find(QKeySequence(key->keyCode, key->modifiers));
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

    LogDebug("Performing action " + action.toStdString() + "for entity " + ToString(entity->GetId()));
    entity->Exec(action);
}

