#include "StableHeaders.h"

#include "ScriptAsset.h"
#include <boost/regex.hpp>

ScriptAsset::~ScriptAsset()
{
    Unload();
}

void ScriptAsset::Unload()
{
    scriptContent = "";
    references.clear();
}

bool ScriptAsset::DeserializeFromData(const u8 *data, size_t numBytes)
{
    QByteArray arr((const char *)data, numBytes);
    arr.append('\0');
    scriptContent = arr;

    ParseReferences();

    return true;
}

void ScriptAsset::ParseReferences()
{
    references.clear();
    std::string content = scriptContent.toStdString();
    boost::sregex_iterator searchEnd;

    boost::regex expression("!ref:\\s*(.*)\\s*(\\n|$)");
    for(boost::sregex_iterator iter(content.begin(), content.end(), expression); iter != searchEnd; ++iter)
    {
        AssetReference ref;
        ref.ref = (*iter)[1].str().c_str();
        ref.ref = ref.ref.trimmed();
        references.push_back(ref);
    }

    expression = boost::regex("engine.IncludeFile\\(\\s*\"\\s*(.*?)\\s*\"\\s*\\)");
    for(boost::sregex_iterator iter(content.begin(), content.end(), expression); iter != searchEnd; ++iter)
    {
        AssetReference ref;
        ref.ref = (*iter)[1].str().c_str();
        ref.ref = ref.ref.trimmed();
        references.push_back(ref);
    }
}
