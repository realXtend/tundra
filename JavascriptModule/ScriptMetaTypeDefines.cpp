/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ScriptMetaTypeDefines.cpp
 *  @brief  Registration of Naali Core API to Javascript.
 */

#include "StableHeaders.h"
#include "ScriptMetaTypeDefines.h"

#include "Entity.h"
#include "QtInputMouseEvent.h"
#include "QtInputKeyEvent.h"
#include "UiProxyWidget.h"
#include "Frame.h"

#include "QtInputMouseEvent.h"
#include "QtInputKeyEvent.h"
#include "EntityAction.h"

#include <QUiLoader>
#include <QFile>

//! Qt defines
Q_SCRIPT_DECLARE_QMETAOBJECT(QPushButton, QWidget*)
Q_SCRIPT_DECLARE_QMETAOBJECT(QWidget, QWidget*)

//! Naali input defines
Q_DECLARE_METATYPE(MouseEvent*)
Q_DECLARE_METATYPE(KeyEvent*)

//! Naali Ui defines
Q_DECLARE_METATYPE(UiProxyWidget*);

//! Naali Scene defines.
Q_DECLARE_METATYPE(Scene::Entity*);
Q_DECLARE_METATYPE(Scene::EntityAction*);
Q_DECLARE_METATYPE(AttributeChange*);
Q_DECLARE_METATYPE(Foundation::ComponentInterface*);

//! Naali core API object defines.
Q_DECLARE_METATYPE(Frame*);

void ReqisterQtMetaTypes(QScriptEngine *engine)
{
    QScriptValue object = engine->scriptValueFromQMetaObject<QPushButton>();
    engine->globalObject().setProperty("QPushButton", object);
    object = engine->scriptValueFromQMetaObject<QWidget>();
    engine->globalObject().setProperty("QWidget", object);
    object = engine->scriptValueFromQMetaObject<QUiLoader>();
}

void ReqisterInputMetaTypes(QScriptEngine *engine)
{
    qScriptRegisterQObjectMetaType<MouseEvent*>(engine);
    qScriptRegisterQObjectMetaType<KeyEvent*>(engine);
}

void ReqisterSceneMetaTypes(QScriptEngine *engine)
{
    qScriptRegisterQObjectMetaType<Scene::Entity*>(engine);
    qScriptRegisterQObjectMetaType<Scene::EntityAction*>(engine);
    qScriptRegisterQObjectMetaType<AttributeChange*>(engine);
    qScriptRegisterQObjectMetaType<Foundation::ComponentInterface*>(engine);

    qRegisterMetaType<AttributeChange::Type>("AttributeChange::Type");
    qRegisterMetaType<Scene::EntityAction*>("Scene::EntityAction*");
}

void ReqisterUiMetaTypes(QScriptEngine *engine)
{
    qScriptRegisterQObjectMetaType<UiProxyWidget*>(engine);

    qRegisterMetaType<KeyEvent::EventType>("KeyEvent::EventType");
    qRegisterMetaType<MouseEvent::EventType>("MouseEvent::EventType");
    qRegisterMetaType<MouseEvent::MouseButton>("MouseEvent::MouseButton");
}

void ReqisterFrameMetaTypes(QScriptEngine *engine)
{
    qScriptRegisterQObjectMetaType<Frame*>(engine);
}

