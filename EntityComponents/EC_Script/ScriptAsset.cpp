#include "StableHeaders.h"

#include "ScriptAsset.h"
#include "AssetAPI.h"
#include <boost/regex.hpp>

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
    std::string content = scriptContent.toStdString();
    boost::sregex_iterator searchEnd;

    boost::regex expression("!ref:\\s*(.*?)\\s*(\\n|$)");
    for(boost::sregex_iterator iter(content.begin(), content.end(), expression); iter != searchEnd; ++iter)
    {
        AssetReference ref;
        ///\todo The design of whether the LookupAssetRefToStorage should occur here, or internal to Asset API needs to be revisited.
        ref.ref = assetAPI->LookupAssetRefToStorage((*iter)[1].str().c_str());
        references.push_back(ref);
    }

    expression = boost::regex("engine.IncludeFile\\(\\s*\"\\s*(.*?)\\s*\"\\s*\\)");
    for(boost::sregex_iterator iter(content.begin(), content.end(), expression); iter != searchEnd; ++iter)
    {
        AssetReference ref;
        ///\todo The design of whether the LookupAssetRefToStorage should occur here, or internal to Asset API needs to be revisited.
        ref.ref = assetAPI->LookupAssetRefToStorage((*iter)[1].str().c_str());
        references.push_back(ref);
    }
}
