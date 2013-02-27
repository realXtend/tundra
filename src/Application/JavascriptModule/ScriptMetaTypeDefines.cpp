/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   ScriptMetaTypeDefines.cpp
    @brief  Registration of the Core API objects to QtScript. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ScriptMetaTypeDefines.h"

#include "SceneAPI.h"
#include "ChangeRequest.h"
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
#include "Scene/Scene.h"
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
#include "Math/MathFunc.h"
#include "QScriptEngineHelpers.h"
#include "InputAPI.h"

#include <QFile>
#include <QFontDatabase>

#include "MemoryLeakCheck.h"

// Input API defines
Q_DECLARE_METATYPE(MouseEvent*)
Q_DECLARE_METATYPE(KeyEvent*)
Q_DECLARE_METATYPE(GestureEvent*)
Q_DECLARE_METATYPE(InputContext*)
Q_DECLARE_METATYPE(InputContextPtr);
Q_DECLARE_METATYPE(InputAPI::KeyBindingMap);

// Asset API defines
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
Q_DECLARE_METATYPE(AssetTransferMap);
Q_DECLARE_METATYPE(AssetStorageVector);
Q_DECLARE_METATYPE(IAssetStorage::ChangeType);
Q_DECLARE_METATYPE(IAssetStorage::TrustState);

// Ui defines
Q_DECLARE_METATYPE(UiProxyWidget*);
Q_DECLARE_METATYPE(UiMainWindow*);
Q_DECLARE_METATYPE(UiGraphicsView*);
Q_SCRIPT_DECLARE_QMETAOBJECT(UiProxyWidget, QWidget*)

// Scene API defines.
Q_DECLARE_METATYPE(SceneAPI*);
Q_DECLARE_METATYPE(Scene*);
Q_DECLARE_METATYPE(Entity*);
Q_DECLARE_METATYPE(EntityAction*);
Q_DECLARE_METATYPE(EntityAction::ExecType);
Q_DECLARE_METATYPE(EntityAction::ExecTypeField);
Q_DECLARE_METATYPE(AttributeChange*);
Q_DECLARE_METATYPE(ChangeRequest*);
Q_DECLARE_METATYPE(IComponent*);
Q_DECLARE_METATYPE(AttributeChange::Type);

// Framework object defines.
Q_DECLARE_METATYPE(Framework*);
Q_DECLARE_METATYPE(IModule*);
Q_DECLARE_METATYPE(FrameAPI*);
Q_DECLARE_METATYPE(ConsoleAPI*);
Q_DECLARE_METATYPE(ConsoleCommand*);
Q_DECLARE_METATYPE(DelayedSignal*);
Q_DECLARE_METATYPE(ConfigAPI*);
Q_DECLARE_METATYPE(ConfigData*);
Q_DECLARE_METATYPE(RaycastResult*);

// Audio API defines.
Q_DECLARE_METATYPE(AudioAPI*);
Q_DECLARE_METATYPE(SoundChannel*);
Q_DECLARE_METATYPE(SoundChannel::SoundType)
Q_DECLARE_METATYPE(SoundChannel::SoundState)
Q_DECLARE_METATYPE(SoundChannelPtr);

QScriptValue qScriptValueFromAssetMap(QScriptEngine *engine, const AssetMap &assetMap)
{
    QScriptValue v = engine->newArray(assetMap.size());
    int idx = 0;
    /// \todo Want to change this to an associative array on the script side.
    for(AssetMap::const_iterator iter = assetMap.begin(); iter != assetMap.end(); ++iter)
    {
        QScriptValue elem = qScriptValueFromBoostSharedPtr(engine, iter->second);
        v.setProperty(idx++, elem);
    }

    return v;
}

/// Deliberately a null function. Currently we don't need setting asset maps from the script side.
void qScriptValueToAssetMap(const QScriptValue &/*value*/, AssetMap &/*assetMap*/)
{
}

QScriptValue qScriptValueFromAssetTransferMap(QScriptEngine *engine, const AssetTransferMap &assetMap)
{
    QScriptValue v = engine->newArray(assetMap.size());
    int idx = 0;
    for(AssetTransferMap::const_iterator iter = assetMap.begin(); iter != assetMap.end(); ++iter)
    {
        QScriptValue elem = qScriptValueFromBoostSharedPtr(engine, iter->second);
        v.setProperty(idx++, elem);
    }

    return v;
}

/// Deliberately a null function. Currently we don't need setting asset maps from the script side.
void qScriptValueToAssetTransferMap(const QScriptValue &/*value*/, AssetTransferMap &/*assetMap*/)
{
}

QScriptValue qScriptValueFromKeyBindingMap(QScriptEngine *engine, const InputAPI::KeyBindingMap &map)
{
    QScriptValue v = engine->newObject();
    for(InputAPI::KeyBindingMap::const_iterator iter = map.begin(); iter != map.end(); ++iter)
        v.setProperty(iter.key(), iter.value().toString());
    return v;
}

void qScriptValueToKeyBindingMap(const QScriptValue &value, InputAPI::KeyBindingMap &map)
{
    map.clear();
    QScriptValueIterator it(value);
    while(it.hasNext())
    {
        it.next();
        map[it.name()] = QKeySequence(it.value().toString(), QKeySequence::NativeText);
    }
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
void qScriptValueToAssetStoragePtrVector(const QScriptValue & /*value*/, AssetStorageVector& /*vec*/)
{
}

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

QScriptValue addApplicationFont(QScriptContext *ctx, QScriptEngine * /*eng*/)
{
    if(ctx->argumentCount() == 1)
    {
        QString fontName = qscriptvalue_cast<QString>(ctx->argument(0));
        QFontDatabase::addApplicationFont(fontName);
    }
    return QScriptValue();
}


// Helper function. Added because new'ing a QPixmap in script seems to lead into growing memory use
QScriptValue setPixmapToLabel(QScriptContext *ctx, QScriptEngine * /*eng*/)
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

    engine->globalObject().setProperty("findChild", engine->newFunction(findChild));
    engine->globalObject().setProperty("setPixmapToLabel", engine->newFunction(setPixmapToLabel));
    engine->globalObject().setProperty("addApplicationFont", engine->newFunction(addApplicationFont));
}

// Math classes.
QScriptValue register_float2_prototype(QScriptEngine *engine);
QScriptValue register_float3_prototype(QScriptEngine *engine);
QScriptValue register_float4_prototype(QScriptEngine *engine);
QScriptValue register_float3x3_prototype(QScriptEngine *engine);
QScriptValue register_float3x4_prototype(QScriptEngine *engine);
QScriptValue register_float4x4_prototype(QScriptEngine *engine);
QScriptValue register_Quat_prototype(QScriptEngine *engine);

QScriptValue register_AABB_prototype(QScriptEngine *engine);
QScriptValue register_Capsule_prototype(QScriptEngine *engine);
QScriptValue register_Circle_prototype(QScriptEngine *engine);
QScriptValue register_Frustum_prototype(QScriptEngine *engine);
QScriptValue register_LCG_prototype(QScriptEngine *engine);
QScriptValue register_Line_prototype(QScriptEngine *engine);
QScriptValue register_LineSegment_prototype(QScriptEngine *engine);
QScriptValue register_OBB_prototype(QScriptEngine *engine);
QScriptValue register_Plane_prototype(QScriptEngine *engine);
QScriptValue register_Ray_prototype(QScriptEngine *engine);
QScriptValue register_ScaleOp_prototype(QScriptEngine *engine);
QScriptValue register_Sphere_prototype(QScriptEngine *engine);
QScriptValue register_TranslateOp_prototype(QScriptEngine *engine);
QScriptValue register_Transform_prototype(QScriptEngine *engine);
QScriptValue register_Triangle_prototype(QScriptEngine *engine);

static QScriptValue math_SetMathBreakOnAssume(QScriptContext *context, QScriptEngine * /*engine*/)
{
    SetMathBreakOnAssume(qscriptvalue_cast<bool>(context->argument(0)));
    return QScriptValue();
}

static QScriptValue math_MathBreakOnAssume(QScriptContext * /*context*/, QScriptEngine *engine)
{
    return qScriptValueFromValue(engine, MathBreakOnAssume());
}

void ExposeCoreApiMetaTypes(QScriptEngine *engine)
{
    // Core type defines
    qRegisterMetaType<u8>("u8");
    qRegisterMetaType<u16>("u16");
    qRegisterMetaType<u32>("u32");
    qRegisterMetaType<u64>("u64");
    /// @todo For some reason simply using qRegisterMetaType for s8 doesn't make it work properly.
//    qRegisterMetaType<s8>("s8");
    qRegisterMetaType<s16>("s16");
    qRegisterMetaType<s32>("s32");
    qRegisterMetaType<s64>("s64");

    // Math
    register_float2_prototype(engine);
    register_float3_prototype(engine);
    register_float3x3_prototype(engine);
    register_float3x4_prototype(engine);
    register_float4_prototype(engine);
    register_float4x4_prototype(engine);
    register_AABB_prototype(engine);
    register_Capsule_prototype(engine);
    register_Circle_prototype(engine);
    register_Frustum_prototype(engine);
    register_LCG_prototype(engine);
    register_Line_prototype(engine);
    register_LineSegment_prototype(engine);
    register_OBB_prototype(engine);
    register_Plane_prototype(engine);
    register_Quat_prototype(engine);
    register_Ray_prototype(engine);
    register_ScaleOp_prototype(engine);
    register_Sphere_prototype(engine);
    register_TranslateOp_prototype(engine);
    register_Triangle_prototype(engine);
    register_Transform_prototype(engine);
    QScriptValue mathNamespace = engine->newObject();
    mathNamespace.setProperty("SetMathBreakOnAssume", engine->newFunction(math_SetMathBreakOnAssume, 1), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mathNamespace.setProperty("MathBreakOnAssume", engine->newFunction(math_MathBreakOnAssume, 0), QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("math", mathNamespace);

    // Input metatypes.
    qScriptRegisterQObjectMetaType<MouseEvent*>(engine);
    qScriptRegisterQObjectMetaType<KeyEvent*>(engine);
    qScriptRegisterQObjectMetaType<GestureEvent*>(engine);
    qScriptRegisterQObjectMetaType<InputContext*>(engine);
    qRegisterMetaType<InputContextPtr>("InputContextPtr");
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<InputContext>, qScriptValueToBoostSharedPtr<InputContext>);
    qRegisterMetaType<KeyEvent::EventType>("EventType");
    qRegisterMetaType<MouseEvent::EventType>("EventType");
    qRegisterMetaType<MouseEvent::MouseButton>("MouseButton");
    qRegisterMetaType<GestureEvent::EventType>("EventType");
    qRegisterMetaType<InputAPI::KeyBindingMap>("KeyBindingMap");
    qScriptRegisterMetaType<InputAPI::KeyBindingMap>(engine, qScriptValueFromKeyBindingMap, qScriptValueToKeyBindingMap);

    // Scene metatypes.
    qScriptRegisterQObjectMetaType<SceneAPI*>(engine);
    qScriptRegisterQObjectMetaType<Scene*>(engine);
    qScriptRegisterQObjectMetaType<Entity*>(engine);
    qScriptRegisterQObjectMetaType<EntityAction*>(engine);
    qScriptRegisterQObjectMetaType<AttributeChange*>(engine);
    qScriptRegisterQObjectMetaType<ChangeRequest*>(engine);
    qScriptRegisterQObjectMetaType<IComponent*>(engine);
    qScriptRegisterMetaType(engine, toScriptValueEnum<AttributeChange::Type>, fromScriptValueEnum<AttributeChange::Type>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<EntityAction::ExecType>, fromScriptValueEnum<EntityAction::ExecType>);
    qRegisterMetaType<EntityAction::ExecTypeField>("EntityAction::ExecTypeField");

    qRegisterMetaType<entity_id_t>("entity_id_t");
    qScriptRegisterMetaType(engine, toScriptUInt<entity_id_t>, fromScriptUInt<entity_id_t>);
    qRegisterMetaType<component_id_t>("component_id_t");
    qScriptRegisterMetaType(engine, toScriptUInt<component_id_t>, fromScriptUInt<component_id_t>);

    // Framework metatypes.
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
    qScriptRegisterQObjectMetaType<ConfigData*>(engine);

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

    qRegisterMetaType<AssetTransferMap>("AssetTransferMap");
    qScriptRegisterMetaType<AssetTransferMap>(engine, qScriptValueFromAssetTransferMap, qScriptValueToAssetTransferMap);

    qRegisterMetaType<AssetStorageVector>("AssetStorageVector");
    qScriptRegisterMetaType<AssetStorageVector>(engine, qScriptValueFromAssetStoragePtrVector, qScriptValueToAssetStoragePtrVector);

    qScriptRegisterMetaType(engine, toScriptValueEnum<IAssetStorage::ChangeType>, fromScriptValueEnum<IAssetStorage::ChangeType>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<IAssetStorage::TrustState>, fromScriptValueEnum<IAssetStorage::TrustState>);

    // Ui metatypes.
    qScriptRegisterQObjectMetaType<UiMainWindow*>(engine);
    qScriptRegisterQObjectMetaType<UiGraphicsView*>(engine);
    qScriptRegisterQObjectMetaType<UiProxyWidget*>(engine);

    // Add support to create proxy widgets in javascript side.
    QScriptValue object = engine->scriptValueFromQMetaObject<UiProxyWidget>();
    engine->globalObject().setProperty("UiProxyWidget", object);
    
    // Sound metatypes.
    qRegisterMetaType<sound_id_t>("sound_id_t");
    qScriptRegisterMetaType(engine, toScriptValueEnum<SoundChannel::SoundState>, fromScriptValueEnum<SoundChannel::SoundState>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<SoundChannel::SoundType>, fromScriptValueEnum<SoundChannel::SoundType>);
    qRegisterMetaType<SoundChannelPtr>("SoundChannelPtr");
    qScriptRegisterQObjectMetaType<SoundChannel*>(engine);
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<SoundChannel>, qScriptValueToBoostSharedPtr<SoundChannel>);

    // Renderer metatypes
    qScriptRegisterQObjectMetaType<RaycastResult*>(engine);
}
