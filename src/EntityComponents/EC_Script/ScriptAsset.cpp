// For conditions of distribution and use, see copyright notice in LICENSE

#include "DebugOperatorNew.h"

#include "ScriptAsset.h"
#include "AssetAPI.h"
#include "LoggingFunctions.h"

#include <QList>
#include <QDir>

#include "MemoryLeakCheck.h"

ScriptAsset::~ScriptAsset()
{
    Unload();
}

void ScriptAsset::DoUnload()
{
    scriptContent = "";
    references.clear();
}

bool ScriptAsset::DeserializeFromData(const u8 *data, size_t numBytes, bool /*allowAsynchronous*/)
{
    QByteArray arr((const char *)data, (int)numBytes);
    scriptContent = arr;

    ParseReferences();
    assetAPI->AssetLoadCompleted(Name());
    return true;
}

bool ScriptAsset::SerializeTo(std::vector<u8> &dst, const QString &/*serializationParameters*/) const
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
    sregex_iterator searchEnd;

    // In headless mode we don't want to mark certain asset types as
    // dependencies for the script, as they will fail Load() anyways
    QStringList ignoredAssetTypes;
    if (assetAPI->IsHeadless())
        ignoredAssetTypes << "QtUiFile" << "Texture" << "OgreParticle" << "OgreMaterial" << "Audio";

    // Script asset dependencies are expressed in code comments using lines like "// !ref: http://myserver.com/myasset.png".
    // The asset type can be specified using a comma: "// !ref: http://myserver.com/avatarasset.xml, Avatar".
    regex expression("!ref:\\s*(.*?)(\\s*,\\s*(.*?))?\\s*(\\n|\\r|$)");
    for(sregex_iterator iter(content.begin(), content.end(), expression); iter != searchEnd; ++iter)
    {
        QString regexResult = QString::fromStdString((*iter)[1].str());

        AssetReference ref;
        ref.ref = assetAPI->ResolveAssetRef(Name(), regexResult);
        if ((*iter)[3].matched)
            ref.type = (*iter)[3].str().c_str();

        if (ignoredAssetTypes.contains(assetAPI->GetResourceTypeFromAssetRef(ref.ref)))
            continue;

        // Don't allow including our own ref, will break AssetAPI dependency code to infinite recursion.
        if (Name().compare(regexResult, Qt::CaseSensitive) == 0 || Name().compare(ref.ref, Qt::CaseSensitive) == 0)
        {
            LogWarning("[ScriptAsset]: Script " + Name() + " has a !ref dependency declaration to itself, this is not allowed!");
            continue;
        }

        if (!addedRefs.contains(ref.ref, Qt::CaseInsensitive))
        {
            references.push_back(ref);
            addedRefs << ref.ref;
        }
    }

    expression = regex("engine.IncludeFile\\(\\s*\"\\s*(.*?)\\s*\"\\s*\\)");
    for(sregex_iterator iter(content.begin(), content.end(), expression); iter != searchEnd; ++iter)
    {
        QString regexResult = QString::fromStdString((*iter)[1].str());
        
        // First check if this is a relative ref directly to jsmodules
        // We don't want to add these to the references list as it will request them via asset api
        // with a relative path and it will always fail (as we dont have working file:// schema etc.)
        // The IncludeFile function will take care of relative refs when the script is ran.
        if (QDir::isRelativePath(regexResult) && (regexResult.startsWith("jsmodules") ||
            regexResult.startsWith("/jsmodules") || regexResult.startsWith("./jsmodules")))
            continue;

        // Ask AssetAPI to resolve the ref
        AssetReference ref;
        ref.ref = assetAPI->ResolveAssetRef(Name(), regexResult);

        // Don't allow including our own ref, will break AssetAPI dependency code to infinite recursion.
        if (Name().compare(regexResult, Qt::CaseSensitive) == 0 || Name().compare(ref.ref, Qt::CaseSensitive) == 0)
        {
            LogWarning("[ScriptAsset]: Script " + Name() + " has engine.IncludeFile invocation to itself, this is not allowed!");
            continue;
        }

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
