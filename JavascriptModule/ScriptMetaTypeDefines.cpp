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
#include "SceneManager.h"
#include "Audio.h"
#include "SoundChannel.h"
#include "InputContext.h"
#include "RenderServiceInterface.h"
#include "CommunicationsService.h"
#include "NaaliMainWindow.h"
#include "NaaliGraphicsView.h"

#include "EntityAction.h"

#include "LoggingFunctions.h"

#include <QUiLoader>
#include <QFile>
#include "MemoryLeakCheck.h"

DEFINE_POCO_LOGGING_FUNCTIONS("Script")

//! Qt defines
Q_SCRIPT_DECLARE_QMETAOBJECT(QPushButton, QWidget*)
Q_SCRIPT_DECLARE_QMETAOBJECT(QWidget, QWidget*)
Q_SCRIPT_DECLARE_QMETAOBJECT(QTimer, QObject*);

///\todo Remove these two and move to Input API once NaaliCore is merged.
//! Naali input defines
Q_DECLARE_METATYPE(MouseEvent*)
Q_DECLARE_METATYPE(KeyEvent*)
Q_DECLARE_METATYPE(InputContext*)

//! Naali Ui defines
Q_DECLARE_METATYPE(UiProxyWidget*);
Q_DECLARE_METATYPE(NaaliMainWindow*);
Q_DECLARE_METATYPE(NaaliGraphicsView*);
Q_SCRIPT_DECLARE_QMETAOBJECT(UiProxyWidget, QWidget*)

//! Naali Scene defines.
Q_DECLARE_METATYPE(Scene::SceneManager*);
Q_DECLARE_METATYPE(Scene::Entity*);
Q_DECLARE_METATYPE(EntityAction*);
Q_DECLARE_METATYPE(EntityAction::ExecutionType);
Q_DECLARE_METATYPE(AttributeChange*);
Q_DECLARE_METATYPE(IComponent*);
Q_DECLARE_METATYPE(AttributeChange::Type);

//! Naali core API object defines.
Q_DECLARE_METATYPE(Foundation::Framework*);
Q_DECLARE_METATYPE(Frame*);
Q_DECLARE_METATYPE(ScriptConsole*);
Q_DECLARE_METATYPE(Command*);
Q_DECLARE_METATYPE(DelayedSignal*);

//! Naali Audio API object.
Q_DECLARE_METATYPE(AudioAPI*);
Q_DECLARE_METATYPE(SoundChannel*);

//! Naali renderer defines
Q_DECLARE_METATYPE(RaycastResult*);

//! Communications metatype
Q_DECLARE_METATYPE(Communications::InWorldVoice::SessionInterface*);
Q_DECLARE_METATYPE(Communications::InWorldVoice::ParticipantInterface*);

QScriptValue findChild(QScriptContext *ctx, QScriptEngine *eng)
{
    if(ctx->argumentCount() == 2)
    {
        QObject *object = qscriptvalue_cast<QObject*>(ctx->argument(0));
        QString childName = qscriptvalue_cast<QString>(ctx->argument(1));
        if(object)
        {
            QObject *obj = object->findChild<QObject*>(childName);
            if (obj)
                return eng->newQObject(obj);
        }
    }
    return QScriptValue();
}

void ExposeQtMetaTypes(QScriptEngine *engine)
{
    assert(engine);
    if (!engine)
        return;

    QScriptValue object = engine->scriptValueFromQMetaObject<QPushButton>();
    engine->globalObject().setProperty("QPushButton", object);
    object = engine->scriptValueFromQMetaObject<QWidget>();
    engine->globalObject().setProperty("QWidget", object);
    object = engine->scriptValueFromQMetaObject<QTimer>();
    engine->globalObject().setProperty("QTimer", object);
    engine->globalObject().setProperty("findChild", engine->newFunction(findChild));
/*
    ImportExtension(engine, "qt.core");
    ImportExtension(engine, "qt.gui");
    ImportExtension(engine, "qt.network");
    ImportExtension(engine, "qt.uitools");
    ImportExtension(engine, "qt.xml");
    ImportExtension(engine, "qt.xmlpatterns");
*/
//  Our deps contain these plugins as well, but we don't use them (for now at least).
//    ImportExtension(engine, "qt.opengl");
//    ImportExtension(engine, "qt.phonon");
//    ImportExtension(engine, "qt.webkit"); // The webkit plugin of QtScriptGenerator fails to load.

}

template<typename T>
static QScriptValue DereferenceBoostSharedPtr(QScriptContext *context, QScriptEngine *engine)
{
    boost::shared_ptr<T> ptr = context->thisObject().toVariant().value<boost::shared_ptr<T> >();
    return engine->newQObject(ptr.get());
}

template<typename T>
QScriptValue qScriptValueFromBoostSharedPtr(QScriptEngine *engine, const boost::shared_ptr<T> &ptr)
{
    QScriptValue v = engine->newVariant(QVariant::fromValue<boost::shared_ptr<T> >(ptr));
    v.setProperty("get", engine->newFunction(DereferenceBoostSharedPtr<T>), QScriptValue::ReadOnly | QScriptValue::Undeletable);
    return v;
}

template<typename T>
void qScriptValueToBoostSharedPtr(const QScriptValue &value, boost::shared_ptr<T> &ptr)
{   
    ptr = value.toVariant().value<boost::shared_ptr<T> >();
}


Q_DECLARE_METATYPE(AssetPtr);
Q_DECLARE_METATYPE(SoundChannelPtr);
Q_DECLARE_METATYPE(InputContextPtr);

void ExposeCoreApiMetaTypes(QScriptEngine *engine)
{
    // Input metatypes.
    qScriptRegisterQObjectMetaType<MouseEvent*>(engine);
    qScriptRegisterQObjectMetaType<KeyEvent*>(engine);
    qScriptRegisterQObjectMetaType<InputContext*>(engine);
    //qRegisterMetaType<InputContextPtr>("InputContextPtr");
    qRegisterMetaType<InputContextPtr>("InputContextPtr");
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<InputContext>,
                            qScriptValueToBoostSharedPtr<InputContext>);
    qRegisterMetaType<KeyEvent::EventType>("KeyEvent::EventType");
    qRegisterMetaType<MouseEvent::EventType>("MouseEvent::EventType");
    qRegisterMetaType<MouseEvent::MouseButton>("MouseEvent::MouseButton");

    // Scene metatypes.
    qScriptRegisterQObjectMetaType<Scene::SceneManager*>(engine);
    qScriptRegisterQObjectMetaType<Scene::Entity*>(engine);
    qScriptRegisterQObjectMetaType<EntityAction*>(engine);
    qScriptRegisterQObjectMetaType<AttributeChange*>(engine);
    qScriptRegisterQObjectMetaType<IComponent*>(engine);
    //qRegisterMetaType<AttributeChange::Type>("AttributeChange::Type");
    qScriptRegisterMetaType(engine, toScriptValueEnum<AttributeChange::Type>, fromScriptValueEnum<AttributeChange::Type>);
    //qRegisterMetaType<EntityAction::ExecutionType>("EntityAction::ExecutionType");
    qScriptRegisterMetaType(engine, toScriptValueEnum<EntityAction::ExecutionType>, fromScriptValueEnum<EntityAction::ExecutionType>);

    // Framework metatype
    qScriptRegisterQObjectMetaType<Foundation::Framework*>(engine);
    
    // Console metatypes.
    qScriptRegisterQObjectMetaType<ScriptConsole*>(engine);
    qScriptRegisterQObjectMetaType<Command*>(engine);

    // Frame metatypes.
    qScriptRegisterQObjectMetaType<Frame*>(engine);
    qScriptRegisterQObjectMetaType<DelayedSignal*>(engine);

    qRegisterMetaType<AssetPtr>("AssetPtr");
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<IAsset>, qScriptValueToBoostSharedPtr<IAsset>);

    // Ui metatypes.
    qScriptRegisterQObjectMetaType<NaaliMainWindow*>(engine);
    qScriptRegisterQObjectMetaType<NaaliGraphicsView*>(engine);
    qScriptRegisterQObjectMetaType<UiProxyWidget*>(engine);
    qScriptRegisterQObjectMetaType<QGraphicsScene*>(engine);
    //Add support to create proxy widgets in javascript side.
    QScriptValue object = engine->scriptValueFromQMetaObject<UiProxyWidget>();
    engine->globalObject().setProperty("UiProxyWidget", object);
    
    // Sound metatypes.
    qRegisterMetaType<sound_id_t>("sound_id_t");
    qRegisterMetaType<SoundChannel::SoundState>("SoundState");
    qRegisterMetaType<SoundChannelPtr>("SoundChannelPtr");
    qScriptRegisterQObjectMetaType<SoundChannel*>(engine);
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<SoundChannel>, qScriptValueToBoostSharedPtr<SoundChannel>);
    
    qRegisterMetaType<SoundChannel::SoundState>("SoundChannel::SoundState");
    qRegisterMetaType<SoundChannel::SoundType>("SoundType");
    qRegisterMetaType<SoundChannel::SoundType>("SoundChannel::SoundType");

    // Renderer metatypes
    qScriptRegisterQObjectMetaType<RaycastResult*>(engine);

    // Communications metatypes
    qScriptRegisterQObjectMetaType<Communications::InWorldVoice::SessionInterface*>(engine);
    qScriptRegisterQObjectMetaType<Communications::InWorldVoice::ParticipantInterface*>(engine);
}


