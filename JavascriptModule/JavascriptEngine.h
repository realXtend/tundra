/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   JavascriptEngine.h
 *  @brief  
 */

#ifndef incl_JavascriptModule_JavascriptEngine_h
#define incl_JavascriptModule_JavascriptEngine_h

#include "IScriptInstance.h"

#include <QtScript>

class JavascriptEngine: public IScriptInstance
{
    Q_OBJECT

public:
    explicit JavascriptEngine(const QString &scriptRef);
    virtual ~JavascriptEngine();

    //! Overload from IScriptInstance
    void Reload();

    //! Overload from IScriptInstance
    void Unload();

    //! Overload from IScriptInstance
    void Run();

    //! Overload from IScriptInstance
    void Stop();

    //! Register new service to java script engine.
    void RegisterService(QObject *serviceObject, const QString &name);

    //void SetPrototype(QScriptable *prototype, );

    QScriptEngine* GetEngine() const { return engine_; }

public slots:
    /// Loads a given script in engine. This is function which can be used to 
    /// create a property as you could include js-files. 
    /// @param path is relative path from bin/ to file. Example jsmodules/apitest/myscript.js
    void IncludeFile(const QString& file);

private slots:
    void OnSignalHandlerException(const QScriptValue& exception);
    
private:
    QString LoadScript() const;

    QScriptEngine *engine_;
    QString scriptRef_;
};

#endif