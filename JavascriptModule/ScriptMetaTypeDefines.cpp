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
#include "QtInputMouseEvent.h"
#include "QtInputKeyEvent.h"
#include "UiProxyWidget.h"
#include "Frame.h"
#include "Console.h"
#include "SoundServiceInterface.h"

#include "QtInputMouseEvent.h"
#include "QtInputKeyEvent.h"
#include "EntityAction.h"

#include <QUiLoader>
#include <QFile>
#include "MemoryLeakCheck.h"

//! Qt defines
Q_SCRIPT_DECLARE_QMETAOBJECT(QPushButton, QWidget*)
Q_SCRIPT_DECLARE_QMETAOBJECT(QWidget, QWidget*)

//! Naali input defines
Q_DECLARE_METATYPE(MouseEvent*)
Q_DECLARE_METATYPE(KeyEvent*)

//! Naali Ui defines
Q_DECLARE_METATYPE(UiProxyWidget*);
Q_SCRIPT_DECLARE_QMETAOBJECT(UiProxyWidget, QWidget*)

//! Naali Scene defines.
Q_DECLARE_METATYPE(Scene::Entity*);
Q_DECLARE_METATYPE(EntityAction*);
Q_DECLARE_METATYPE(AttributeChange*);
Q_DECLARE_METATYPE(IComponent*);

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
    qRegisterMetaType<KeyEvent::EventType>("KeyEvent::EventType");
    qRegisterMetaType<MouseEvent::EventType>("MouseEvent::EventType");
    qRegisterMetaType<MouseEvent::MouseButton>("MouseEvent::MouseButton");

    // Scene metatypes.
    qScriptRegisterQObjectMetaType<Scene::Entity*>(engine);
    qScriptRegisterQObjectMetaType<EntityAction*>(engine);
    qScriptRegisterQObjectMetaType<AttributeChange*>(engine);
    qScriptRegisterQObjectMetaType<IComponent*>(engine);
    qRegisterMetaType<AttributeChange::Type>("AttributeChange::Type");
    qRegisterMetaType<EntityAction::ExecutionType>("EntityAction::ExecutionType");

    // Console metatypes.
    qScriptRegisterQObjectMetaType<ScriptConsole*>(engine);
    qScriptRegisterQObjectMetaType<Command*>(engine);

    // Frame metatypes.
    qScriptRegisterQObjectMetaType<Frame*>(engine);
    qScriptRegisterQObjectMetaType<DelayedSignal*>(engine);

    // Ui metatypes.
    qScriptRegisterQObjectMetaType<UiProxyWidget*>(engine);
    qScriptRegisterQObjectMetaType<QGraphicsScene*>(engine);
    //Add support to create proxy widget in javascript side.
    QScriptValue object = engine->scriptValueFromQMetaObject<UiProxyWidget>();
    engine->globalObject().setProperty("UiProxyWidget", object);
    
    // Sound metatypes.
    qRegisterMetaType<sound_id_t>("sound_id_t");
    qRegisterMetaType<Foundation::SoundServiceInterface::SoundState>("SoundState");
    qRegisterMetaType<Foundation::SoundServiceInterface::SoundState>("Foundation::SoundServiceInterface::SoundState");
    qRegisterMetaType<Foundation::SoundServiceInterface::SoundType>("SoundType");
    qRegisterMetaType<Foundation::SoundServiceInterface::SoundType>("Foundation::SoundServiceInterface::SoundType");
}


