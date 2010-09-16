// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "JavascriptEngine.h"
#include "JavascriptModule.h"

#include "ScriptMetaTypeDefines.h"
#include <QFile>
#include <QUiLoader>

#include "MemoryLeakCheck.h"

JavascriptEngine::JavascriptEngine(const QString &scriptRef):
    engine_(0),
    scriptRef_(scriptRef)
{
    Initialize();
}

JavascriptEngine::~JavascriptEngine()
{
    SAFE_DELETE(engine_);
}

void JavascriptEngine::Reload()
{
    
}

void JavascriptEngine::Unload()
{
    
}

void JavascriptEngine::Run()
{
    QString program = LoadScript();
    //Before we begin to run the script.
    QScriptSyntaxCheckResult syntaxResult = engine_->checkSyntax(program);
    if(syntaxResult.state() != QScriptSyntaxCheckResult::Valid)
    {
        JavascriptModule::LogError("Syntax error: " + syntaxResult.errorMessage().toStdString() + 
                                   " In line:" + QString::number(syntaxResult.errorLineNumber()).toStdString());
        return;
    }

    QScriptValue result = engine_->evaluate(program, scriptRef_);
    if (engine_->hasUncaughtException())
        JavascriptModule::LogError(result.toString().toStdString());
}

void JavascriptEngine::Stop()
{
    engine_->abortEvaluation();
}

void JavascriptEngine::RegisterService(QObject *serviceObject, const QString &name)
{
    QScriptValue scriptValue = engine_->newQObject(serviceObject);
    engine_->globalObject().setProperty(name, scriptValue);
}

QString JavascriptEngine::LoadScript()
{
    QFile scriptFile(scriptRef_);
    scriptFile.open(QIODevice::ReadOnly);
    QString result = scriptFile.readAll();
    scriptFile.close();
    return result;
}

void JavascriptEngine::Initialize()
{
    engine_ = new QScriptEngine();
    if(!engine_)
        return;

    ReqisterInputMetaTypes(engine_);
    ReqisterSceneMetaTypes(engine_);
    ReqisterUiMetaTypes(engine_);
    ReqisterQtMetaTypes(engine_);
}