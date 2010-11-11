/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   JavascriptInstance.h
 *  @brief  Javascript script instance used wit EC_Script.
 */

#ifndef incl_JavascriptModule_JavascriptInstance_h
#define incl_JavascriptModule_JavascriptInstance_h

#include "IScriptInstance.h"
#include "ForwardDefines.h"

#include <QtScript>

class JavascriptModule;

/// Javascript script instance used wit EC_Script.
class JavascriptInstance : public IScriptInstance
{
    Q_OBJECT

public:
    ///
    /** @param scriptRef
        @param module Javascript module
    */
    JavascriptInstance(const QString &scriptRef, JavascriptModule *module);

    ///
    virtual ~JavascriptInstance();

    //! Overload from IScriptInstance
    void Load();

    //! Overload from IScriptInstance
    void Unload();

    //! Overload from IScriptInstance
    void Run();

    //! Register new service to java script engine.
    void RegisterService(QObject *serviceObject, const QString &name);

    //void SetPrototype(QScriptable *prototype, );
    QScriptEngine* GetEngine() const { return engine_; }

    /// Sets owner (EC_Script) component.
    /** @param owner Owner component.
    */
    void SetOwnerComponent(ComponentPtr owner) { owner_ = owner; }

public slots:
    /// Loads a given script in engine. This function can be used to create a property as you could include js-files.
    /** @param path is relative path from bin/ to file. Example jsmodules/apitest/myscript.js
    */
    void IncludeFile(const QString& file);

private:
    /// Loads script from the current script reference location.
    QString LoadScript() const;

    /// Creates new script context/engine.
    void CreateEngine();

    /// Deletes script context/engine.
    void DeleteEngine();

    QScriptEngine *engine_; ///< Qt script engine.
    QString scriptRef_; ///< Reference to a script file.
    QString program_; ///< Program loaded from script file.
    ComponentWeakPtr owner_; ///< Owner (EC_Script) component, if existing.
    JavascriptModule *module_; ///< Javascript module.
    bool evaluated; ///< Has the script program been evaluated.

private slots:
    void OnSignalHandlerException(const QScriptValue& exception);
};

#endif
