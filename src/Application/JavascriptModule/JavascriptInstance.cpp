/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   JavascriptInstance.cpp
 *  @brief  Javascript script instance used wit EC_Script.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "JavascriptInstance.h"
#include "JavascriptModule.h"
#include "ScriptMetaTypeDefines.h"
#include "ScriptCoreTypeDefines.h"
#include "EC_Script.h"
#include "ScriptAsset.h"
#include "AssetAPI.h"
#include "Application.h"
#include "IAssetStorage.h"
#include "LoggingFunctions.h"
#include "Profiler.h"

#include <QDir>
#include <QFile>
#include <sstream>

#include <QScriptClass>
Q_DECLARE_METATYPE(QScriptClass*)

//#ifndef QT_NO_SCRIPTTOOLS
//#include <QScriptEngineDebugger>
//#endif

#include "MemoryLeakCheck.h"

JavascriptInstance::JavascriptInstance(const QString &fileName, JavascriptModule *module) :
    engine_(0),
    sourceFile(fileName),
    module_(module),
    evaluated(false)
{
    CreateEngine();
    Load();
}

JavascriptInstance::JavascriptInstance(ScriptAssetPtr scriptRef, JavascriptModule *module) :
    engine_(0),
    module_(module),
    evaluated(false)
{
    // Make sure we do not push null or empty script assets as sources
    if (scriptRef && !scriptRef->scriptContent.isEmpty())
        scriptRefs_.push_back(scriptRef);
    
    CreateEngine();
    Load();
}

JavascriptInstance::JavascriptInstance(const std::vector<ScriptAssetPtr>& scriptRefs, JavascriptModule *module) :
    engine_(0),
    module_(module),
    evaluated(false)
{
    // Make sure we do not push null or empty script assets as sources
    for (unsigned i = 0; i < scriptRefs.size(); ++i)
        if (scriptRefs[i] && !scriptRefs[i]->scriptContent.isEmpty()) scriptRefs_.push_back(scriptRefs[i]);
    
    CreateEngine();
    Load();
}


JavascriptInstance::~JavascriptInstance()
{
    DeleteEngine();
}

QMap<QString, uint> JavascriptInstance::DumpEngineInformation()
{
    if (!engine_)
        return QMap<QString, uint>();
    
    QSet<qint64> ids;
    uint valueCount = 0;
    uint objectCount = 0;
    uint nullCount = 0;
    uint numberCount = 0;
    uint boolCount = 0;
    uint stringCount = 0;
    uint arrayCount = 0;
    uint funcCount = 0;
    uint qobjCount = 0;
    uint qobjMethodCount = 0;   

    GetObjectInformation(engine_->globalObject(), ids, valueCount, objectCount, nullCount, numberCount, boolCount, stringCount, arrayCount, funcCount, qobjCount, qobjMethodCount);

    QMap<QString, uint> dump;
    dump["QScriptValues"] = valueCount;
    dump["Objects"] = objectCount;
    dump["Functions"] = funcCount;
    dump["QObjects"] = qobjCount;
    dump["QObject methods"] = qobjMethodCount;
    dump["Numbers"] = numberCount;
    dump["Booleans"] = boolCount;
    dump["Strings"] = stringCount;
    dump["Arrays"] = arrayCount;
    dump["Is null"] = nullCount;
    return dump;
}   

void JavascriptInstance::GetObjectInformation(const QScriptValue &object, QSet<qint64> &ids, uint &valueCount, uint &objectCount, uint &nullCount, uint &numberCount, 
    uint &boolCount, uint &stringCount, uint &arrayCount, uint &funcCount, uint &qobjCount, uint &qobjMethodCount)
{
    if (!ids.contains(object.objectId()))       
        ids << object.objectId();
    
    QScriptValueIterator iter(object);
    while(iter.hasNext()) 
    {
        iter.next();
        QScriptValue v = iter.value();

        if (ids.contains(v.objectId()))
            continue;
        ids << v.objectId();
        
        valueCount++;
        if (v.isNull())
            nullCount++;

        if (v.isNumber())
            numberCount++;
        else if (v.isBool())
            boolCount++;
        else if (v.isString())
            stringCount++;
        else if (v.isArray())
            arrayCount++;
        else if (v.isFunction())
            funcCount++;
        else if (v.isQObject())
            qobjCount++;
        
        if (v.isObject())
            objectCount++;

        if (v.isQMetaObject())
            qobjMethodCount += v.toQMetaObject()->methodCount();
        
        // Recurse
        if ((v.isObject() || v.isArray()) && !v.isFunction() && !v.isString() && !v.isNumber() && !v.isBool() && !v.isQObject() && !v.isQMetaObject())
            GetObjectInformation(v, ids, valueCount, objectCount, nullCount, numberCount, boolCount, stringCount, arrayCount, funcCount, qobjCount, qobjMethodCount);
    }
}

void JavascriptInstance::Load()
{
    PROFILE(JSInstance_Load);
    if (!engine_)
        CreateEngine();

    if (sourceFile.isEmpty() && scriptRefs_.empty())
    {
        LogError("JavascriptInstance::Load: No script content to load!");
        return;
    }
    // Can't specify both a file source and an Asset API source.
    if (!sourceFile.isEmpty() && !scriptRefs_.empty())
    {
        LogError("JavascriptInstance::Load: Cannot specify both an local input source file and a list of script refs to load!");
        return;
    }

    bool useAssetAPI = !scriptRefs_.empty();
    size_t numScripts = useAssetAPI ? scriptRefs_.size() : 1;

    // Determine based on code origin whether it can be trusted with system access or not
    if (useAssetAPI)
    {
        trusted_ = true;
        for(unsigned i = 0; i < scriptRefs_.size(); ++i)
            trusted_ = trusted_ && scriptRefs_[i]->IsTrusted();
    }
    else // Local file: always trusted.
    {
        program_ = LoadScript(sourceFile);
        trusted_ = true; // This is a file on the local filesystem. We are making an assumption nobody can inject untrusted code here.
        // Actually, we are assuming the attacker does not know the absolute location of the asset cache locally here, since if he makes
        // the client to load a script into local cache, he could use this code path to automatically load that unsafe script from cache, and make it trusted. -jj.
    }

    // Check the validity of the syntax in the input.
    for (size_t i = 0; i < numScripts; ++i)
    {
        QString scriptSourceFilename = (useAssetAPI ? scriptRefs_[i]->Name() : sourceFile);
        QString &scriptContent = (useAssetAPI ? scriptRefs_[i]->scriptContent : program_);

        QScriptSyntaxCheckResult syntaxResult = engine_->checkSyntax(scriptContent);
        if (syntaxResult.state() != QScriptSyntaxCheckResult::Valid)
        {
            LogError("Syntax error in script " + scriptSourceFilename + "," + QString::number(syntaxResult.errorLineNumber()) +
                ": " + syntaxResult.errorMessage());

            // Delete our loaded script content (if any exists).
            program_ == "";
        }
    }
}

QString JavascriptInstance::LoadScript(const QString &fileName)
{
    PROFILE(JSInstance_LoadScript);
    QString filename = fileName.trimmed();

    // First check if the include was supposed to go through the Asset API.
    if (module_)
    {
        ScriptAssetPtr asset = dynamic_pointer_cast<ScriptAsset>(module_->GetFramework()->Asset()->GetAsset(fileName));
        if (asset)
            return asset->scriptContent;
    }

    /// @bug When including other scripts from startup scripts the only way to include is with relative paths.
    /// As you cannot use !rel: ref in startup scripts (loaded without EC_Script) so you cannot use local:// refs either. 
    /// You have to do engine.IncludeFile("lib/class.js") etc. and this below code needs to find the file whatever the working dir is, a plain QFile::open() wont cut it!

    // Otherwise, treat fileName as a local file to load up.
    // Check install dir and the clean rel path.
    QString pathToFile = "";
    QDir jsPluginDir(QDir::fromNativeSeparators(Application::InstallationDirectory()) + "jsmodules");
    if (jsPluginDir.exists(filename))
        pathToFile = jsPluginDir.filePath(filename);
    else if (QFile::exists(filename))
        pathToFile = filename;
    if (pathToFile.isEmpty())
    {
        LogError("JavascriptInstance::LoadScript: Failed to load script from file " + filename + "!");
        return "";
    }

    QFile scriptFile(pathToFile);
    if (!scriptFile.open(QIODevice::ReadOnly))
    {
        LogError("JavascriptInstance::LoadScript: Failed to load script from file " + filename + "!");
        return "";
    }

    QString result = scriptFile.readAll();
    scriptFile.close();

    QString trimmedResult = result.trimmed();
    if (trimmedResult.isEmpty())
    {
        LogWarning("JavascriptInstance::LoadScript: Warning Loaded script from file " + filename + ", but the content was empty.");
        return "";
    }
    return result;
}

void JavascriptInstance::Unload()
{
    DeleteEngine();
}

void JavascriptInstance::Run()
{
    PROFILE(JSInstance_Run);
    // Need to have either absolute file path source or an Asset API source.
    if (scriptRefs_.empty() && program_.isEmpty())
    {
        LogError("JavascriptInstance::Run: Cannot run, no script reference loaded.");
        return;
    }

    // Can't specify both a file source and an Asset API source.
    assert(sourceFile.isEmpty() || scriptRefs_.empty());

    // If we've already evaluated this script once before, create a new script engine to run it again, or otherwise
    // the effects would stack (we'd possibly register into signals twice, or other odd side effects).
    // We never allow a script to be run twice in this kind of "stacking" manner.
    if (evaluated)
    {
        Unload();
        Load();
    }

    if (!engine_)
    {
        LogError("JavascriptInstance::Run: Cannot run, script engine not loaded.");
        return;
    }

    // If no script specified at all, we'll have to abort.
    if (program_.isEmpty() && scriptRefs_.empty())
        return;

    bool useAssets = !scriptRefs_.empty();
    size_t numScripts = useAssets ? scriptRefs_.size() : 1;
    includedFiles.clear();
    
    for (size_t i = 0; i < numScripts; ++i)
    {
        PROFILE(JSInstance_Evaluate);
        QString scriptSourceFilename = (useAssets ? scriptRefs_[i]->Name() : sourceFile);
        QString &scriptContent = (useAssets ? scriptRefs_[i]->scriptContent : program_);

        QScriptValue result = engine_->evaluate(scriptContent, scriptSourceFilename);
        CheckAndPrintException("In run/evaluate: ", result);
    }
    
    evaluated = true;
    emit ScriptEvaluated();
}

bool JavascriptInstance::RegisterService(QObject *serviceObject, const QString &name)
{
    if (!engine_)
    {
        LogError("JavascriptInstance::RegisterService: No Qt script engine created when trying to register service to js script instance.");
        return false;
    }
    if (!serviceObject)
    {
        LogError("JavascriptInstance::RegisterService: Trying to pass a null service object pointer to RegisterService!");
        return false;
    }

    QScriptValue scriptValue = engine_->newQObject(serviceObject);
    engine_->globalObject().setProperty(name, scriptValue);
    return true;
}

void JavascriptInstance::IncludeFile(const QString &path)
{
    for(uint i = 0; i < includedFiles.size(); ++i)
        if (includedFiles[i].toLower() == path.toLower())
        {
            LogDebug("JavascriptInstance::IncludeFile: Not including already included file " + path);
            return;
        }

    QString script = LoadScript(path);

    QScriptContext *context = engine_->currentContext();
    assert(context);
    if (!context)
    {
        LogError("JavascriptInstance::IncludeFile: QScriptEngine::currentContext() returned null!");
        return;
    }

    QScriptContext *parent = context->parentContext();
    if (!parent)
    {
        LogError("JavascriptInstance::IncludeFile: QScriptEngine::parentContext() returned null!");
        return;
    }

    context->setActivationObject(context->parentContext()->activationObject());
    context->setThisObject(context->parentContext()->thisObject());

    QScriptSyntaxCheckResult syntaxResult = engine_->checkSyntax(script);
    if(syntaxResult.state() != QScriptSyntaxCheckResult::Valid)
    {
        LogError("JavascriptInstance::IncludeFile: Syntax error in " + path + ". " + syntaxResult.errorMessage() +
            " In line:" + QString::number(syntaxResult.errorLineNumber()));
        return;
    }

    QScriptValue result = engine_->evaluate(script, path);

    includedFiles.push_back(path);
    
    if (engine_->hasUncaughtException())
        LogError(result.toString());
}

bool JavascriptInstance::ImportExtension(const QString &scriptExtensionName)
{
    // Currently QtScriptGenerator extensions are not supported on Android. Attempting to import is a fatal error for the script engine,
    // so bypass for now 
#ifdef ANDROID
    LogWarning("JavascriptInstance::ImportExtension(" + scriptExtensionName + ") failed, script extensions not yet supported on Android");
    return false;
#endif

    assert(engine_);
    if (!engine_)
    {
        LogWarning("JavascriptInstance::ImportExtension(" + scriptExtensionName + ") failed, QScriptEngine == null!");
        return false;
    }

    QStringList qt_extension_whitelist;
    QStringList qt_class_blacklist;

    /// Allowed extension imports
    qt_extension_whitelist << "qt.core" << "qt.gui" << "qt.xml" << "qt.xmlpatterns" << "qt.opengl" << "qt.webkit";

    /// qt.core and qt.gui: Classes that may be harmful to your system from untrusted scripts
    qt_class_blacklist << "QLibrary" << "QPluginLoader" << "QProcess"               // process and library access
                       << "QFile" << "QDir" << "QFileSystemModel" << "QDirModel"    // file system access
                       << "QFileDialog" << "QFileSystemWatcher" << "QFileInfo" 
                       << "QFileOpenEvent" << "QFileSystemModel"
                       << "QClipboard" << "QDesktopServices";                       // "system" access
    
    /// qt.webkit: Initial blacklist, enabling some of these can be discussed. 
    /// Availble classes: QWebView, QGraphicsWebView, QWebPage, QWebFrame
    qt_class_blacklist << "QWebDatabase" << "QWebElement" << "QWebElementCollection" << "QWebHistory" << "QWebHistoryInterface" << "QWebHistoryItem"
                       << "QWebHitTestResult" << "QWebInspector" << "QWebPluginFactory" << "QWebSecurityOrigin" << "QWebSettings"; 

    if (!trusted_ && !qt_extension_whitelist.contains(scriptExtensionName, Qt::CaseInsensitive))
    {
        LogWarning("JavascriptInstance::ImportExtension: refusing to load a QtScript plugin for an untrusted instance: " + scriptExtensionName);
        return false;
    }

    bool ret = true;

    QScriptValue success = engine_->importExtension(scriptExtensionName);
    if (!success.isUndefined()) // Yes, importExtension returns undefinedValue if the import succeeds. http://doc.qt.nokia.com/4.7/qscriptengine.html#importExtension
    {
        LogWarning("JavascriptInstance::ImportExtension: Failed to load " + scriptExtensionName + " plugin for QtScript!");
        ret = false;
    }

    if (!trusted_)
    {
        QScriptValue exposed;
        foreach (const QString &blacktype, qt_class_blacklist)
        {
            exposed = engine_->globalObject().property(blacktype);
            if (exposed.isValid())
            {
                engine_->globalObject().setProperty(blacktype, QScriptValue()); //passing an invalid val removes the property, http://doc.qt.nokia.com/4.6/qscriptvalue.html#setProperty
                //LogInfo("JavascriptInstance::ImportExtension: removed a type from the untrusted context: " + blacktype);
            }
        }
    }

    return ret;
}

bool JavascriptInstance::CheckAndPrintException(const QString& message, const QScriptValue& result)
{
    if (engine_->hasUncaughtException())
    {
        LogError(message + result.toString());
        foreach(const QString &error, engine_->uncaughtExceptionBacktrace())
            LogError(error);
        LogError("Line " + QString::number(engine_->uncaughtExceptionLineNumber()) + ".");
        engine_->clearExceptions();
        return true;
    }
    return false;
}

void JavascriptInstance::CreateEngine()
{
    if (engine_)
        DeleteEngine();
    engine_ = new QScriptEngine;
    connect(engine_, SIGNAL(signalHandlerException(const QScriptValue &)), SLOT(OnSignalHandlerException(const QScriptValue &)));
//#ifndef QT_NO_SCRIPTTOOLS
//    debugger_ = new QScriptEngineDebugger();
//    debugger.attachTo(engine_);
////  debugger_->action(QScriptEngineDebugger::InterruptAction)->trigger();
//#endif
    engine_->installTranslatorFunctions();

    ExposeQtMetaTypes(engine_);
    ExposeCoreTypes(engine_);
    ExposeCoreApiMetaTypes(engine_);

    EC_Script *ec = dynamic_cast<EC_Script *>(owner_.lock().get());
    module_->PrepareScriptInstance(this, ec);
    evaluated = false;
}

void JavascriptInstance::DeleteEngine()
{
    if (!engine_)
        return;

    program_ = "";
    engine_->abortEvaluation();

    // As a convention, we call a function 'OnScriptDestroyed' for each JS script
    // so that they can clean up their data before the script is removed from the object,
    // or when the system is unloading.
    
    emit ScriptUnloading();
    
    QScriptValue destructor = engine_->globalObject().property("OnScriptDestroyed");
    if (!destructor.isUndefined())
    {
        QScriptValue result = destructor.call();
        CheckAndPrintException("In script destructor: ", result);
    }
    
    SAFE_DELETE(engine_);
    //SAFE_DELETE(debugger_);
}

void JavascriptInstance::OnSignalHandlerException(const QScriptValue& exception)
{
    LogError(exception.toString());
    foreach(const QString &error, engine_->uncaughtExceptionBacktrace())
        LogError(error);
    LogError("Line " + QString::number(engine_->uncaughtExceptionLineNumber()) + ".");
}
