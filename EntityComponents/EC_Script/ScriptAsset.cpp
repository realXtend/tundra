#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include <boost/regex.hpp>
#include <QList>
#include "MemoryLeakCheck.h"

#include "ScriptAsset.h"
#include "AssetAPI.h"

ScriptAsset::~ScriptAsset()
{
    Unload();
}

void ScriptAsset::DoUnload()
{
    scriptContent = "";
    references.clear();
}

bool ScriptAsset::DeserializeFromData(const u8 *data, size_t numBytes)
{
    QByteArray arr((const char *)data, numBytes);
    scriptContent = arr;

    ParseReferences();

    return true;
}

bool ScriptAsset::SerializeTo(std::vector<u8> &dst, const QString &serializationParameters)
{
    QByteArray arr(scriptContent.toStdString().c_str());
    dst.clear();
    dst.insert(dst.end(), arr.data(), arr.data() + arr.size());
    return true;
}

void ScriptAsset::ParseReferences()
{
    references.clear();
    QStringList addedRefs;
    std::string content = scriptContent.toStdString();
    boost::sregex_iterator searchEnd;

    // In headless mode we dont want to mark certain asset types as
    // dependencies for the script, as they will fail Load() anyways
    QStringList ignoredAssetTypes;
    if (assetAPI->IsHeadless())
        ignoredAssetTypes << "QtUiFile" << "Texture" << "OgreParticle" << "OgreMaterial" << "Audio";

    boost::regex expression("!ref:\\s*(.*?)\\s*(\\n|$)");
    for(boost::sregex_iterator iter(content.begin(), content.end(), expression); iter != searchEnd; ++iter)
    {
        AssetReference ref;
        ///\todo The design of whether the LookupAssetRefToStorage should occur here, or internal to Asset API needs to be revisited.
        ref.ref = assetAPI->LookupAssetRefToStorage((*iter)[1].str().c_str());
        
        if (ignoredAssetTypes.contains(assetAPI->GetAssetTypeFromFileName(ref.ref)))
            continue;
        if (!addedRefs.contains(ref.ref, Qt::CaseInsensitive))
        {
            references.push_back(ref);
            addedRefs << ref.ref;
        }
    }

    expression = boost::regex("engine.IncludeFile\\(\\s*\"\\s*(.*?)\\s*\"\\s*\\)");
    for(boost::sregex_iterator iter(content.begin(), content.end(), expression); iter != searchEnd; ++iter)
    {
        AssetReference ref;
        ///\todo The design of whether the LookupAssetRefToStorage should occur here, or internal to Asset API needs to be revisited.
        ref.ref = assetAPI->LookupAssetRefToStorage((*iter)[1].str().c_str());
        if (!addedRefs.contains(ref.ref, Qt::CaseInsensitive))
        {
            references.push_back(ref);
            addedRefs << ref.ref;
        }
    }
}

bool ScriptAsset::IsLoaded() const
{
    return !scriptContent.isEmpty();
}
