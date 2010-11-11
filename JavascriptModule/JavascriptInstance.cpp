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
    module_(module)
{
    CreateEngine();
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
    if (!engine_)
        return; // Already stopped/deleted/unloaded.

    DeleteEngine();
}

void JavascriptInstance::Run()
{
    ///\todo Do we want to load script here automatically or should user call Load() explicitily before calling Run()?
    Load();
    if (!program_.isEmpty())
    {
        QScriptValue result = engine_->evaluate(program_, scriptRef_);
        if (engine_->hasUncaughtException())
            LogError(result.toString().toStdString());
    }
}

void JavascriptInstance::Stop()
{
    DeleteEngine();
}

void JavascriptInstance::RegisterService(QObject *serviceObject, const QString &name)
{
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

void JavascriptInstance::IncludeFile(const QString& path)
{
    QFile scriptFile(path);
    if (!scriptFile.open(QIODevice::ReadOnly))
    {
        LogError(("Failed to load script from file " + path + "!").toStdString());
        return;
    }

    QString script = scriptFile.readAll();
    scriptFile.close();

    QScriptContext* context = engine_->currentContext();
    assert(context);
    if (!context)
        return;

    QScriptContext* parent = context->parentContext();
    if(parent != 0 )
    {
        context->setActivationObject(context->parentContext()->activationObject());
        context->setThisObject(context->parentContext()->thisObject());
    }
    else
    {
        LogError("Did not find parent contex for script");
        return;
    }

    QScriptSyntaxCheckResult syntaxResult = engine_->checkSyntax(script);
    if(syntaxResult.state() != QScriptSyntaxCheckResult::Valid)
    {
        LogError("Syntax error in " + path.toStdString() + syntaxResult.errorMessage().toStdString()
            + " In line:" + QString::number(syntaxResult.errorLineNumber()).toStdString());
        return;
    }

    QScriptValue result = engine_->evaluate(script);

    if (engine_->hasUncaughtException())
        LogError(result.toString().toStdString());
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

