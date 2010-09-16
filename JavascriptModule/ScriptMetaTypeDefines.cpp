#include "StableHeaders.h"
#include "ScriptMetaTypeDefines.h"

#include "Entity.h"
#include "QtInputMouseEvent.h"
#include "QtInputKeyEvent.h"
#include "UiProxyWidget.h"

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
Q_DECLARE_METATYPE(AttributeChange*);
Q_DECLARE_METATYPE(Foundation::ComponentInterface*);

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
    qScriptRegisterQObjectMetaType<AttributeChange*>(engine);
    qScriptRegisterQObjectMetaType<Foundation::ComponentInterface*>(engine);

    qRegisterMetaType<AttributeChange::Type>("AttributeChange::Type");
}

void ReqisterUiMetaTypes(QScriptEngine *engine)
{
    qScriptRegisterQObjectMetaType<UiProxyWidget*>(engine);
}