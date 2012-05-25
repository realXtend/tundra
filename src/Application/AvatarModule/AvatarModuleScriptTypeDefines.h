// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "AvatarDescAsset.h"

#include "../JavascriptModule/ScriptMetaTypeDefines.h"
#include "QScriptEngineHelpers.h"

#include <QScriptEngine>

Q_DECLARE_METATYPE(AvatarDescAssetPtr);

void RegisterAvatarModuleMetaTypes(QScriptEngine *engine)
{
    qRegisterMetaType<AvatarDescAssetPtr>("AvatarDescAssetPtr");
    qScriptRegisterMetaType(engine, qScriptValueFromBoostSharedPtr<AvatarDescAsset>, qScriptValueToBoostSharedPtr<AvatarDescAsset>);
}
