/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   JavascriptInstance.cpp
 *  @brief  Javascript script instance used wit EC_Script.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "JavascriptInstance.h"
#include "JavascriptModule.h"
#include "ScriptMetaTypeDefines.h"
#include "NaaliCoreTypeDefines.h"
#include "EC_Script.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("JavascriptInstance")

#include <QFile>

#include "MemoryLeakCheck.h"

JavascriptInstance::JavascriptInstance(const QString &scriptRef, JavascriptModule *module):
    engine_(0),
    scriptRef_(scriptRef),
    module_(module),
    evaluated(false)
{
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

    program_ = LoadScript();
    QScriptSyntaxCheckResult syntaxResult = engine_->checkSyntax(program_);
    if(syntaxResult.state() != QScriptSyntaxCheckResult::Valid)
    {
        LogError("Syntax error in " + scriptRef_.toStdString() + syntaxResult.errorMessage().toStdString()
            + " In line:" + QString::number(syntaxResult.errorLineNumber()).toStdString());
        program_ = "";
    }
}

void JavascriptInstance::Unload()
{
    DeleteEngine();
}

void JavascriptInstance::Run()
{
    if (evaluated)
    {
        Unload();
        Load();
    }

    if (!program_.isEmpty())
    {
        included_files_.clear();
        QScriptValue result = engine_->evaluate(program_, scriptRef_);
        if (engine_->hasUncaughtException())
            LogError(result.toString().toStdString());

        evaluated = true;
    }
}

void JavascriptInstance::RegisterService(QObject *serviceObject, const QString &name)
{
    if (!engine_)
    {
        LogError("No Qt script engine created when trying to register service to js script instance.");
        return;
    }

    QScriptValue scriptValue = engine_->newQObject(serviceObject);
    engine_->globalObject().setProperty(name, scriptValue);
}

QString JavascriptInstance::LoadScript() const
{
    QString filename = scriptRef_.trimmed();
    ///\todo Now all strings are evaluated as being relative filenames to the current working directory.
    ///Scripts are all loaded locally. Support here fetching from the asset store, i.e. file://, or knet://, or http://.
    QFile scriptFile(filename);
    if (!scriptFile.open(QIODevice::ReadOnly))
    {
        LogError(("Failed to load script from file " + filename + "!").toStdString());
        return "";
    }

    QString result = scriptFile.readAll();
    scriptFile.close();
    return result;
}

void JavascriptInstance::IncludeFile(const QString &path)
{
    for (uint i = 0; i < included_files_.size(); ++i)
    {
        if (included_files_[i].toLower() == path.toLower())
        {
            LogDebug("JavascriptInstance::IncludeFile: Not including already included file " + path.toStdString());
            return;
        }
    }
    
    QFile scriptFile(path);
    if (!scriptFile.open(QIODevice::ReadOnly))
    {
        LogError(("JavascriptInstance::IncludeFile: Failed to load script from file \"" + path + "\"!").toStdString());
        return;
    }

    QString script = scriptFile.readAll();
    scriptFile.close();

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

    included_files_.push_back(path);
    
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

    QScriptValue success = engine_->importExtension(scriptExtensionName);
    if (!success.isUndefined()) // Yes, importExtension returns undefinedValue if the import succeeds. http://doc.qt.nokia.com/4.7/qscriptengine.html#importExtension
        LogWarning(std::string("JavascriptInstance::ImportExtension: Failed to load ") + scriptExtensionName.toStdString() + " plugin for QtScript!");
}

void JavascriptInstance::CreateEngine()
{
    if (engine_)
        DeleteEngine();
    engine_ = new QScriptEngine;
    connect(engine_, SIGNAL(signalHandlerException(const QScriptValue &)), SLOT(OnSignalHandlerException(const QScriptValue &)));

    ExposeQtMetaTypes(engine_);
    ExposeNaaliCoreTypes(engine_);
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
    QScriptValue destructor = engine_->globalObject().property("OnScriptDestroyed");
    if (!destructor.isUndefined())
        destructor.call();
    SAFE_DELETE(engine_);
}

void JavascriptInstance::OnSignalHandlerException(const QScriptValue& exception)
{
    LogError(exception.toString().toStdString());
}

