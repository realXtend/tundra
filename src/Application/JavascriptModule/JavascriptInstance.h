/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   JavascriptInstance.h
 *  @brief  Javascript script instance used wit EC_Script.
 */

#pragma once

#include "IScriptInstance.h"
#include "SceneFwd.h"
#include "AssetFwd.h"
#include "JavascriptFwd.h"

//#include <QtScript>
//#ifndef QT_NO_SCRIPTTOOLS
//#include <QScriptEngineDebugger>
//#endif

class JavascriptModule;

/// Javascript script instance used wit EC_Script.
class JavascriptInstance : public IScriptInstance
{
    Q_OBJECT

public:
    /// Creates script engine for this script instance and loads the script but doesn't run it yet.
    /** @param scriptRef Script asset reference.
        @param module Javascript module. */
    JavascriptInstance(const QString &fileName, JavascriptModule *module);

    /// Creates script engine for this script instance and loads the script but doesn't run it yet.
    /** @param scriptRef Script asset reference.
        @param module Javascript module. */
    JavascriptInstance(ScriptAssetPtr scriptRef, JavascriptModule *module);

    /// Creates script engine for this script instance and loads the script but doesn't run it yet.
    /** @param scriptRefs Script asset references.
        @param module Javascript module. */
    JavascriptInstance(const std::vector<ScriptAssetPtr>& scriptRefs, JavascriptModule *module);

    /// Destroys script engine created for this script instance.
    virtual ~JavascriptInstance();

    /// IScriptInstance override.
    void Load();

    /// IScriptInstance override.
    void Unload();

    /// IScriptInstance override.
    void Run();

    /// Register new service to java script engine.
    /** @return Returns if the registration was successful. */
    bool RegisterService(QObject *serviceObject, const QString &name);

    //void SetPrototype(QScriptable *prototype, );
    QScriptEngine* Engine() const { return engine_; }

    /// Sets owner (EC_Script) component.
    /** @param owner Owner component. */
    void SetOwner(const ComponentPtr &owner) { owner_ = owner; }

    /// Return owner component
    ComponentWeakPtr Owner() const { return owner_; }

public slots:
    /// Loads a given script in engine. This function can be used to create a property as you could include js-files.
    /** Multiple inclusion of same file is prevented. (by using simple string compare)
        @param path is relative path from bin/ to file. Example jsmodules/apitest/myscript.js */
    void IncludeFile(const QString &file);

    /// Imports the given QtScript extension plugin into the current script instance. Returns true if successful.
    bool ImportExtension(const QString &scriptExtensionName);

    /// Return whether has been evaluated
    virtual bool IsEvaluated() const { return evaluated; }

    /// Dumps engine information into a string. Used for debugging/profiling.
    virtual QMap<QString, uint> DumpEngineInformation();
    
    /// Check and print error if the engine has an uncaught exception
    bool CheckAndPrintException(const QString& message, const QScriptValue& result);

signals:
    /// The scripts have been run. This is the trigger to create script objects as necessary
    void ScriptEvaluated();

    /// The script engine is about to unload. This is the trigger to delete script objects as necessary
    void ScriptUnloading();

private:
    /// Creates new script context/engine.
    void CreateEngine();

    /// Deletes script context/engine.
    void DeleteEngine();

    QString LoadScript(const QString &fileName);
    
    void GetObjectInformation(const QScriptValue &object, QSet<qint64> &ids, uint &valueCount, uint &objectCount, uint &nullCount, uint &numberCount, 
        uint &boolCount, uint &stringCount, uint &arrayCount, uint &funcCount, uint &qobjCount, uint &qobjMethodCount);
        
    QScriptEngine *engine_; ///< Qt script engine.

    // The script content for a JavascriptInstance is loaded either using the Asset API or 
    // using an absolute path name from the local file system.

    /// If the script content is loaded using the Asset API, this points to the asset that is loaded.
    std::vector<ScriptAssetPtr> scriptRefs_; 

    /// If the script content is loaded directly from local file, this points to the actual script content.  
    QString program_;
    
    /// Specifies the absolute path of the source file where the script is loaded from, if the content is directly loaded from file.
    QString sourceFile;

    /// Current script name that is loaded into this instance.
    QString currentScriptName;

    ComponentWeakPtr owner_; ///< Owner (EC_Script) component, if existing.
    JavascriptModule *module_; ///< Javascript module.
    bool evaluated; ///< Has the script program been evaluated.
    //QScriptEngineDebugger *debugger_;

    /// Already included files for preventing multi-inclusion
    std::vector<QString> includedFiles;

private slots:
    void OnSignalHandlerException(const QScriptValue& exception);
};

