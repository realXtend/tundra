/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   JavascriptInstance.cpp
 *  @brief  Javascript script instance used wit EC_Script.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "JavascriptInstance.h"
#include "JavascriptModule.h"
#include "ScriptMetaTypeDefines.h"
#include "ScriptCoreTypeDefines.h"
#include "EC_Script.h"
#include "ScriptAsset.h"
#include "IModule.h"
#include "AssetAPI.h"
#include "IAssetProvider.h" //to check if the code was loaded from a local or remote storage

#include "LoggingFunctions.h"

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

void JavascriptInstance::Load()
{
    if (!engine_)
        CreateEngine();

    // Can't specify both a file source and an Asset API source.
    assert(sourceFile.isEmpty() || scriptRefs_.empty());

    // Determine based on code origin whether it can be trusted with system access or not
    if (!scriptRefs_.empty())
    {
        trusted_ = true;
        for (unsigned i = 0; i < scriptRefs_.size(); ++i)
        {
            AssetProviderPtr provider = scriptRefs_[i]->GetAssetProvider();
            if (provider.get())
                if (provider->Name() != "Local")
                    trusted_ = false;
        }
    }
    
    // Local file: trusted
    if (!sourceFile.isEmpty())
    {
        program_ = LoadScript(sourceFile);
        trusted_ = true; //this is a local file directly, right?
    }

    // Do we even have a script to execute?
    if (program_.isEmpty() && scriptRefs_.empty())
    {
        LogError("JavascriptInstance::Load: No script content to load!");
        return;
    }

    bool useAssets = !scriptRefs_.empty();
    unsigned numScripts = useAssets ? scriptRefs_.size() : 1;

    for (unsigned i = 0; i < numScripts; ++i)
    {
        QString scriptSourceFilename = (useAssets ? scriptRefs_[i]->Name() : sourceFile);
        QString &scriptContent = (useAssets ? scriptRefs_[i]->scriptContent : program_);

        QScriptSyntaxCheckResult syntaxResult = engine_->checkSyntax(scriptContent);
        if (syntaxResult.state() != QScriptSyntaxCheckResult::Valid)
        {
            LogError("Syntax error in script " + scriptSourceFilename.toStdString() + "," + QString::number(syntaxResult.errorLineNumber()).toStdString() +
                ": " + syntaxResult.errorMessage().toStdString());

            // Delete our loaded script content (if any exists).
            program_ == "";
        }
    }
}

QString JavascriptInstance::LoadScript(const QString &fileName)
{
    QString filename = fileName.trimmed();

    // First check if the include was supposed to go through the Asset API.
    if (module_)
    {
        ScriptAssetPtr asset = boost::dynamic_pointer_cast<ScriptAsset>(module_->GetFramework()->Asset()->GetAsset(fileName));
        if (asset)
            return asset->scriptContent;
    }

    // Otherwise, treat fileName as a local file to load up.

    QFile scriptFile(filename);
    if (!scriptFile.open(QIODevice::ReadOnly))
    {
        LogError(("JavascriptInstance::LoadScript: Failed to load script from file " + filename + "!").toStdString());
        return "";
    }

    QString result = scriptFile.readAll();
    scriptFile.close();

    QString trimmedResult = result.trimmed();
    if (trimmedResult.isEmpty())
    {
        LogWarning(("JavascriptInstance::LoadScript: Warning Loaded script from file " + filename + ", but the content was empty.").toStdString());
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
    unsigned numScripts = useAssets ? scriptRefs_.size() : 1;
    includedFiles.clear();
    
    for (unsigned i = 0; i < numScripts; ++i)
    {
        QString scriptSourceFilename = (useAssets ? scriptRefs_[i]->Name() : sourceFile);
        QString &scriptContent = (useAssets ? scriptRefs_[i]->scriptContent : program_);

        QScriptValue result = engine_->evaluate(scriptContent, scriptSourceFilename);
        CheckAndPrintException("In run/evaluate: ", result);
    }
    
    evaluated = true;
    emit ScriptEvaluated();
}

void JavascriptInstance::RegisterService(QObject *serviceObject, const QString &name)
{
    if (!engine_)
    {
        LogError("JavascriptInstance::RegisterService: No Qt script engine created when trying to register service to js script instance.");
        return;
    }
    if (!serviceObject)
    {
        LogError("JavascriptInstance::RegisterService: Trying to pass a null service object pointer to RegisterService!");
        return;
    }

    QScriptValue scriptValue = engine_->newQObject(serviceObject);
    engine_->globalObject().setProperty(name, scriptValue);
}

void JavascriptInstance::IncludeFile(const QString &path)
{
    for(uint i = 0; i < includedFiles.size(); ++i)
    {
        if (includedFiles[i].toLower() == path.toLower())
        {
            LogDebug("JavascriptInstance::IncludeFile: Not including already included file " + path.toStdString());
            return;
        }
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
        LogError("JavascriptInstance::IncludeFile: Syntax error in " + path.toStdString() + syntaxResult.errorMessage().toStdString()
            + " In line:" + QString::number(syntaxResult.errorLineNumber()).toStdString());
        return;
    }

    QScriptValue result = engine_->evaluate(script);

    includedFiles.push_back(path);
    
    if (engine_->hasUncaughtException())
        LogError(result.toString().toStdString());
}

void JavascriptInstance::ImportExtension(const QString &scriptExtensionName)
{
    assert(engine_);
    if (!engine_)
    {
        LogWarning(("JavascriptInstance::ImportExtension(" + scriptExtensionName + ") failed, QScriptEngine==null!").toStdString());
        return;
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
        LogWarning("JavascriptInstance::ImportExtension: refusing to load a QtScript plugin for an untrusted instance: " + scriptExtensionName.toStdString());
        return;
    }

    QScriptValue success = engine_->importExtension(scriptExtensionName);
    if (!success.isUndefined()) // Yes, importExtension returns undefinedValue if the import succeeds. http://doc.qt.nokia.com/4.7/qscriptengine.html#importExtension
        LogWarning(std::string("JavascriptInstance::ImportExtension: Failed to load ") + scriptExtensionName.toStdString() + " plugin for QtScript!");
    
    if (!trusted_)
    {
        QScriptValue exposed;
        foreach (const QString &blacktype, qt_class_blacklist)
        {
            exposed = engine_->globalObject().property(blacktype);
            if (exposed.isValid())
            {
                engine_->globalObject().setProperty(blacktype, QScriptValue()); //passing an invalid val removes the property, http://doc.qt.nokia.com/4.6/qscriptvalue.html#setProperty
                //LogInfo("JavascriptInstance::ImportExtension: removed a type from the untrusted context: " + blacktype.toStdString());
            }
        }
    }
}

bool JavascriptInstance::CheckAndPrintException(const QString& message, const QScriptValue& result)
{
    if (engine_->hasUncaughtException())
    {
        LogError(message + result.toString());
        QStringList trace = engine_->uncaughtExceptionBacktrace();
        QStringList::const_iterator it;
        for(it = trace.constBegin(); it != trace.constEnd(); ++it)
            LogError((*it).toLocal8Bit().constData());

        std::stringstream ss;
        int linenum = engine_->uncaughtExceptionLineNumber();
        ss << linenum;
        LogError(ss.str());
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
    LogError(exception.toString().toStdString());

    QStringList trace = engine_->uncaughtExceptionBacktrace();
    QStringList::const_iterator it;
    for(it = trace.constBegin(); it != trace.constEnd(); ++it)
        LogError((*it).toStdString());

    std::stringstream ss;
    int linenum = engine_->uncaughtExceptionLineNumber();
    ss << linenum;
    LogError(ss.str());
}

