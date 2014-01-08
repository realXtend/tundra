/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   ScriptMetaTypeDefines.cpp
    @brief  Registration of the Core API objects to QtScript. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ScriptMetaTypeDefines.h"

#include "Framework.h"
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

Q_SCRIPT_DECLARE_QMETAOBJECT(UiProxyWidget, QWidget*)

QScriptValue qScriptValueFromAssetMap(QScriptEngine *engine, const AssetMap &assetMap)
{
    QScriptValue v = engine->newArray((uint)assetMap.size());
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
    QScriptValue v = engine->newArray((uint)assetMap.size());
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
    QScriptValue v = engine->newArray((uint)vec.size());
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

// Tundra classes.
QScriptValue register_Color_prototype(QScriptEngine *engine);
QScriptValue register_ConfigData_prototype(QScriptEngine *engine);

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
    // JS -> C++: Enables correct JS 'number' type to our typedef conversion. Hits when slots take in eg. u32 as a parameter.
    qScriptRegisterMetaType(engine, toScriptU32OrSmaller<size_t>, fromScriptUInt<size_t>);
    qScriptRegisterMetaType(engine, toScriptS32OrSmaller<s8>, fromScriptChar<s8>);
    qScriptRegisterMetaType(engine, toScriptU32OrSmaller<u8>, fromScriptUChar<u8>);
    qScriptRegisterMetaType(engine, toScriptS32OrSmaller<s16>, fromScriptShort<s16>);
    qScriptRegisterMetaType(engine, toScriptU32OrSmaller<u16>, fromScriptUShort<u16>);
    qScriptRegisterMetaType(engine, toScriptS32OrSmaller<s32>, fromScriptInt<s32>);
    qScriptRegisterMetaType(engine, toScriptU32OrSmaller<u32>, fromScriptUInt<u32>);
    qScriptRegisterMetaType(engine, toScriptS64<s64>, fromScriptLongLong<s64>);
    qScriptRegisterMetaType(engine, toScriptU64<u64>, fromScriptULongLong<u64>);

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
    engine->globalObject().setProperty("math", mathNamespace, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    // Input metatypes.
    qScriptRegisterQObjectMetaType<MouseEvent*>(engine);
    qScriptRegisterQObjectMetaType<KeyEvent*>(engine);
    qScriptRegisterQObjectMetaType<GestureEvent*>(engine);
    qScriptRegisterQObjectMetaType<InputContext*>(engine);
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<InputContext>, qScriptValueToBoostSharedPtr<InputContext>);
    qScriptRegisterMetaType<InputAPI::KeyBindingMap>(engine, qScriptValueFromKeyBindingMap, qScriptValueToKeyBindingMap);

    /// @todo Investigate whether or not the enum name-value mapping can be auto-exposed to QtScript.
    QScriptValue mouseEventNs = engine->newObject();
    mouseEventNs.setProperty("NoButton", MouseEvent::NoButton, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("LeftButton", MouseEvent::LeftButton, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("RightButton", MouseEvent::RightButton, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("MiddleButton", MouseEvent::MiddleButton, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("Button4", MouseEvent::Button4, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("Button5", MouseEvent::Button5, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("MaxButtonMask", MouseEvent::MaxButtonMask, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("MouseEventInvalid", MouseEvent::MouseEventInvalid, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("MouseMove", MouseEvent::MouseMove, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("MouseScroll", MouseEvent::MouseScroll, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("MousePressed", MouseEvent::MousePressed, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("MouseReleased", MouseEvent::MouseReleased, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("MouseDoubleClicked", MouseEvent::MouseDoubleClicked, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("PressOriginNone", MouseEvent::PressOriginNone, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("PressOriginScene", MouseEvent::PressOriginScene, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    mouseEventNs.setProperty("PressOriginQtWidget", MouseEvent::PressOriginQtWidget, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("MouseEvent", mouseEventNs, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    QScriptValue keyEventNs = engine->newObject();
    keyEventNs.setProperty("KeyEventInvalid", KeyEvent::KeyEventInvalid, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    keyEventNs.setProperty("KeyPressed", KeyEvent::KeyPressed, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    keyEventNs.setProperty("KeyDown", KeyEvent::KeyDown, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    keyEventNs.setProperty("KeyReleased", KeyEvent::KeyReleased, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("KeyEvent", keyEventNs, QScriptValue::Undeletable | QScriptValue::ReadOnly);

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
    qScriptRegisterMetaType(engine, toScriptU32OrSmaller<entity_id_t>, fromScriptUInt<entity_id_t>);
    qScriptRegisterMetaType(engine, toScriptU32OrSmaller<component_id_t>, fromScriptUInt<component_id_t>);

    // NOTE For some reason attribute change mode is exposed weirdly to QtScript, hence the 'value' property.
    QScriptValue attributeChangeNs = engine->newObject();
    QScriptValue attributeChange0 = engine->newObject();
    attributeChange0.setProperty("value", AttributeChange::Default, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    QScriptValue attributeChange1 = engine->newObject();
    attributeChange1.setProperty("value", AttributeChange::Disconnected, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    QScriptValue attributeChange2 = engine->newObject();
    attributeChange2.setProperty("value", AttributeChange::LocalOnly, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    QScriptValue attributeChange3 = engine->newObject();
    attributeChange3.setProperty("value", AttributeChange::Replicate, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    attributeChangeNs.setProperty("Default", attributeChange0, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    attributeChangeNs.setProperty("Disconnected", attributeChange1, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    attributeChangeNs.setProperty("LocalOnly", attributeChange2, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    attributeChangeNs.setProperty("Replicate", attributeChange3, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("AttributeChange", attributeChangeNs, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    QScriptValue entityActionNs = engine->newObject();
    entityActionNs.setProperty("Invalid", EntityAction::Invalid, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    entityActionNs.setProperty("Local", EntityAction::Local, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    entityActionNs.setProperty("Server", EntityAction::Server, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    entityActionNs.setProperty("Peers", EntityAction::Peers, QScriptValue::Undeletable | QScriptValue::ReadOnly);
    engine->globalObject().setProperty("EntityAction", entityActionNs, QScriptValue::Undeletable | QScriptValue::ReadOnly);

    // Framework metatypes.
    qScriptRegisterQObjectMetaType<Framework*>(engine);
    qScriptRegisterQObjectMetaType<IModule*>(engine);
    register_Color_prototype(engine);

    // Console metatypes.
    qScriptRegisterQObjectMetaType<ConsoleAPI*>(engine);
    qScriptRegisterQObjectMetaType<ConsoleCommand*>(engine);

    // Frame metatypes.
    qScriptRegisterQObjectMetaType<FrameAPI*>(engine);
    qScriptRegisterQObjectMetaType<DelayedSignal*>(engine);

    // Config metatypes.
    qScriptRegisterQObjectMetaType<ConfigAPI*>(engine);
    register_ConfigData_prototype(engine);

    // Asset API
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<IAsset>, qScriptValueToBoostSharedPtr<IAsset>);
    qScriptRegisterQObjectMetaType<IAssetTransfer*>(engine);
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<IAssetTransfer>, qScriptValueToBoostSharedPtr<IAssetTransfer>);
    qScriptRegisterQObjectMetaType<IAssetUploadTransfer*>(engine);
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<IAssetUploadTransfer>, qScriptValueToBoostSharedPtr<IAssetUploadTransfer>);
    qScriptRegisterQObjectMetaType<IAssetStorage*>(engine);
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<IAssetStorage>, qScriptValueToBoostSharedPtr<IAssetStorage>);
    qScriptRegisterQObjectMetaType<AssetCache*>(engine);
    qScriptRegisterMetaType<AssetMap>(engine, qScriptValueFromAssetMap, qScriptValueToAssetMap);
    qScriptRegisterMetaType<AssetTransferMap>(engine, qScriptValueFromAssetTransferMap, qScriptValueToAssetTransferMap);
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
    qScriptRegisterMetaType(engine, toScriptValueEnum<SoundChannel::SoundState>, fromScriptValueEnum<SoundChannel::SoundState>);
    qScriptRegisterMetaType(engine, toScriptValueEnum<SoundChannel::SoundType>, fromScriptValueEnum<SoundChannel::SoundType>);
    qScriptRegisterQObjectMetaType<SoundChannel*>(engine);
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<SoundChannel>, qScriptValueToBoostSharedPtr<SoundChannel>);

    // Renderer metatypes
    qScriptRegisterQObjectMetaType<RaycastResult*>(engine);
}
