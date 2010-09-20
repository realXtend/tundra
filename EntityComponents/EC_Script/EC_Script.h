// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Script_EC_Script_h
#define incl_EC_Script_EC_Script_h

#include "IComponent.h"
#include "Declare_EC.h"

class IScriptInstance;

class EC_Script: public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_Script)

public:
    /// Destructor.
    ~EC_Script();

    /// ComponentInterface override. This component is serializable.
    virtual bool IsSerializable() const { return true; }

    /// Type of the script as string (js/py)
    Attribute<QString> type;

    /// Is the script run as soon as the script reference is set/loaded.
    Attribute<bool> runOnLoad;

    /// Reference to a script file.
    Attribute<QString> scriptRef;

    /** Sets new script instance. Unloads and deletes possible already existing script instance.
        @param instance Script instance.
        @note Takes ownership of the script instace.
    */
    void SetScriptInstance(IScriptInstance *instance);

    /// Returns the current script instance.
    IScriptInstance *GetScriptInstance() const { return scriptInstance_; }

public slots:
    /// Runs the script instance.
    void Run();

    /** This is an overloaded function.
        @param name Name of the script. The script is ran only if the script name matches.
    */
    void Run(const QString &name);

    /// Stops the script instance.
    void Stop();

    /** This is an overloaded function.
        @param name Name of the script. The script is ran only if the script name matches.
    */
    void Stop(const QString &name);

signals:
    /** Emitted when script reference changes.
        @newRef New script reference.
    */
    void ScriptRefChanged(const QString &newRef);

private slots:
    /**
        @param attribute Attribute that changed.
        @param change Change type.
    */
    void HandleAttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    /// Registers the actions this component provides when parent entity is set.
    void RegisterActions();

private:
    /** Constuctor.
        @param module Declaring module.
    */
    explicit EC_Script(IModule *module);

    /// Script instance.
    IScriptInstance *scriptInstance_;

    /// Last reference. We're keeping track so that we don't send multiple changed signals.
    QString lastRef_;
};

#endif
