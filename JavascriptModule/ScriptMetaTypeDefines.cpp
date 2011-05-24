/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ScriptMetaTypeDefines.cpp
 *  @brief  Registration of the Core API objects to Javascript.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "ScriptMetaTypeDefines.h"

#include "SceneAPI.h"
#include "Entity.h"
#include "IModule.h"
#include "IAssetTransfer.h"
#include "IAssetUploadTransfer.h"
#include "IAssetStorage.h"
#include "ScriptAsset.h"
#include "AssetCache.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "UiProxyWidget.h"
#include "FrameAPI.h"
#include "ConsoleAPI.h"
#include "SceneManager.h"
#include "AudioAPI.h"
#include "SoundChannel.h"
#include "InputContext.h"
#include "IRenderer.h"
#include "UiMainWindow.h"
#include "UiGraphicsView.h"
#include "EntityAction.h"
#include "InputFwd.h"
#include "ConfigAPI.h"
#include "LoggingFunctions.h"
#include "AssetAPI.h"

#include <QUiLoader>
#include <QFile>

/// Qt defines
Q_SCRIPT_DECLARE_QMETAOBJECT(QPushButton, QWidget*)
Q_SCRIPT_DECLARE_QMETAOBJECT(QWidget, QWidget*)
Q_SCRIPT_DECLARE_QMETAOBJECT(QTimer, QObject*);

/// Input API defines
Q_DECLARE_METATYPE(MouseEvent*)
Q_DECLARE_METATYPE(KeyEvent*)
Q_DECLARE_METATYPE(GestureEvent*)
Q_DECLARE_METATYPE(InputContext*)

/// Asset API defines
Q_DECLARE_METATYPE(AssetPtr);
Q_DECLARE_METATYPE(AssetTransferPtr);
Q_DECLARE_METATYPE(IAssetTransfer*);
Q_DECLARE_METATYPE(AssetUploadTransferPtr);
Q_DECLARE_METATYPE(IAssetUploadTransfer*);
Q_DECLARE_METATYPE(AssetStoragePtr);
Q_DECLARE_METATYPE(IAssetStorage*);
Q_DECLARE_METATYPE(ScriptAssetPtr);
Q_DECLARE_METATYPE(ScriptAsset*);
Q_DECLARE_METATYPE(AssetCache*);
Q_DECLARE_METATYPE(AssetMap);
Q_DECLARE_METATYPE(AssetStorageVector);

QScriptValue qScriptValueFromAssetMap(QScriptEngine *engine, const AssetMap &assetMap)
{
    QScriptValue v = engine->newArray(assetMap.size());
    int idx = 0;
    for(AssetMap::const_iterator iter = assetMap.begin(); iter != assetMap.end(); ++iter)
    {
        QScriptValue elem = qScriptValueFromBoostSharedPtr(engine, iter->second);
        v.setProperty(idx++, elem);
    }

    return v;
}

/// Deliberately a null function. Currently we don't need setting asset maps from the script side.
void qScriptValueToAssetMap(const QScriptValue &value, AssetMap &assetMap)
{
}

QScriptValue qScriptValueFromAssetStoragePtrVector(QScriptEngine *engine, const AssetStorageVector& vec)
{
    QScriptValue v = engine->newArray(vec.size());
    int idx = 0;
    for(std::vector<AssetStoragePtr>::const_iterator iter = vec.begin(); iter != vec.end(); ++iter)
    {
        QScriptValue elem = qScriptValueFromBoostSharedPtr(engine, *iter);
        v.setProperty(idx++, elem);
    }

    return v;
}

/// Deliberately a null function. Currently we don't need setting asset storage vectors from the script side.
void qScriptValueToAssetStoragePtrVector(const QScriptValue &value, AssetStorageVector& vec)
{
}

/// Ui defines
Q_DECLARE_METATYPE(UiProxyWidget*);
Q_DECLARE_METATYPE(UiMainWindow*);
Q_DECLARE_METATYPE(UiGraphicsView*);
Q_SCRIPT_DECLARE_QMETAOBJECT(UiProxyWidget, QWidget*)

/// Scene API defines.
Q_DECLARE_METATYPE(SceneAPI*);
Q_DECLARE_METATYPE(SceneManager*);
Q_DECLARE_METATYPE(Entity*);
Q_DECLARE_METATYPE(EntityAction*);
Q_DECLARE_METATYPE(EntityAction::ExecutionType);
Q_DECLARE_METATYPE(AttributeChange*);
Q_DECLARE_METATYPE(IComponent*);
Q_DECLARE_METATYPE(AttributeChange::Type);

/// Frame, Console and Debug API object defines.
Q_DECLARE_METATYPE(Framework*);
Q_DECLARE_METATYPE(IModule*);
Q_DECLARE_METATYPE(FrameAPI*);
Q_DECLARE_METATYPE(ConsoleAPI*);
Q_DECLARE_METATYPE(ConsoleCommand*);
Q_DECLARE_METATYPE(DelayedSignal*);

/// Audio API defines.
Q_DECLARE_METATYPE(AudioAPI*);
Q_DECLARE_METATYPE(SoundChannel*);

/// Config API defines.
Q_DECLARE_METATYPE(ConfigAPI*);

/// Renderer defines.
Q_DECLARE_METATYPE(RaycastResult*);

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

// Helper function. Added because new'ing a QPixmap in script seems to lead into growing memory use
QScriptValue setPixmapToLabel(QScriptContext *ctx, QScriptEngine *eng)
{
    if(ctx->argumentCount() == 2)
    {
        QObject *object = qscriptvalue_cast<QObject*>(ctx->argument(0));
        QString filename = qscriptvalue_cast<QString>(ctx->argument(1));
        
        QLabel *label = dynamic_cast<QLabel *>(object);
        if (label && QFile::exists(filename))
            label->setPixmap(QPixmap(filename));
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
    engine->globalObject().setProperty("setPixmapToLabel", engine->newFunction(setPixmapToLabel));   
/*
    engine->importExtension("qt.core");
    engine->importExtension("qt.gui");
    engine->importExtension("qt.network");
    engine->importExtension("qt.uitools");
    engine->importExtension("qt.xml");
    engine->importExtension("qt.xmlpatterns");
*/
//  Our deps contain these plugins as well, but we don't use them (for now at least).
//    engine->importExtension("qt.opengl");
//    engine->importExtension("qt.phonon");
//    engine->importExtension("qt.webkit"); //cvetan hacked this to build with msvc, patch is somewhere

}

Q_DECLARE_METATYPE(SoundChannelPtr);
Q_DECLARE_METATYPE(InputContextPtr);

void ExposeCoreApiMetaTypes(QScriptEngine *engine)
{
    // Input metatypes.
    qScriptRegisterQObjectMetaType<MouseEvent*>(engine);
    qScriptRegisterQObjectMetaType<KeyEvent*>(engine);
    qScriptRegisterQObjectMetaType<GestureEvent*>(engine);
    qScriptRegisterQObjectMetaType<InputContext*>(engine);
    qRegisterMetaType<InputContextPtr>("InputContextPtr");
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<InputContext>,
                            qScriptValueToBoostSharedPtr<InputContext>);
    qRegisterMetaType<KeyEvent::EventType>("KeyEvent::EventType");
    qRegisterMetaType<MouseEvent::EventType>("MouseEvent::EventType");
    qRegisterMetaType<MouseEvent::MouseButton>("MouseEvent::MouseButton");
    qRegisterMetaType<GestureEvent::EventType>("GestureEvent::EventType");

    // Scene metatypes.
    qScriptRegisterQObjectMetaType<SceneAPI*>(engine);
    qScriptRegisterQObjectMetaType<SceneManager*>(engine);
    qScriptRegisterQObjectMetaType<Entity*>(engine);
    qScriptRegisterQObjectMetaType<EntityAction*>(engine);
    qScriptRegisterQObjectMetaType<AttributeChange*>(engine);
    qScriptRegisterQObjectMetaType<IComponent*>(engine);
    //qRegisterMetaType<AttributeChange::Type>("AttributeChange::Type");
    qScriptRegisterMetaType(engine, toScriptValueEnum<AttributeChange::Type>, fromScriptValueEnum<AttributeChange::Type>);
    //qRegisterMetaType<EntityAction::ExecutionType>("EntityAction::ExecutionType");
    qScriptRegisterMetaType(engine, toScriptValueEnum<EntityAction::ExecutionType>, fromScriptValueEnum<EntityAction::ExecutionType>);

    qRegisterMetaType<entity_id_t>("entity_id_t");
    qScriptRegisterMetaType(engine, toScriptUInt<entity_id_t>, fromScriptUInt<entity_id_t>);

    // Framework metatype
    qScriptRegisterQObjectMetaType<Framework*>(engine);
    qScriptRegisterQObjectMetaType<IModule*>(engine);
    
    // Console metatypes.
    qScriptRegisterQObjectMetaType<ConsoleAPI*>(engine);
    qScriptRegisterQObjectMetaType<ConsoleCommand*>(engine);

    // Frame metatypes.
    qScriptRegisterQObjectMetaType<FrameAPI*>(engine);
    qScriptRegisterQObjectMetaType<DelayedSignal*>(engine);

    // Config metatypes.
    qScriptRegisterQObjectMetaType<ConfigAPI*>(engine);

    // Asset API
    qRegisterMetaType<AssetPtr>("AssetPtr");
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<IAsset>, qScriptValueToBoostSharedPtr<IAsset>);

    qRegisterMetaType<AssetTransferPtr>("AssetTransferPtr");
    qScriptRegisterQObjectMetaType<IAssetTransfer*>(engine);
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<IAssetTransfer>, qScriptValueToBoostSharedPtr<IAssetTransfer>);

    qRegisterMetaType<AssetUploadTransferPtr>("AssetUploadTransferPtr");
    qScriptRegisterQObjectMetaType<IAssetUploadTransfer*>(engine);
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<IAssetUploadTransfer>, qScriptValueToBoostSharedPtr<IAssetUploadTransfer>);

    qRegisterMetaType<AssetStoragePtr>("AssetStoragePtr");
    qScriptRegisterQObjectMetaType<IAssetStorage*>(engine);
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<IAssetStorage>, qScriptValueToBoostSharedPtr<IAssetStorage>);
/*
    qRegisterMetaType<AssetStoragePtr>("ScriptAssetPtr");
    qScriptRegisterQObjectMetaType<ScriptAsset*>(engine);
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<ScriptAsset>, qScriptValueToBoostSharedPtr<ScriptAsset>);
*/
    qScriptRegisterQObjectMetaType<AssetCache*>(engine);

    qRegisterMetaType<AssetMap>("AssetMap");
    qScriptRegisterMetaType<AssetMap>(engine, qScriptValueFromAssetMap, qScriptValueToAssetMap);

    qRegisterMetaType<AssetStorageVector>("AssetStorageVector");
    qScriptRegisterMetaType<AssetStorageVector>(engine, qScriptValueFromAssetStoragePtrVector, qScriptValueToAssetStoragePtrVector);
    
    // Ui metatypes.
    qScriptRegisterQObjectMetaType<UiMainWindow*>(engine);
    qScriptRegisterQObjectMetaType<UiGraphicsView*>(engine);
    qScriptRegisterQObjectMetaType<UiProxyWidget*>(engine);
    qScriptRegisterQObjectMetaType<QGraphicsScene*>(engine);

    // Add support to create proxy widgets in javascript side.
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
//    qScriptRegisterQObjectMetaType<Communications::InWorldVoice::SessionInterface*>(engine);
//    qScriptRegisterQObjectMetaType<Communications::InWorldVoice::ParticipantInterface*>(engine);
}


