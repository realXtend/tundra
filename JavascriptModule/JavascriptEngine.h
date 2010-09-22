/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   JavascriptEngine.h
 *  @brief  
 */

#ifndef incl_JavascriptModule_JavascriptEngine_h
#define incl_JavascriptModule_JavascriptEngine_h

#include "IScriptInstance.h"

class QScriptEngine;

class JavascriptEngine: public IScriptInstance
{
public:
    explicit JavascriptEngine(const QString &scriptRef);
    ~JavascriptEngine();

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

private:
    QString LoadScript() const;

    QScriptEngine *engine_;
    QString scriptRef_;
};

#endif