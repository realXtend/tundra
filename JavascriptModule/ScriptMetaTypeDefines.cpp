/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ScriptMetaTypeDefines.cpp
 *  @brief  Registration of Naali Core API to Javascript.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ScriptMetaTypeDefines.h"

#include "Entity.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "UiProxyWidget.h"
#include "Frame.h"
#include "Console.h"
#include "ISoundService.h"
#include "InputContext.h"

#include "EntityAction.h"

#include <QUiLoader>
#include <QFile>
#include "MemoryLeakCheck.h"

//! Qt defines
Q_SCRIPT_DECLARE_QMETAOBJECT(QPushButton, QWidget*)
Q_SCRIPT_DECLARE_QMETAOBJECT(QWidget, QWidget*)

///\todo Remove these two and move to Input API once NaaliCore is merged.
//! Naali input defines
Q_DECLARE_METATYPE(MouseEvent*)
Q_DECLARE_METATYPE(KeyEvent*)
Q_DECLARE_METATYPE(InputContext*)

//! Naali Ui defines
Q_DECLARE_METATYPE(UiProxyWidget*);
Q_SCRIPT_DECLARE_QMETAOBJECT(UiProxyWidget, QWidget*)

//! Naali Scene defines.
Q_DECLARE_METATYPE(Scene::Entity*);
Q_DECLARE_METATYPE(EntityAction*);
Q_DECLARE_METATYPE(EntityAction::ExecutionType);
Q_DECLARE_METATYPE(AttributeChange*);
Q_DECLARE_METATYPE(IComponent*);
Q_DECLARE_METATYPE(AttributeChange::Type);

//! Naali core API object defines.
Q_DECLARE_METATYPE(Frame*);
Q_DECLARE_METATYPE(ScriptConsole*);
Q_DECLARE_METATYPE(Command*);
Q_DECLARE_METATYPE(DelayedSignal*);

void ExposeQtMetaTypes(QScriptEngine *engine)
{
    QScriptValue object = engine->scriptValueFromQMetaObject<QPushButton>();
    engine->globalObject().setProperty("QPushButton", object);
    object = engine->scriptValueFromQMetaObject<QWidget>();
    engine->globalObject().setProperty("QWidget", object);
}

void ExposeCoreApiMetaTypes(QScriptEngine *engine)
{
    // Input metatypes.
    qScriptRegisterQObjectMetaType<MouseEvent*>(engine);
    qScriptRegisterQObjectMetaType<KeyEvent*>(engine);
    qScriptRegisterQObjectMetaType<InputContext*>(engine);
    qRegisterMetaType<KeyEvent::EventType>("KeyEvent::EventType");
    qRegisterMetaType<MouseEvent::EventType>("MouseEvent::EventType");
    qRegisterMetaType<MouseEvent::MouseButton>("MouseEvent::MouseButton");

    // Scene metatypes.
    qScriptRegisterQObjectMetaType<Scene::Entity*>(engine);
    qScriptRegisterQObjectMetaType<EntityAction*>(engine);
    qScriptRegisterQObjectMetaType<AttributeChange*>(engine);
    qScriptRegisterQObjectMetaType<IComponent*>(engine);
    //qRegisterMetaType<AttributeChange::Type>("AttributeChange::Type");
    qScriptRegisterMetaType(engine, toScriptValueEnum<AttributeChange::Type>, fromScriptValueEnum<AttributeChange::Type>);
    //qRegisterMetaType<EntityAction::ExecutionType>("EntityAction::ExecutionType");
    qScriptRegisterMetaType(engine, toScriptValueEnum<EntityAction::ExecutionType>, fromScriptValueEnum<EntityAction::ExecutionType>);

    // Console metatypes.
    qScriptRegisterQObjectMetaType<ScriptConsole*>(engine);
    qScriptRegisterQObjectMetaType<Command*>(engine);

    // Frame metatypes.
    qScriptRegisterQObjectMetaType<Frame*>(engine);
    qScriptRegisterQObjectMetaType<DelayedSignal*>(engine);

    // Ui metatypes.
    qScriptRegisterQObjectMetaType<UiProxyWidget*>(engine);
    qScriptRegisterQObjectMetaType<QGraphicsScene*>(engine);
    //Add support to create proxy widgets in javascript side.
    QScriptValue object = engine->scriptValueFromQMetaObject<UiProxyWidget>();
    engine->globalObject().setProperty("UiProxyWidget", object);
    
    // Sound metatypes.
    qRegisterMetaType<sound_id_t>("sound_id_t");
    qRegisterMetaType<ISoundService::SoundState>("SoundState");
    qRegisterMetaType<ISoundService::SoundState>("ISoundService::SoundState");
    qRegisterMetaType<ISoundService::SoundType>("SoundType");
    qRegisterMetaType<ISoundService::SoundType>("ISoundService::SoundType");
}


