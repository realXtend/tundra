/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IScriptInterface.h
 *  @brief  Interface for different script instances, e.g. Javascript of Python.
 */

#ifndef incl_EC_Script_IScriptInstance_h
#define incl_EC_Script_IScriptInstance_h

#include <QObject>

/// Interface for different script instances, e.g. Javascript or Python.
class IScriptInstance : public QObject
{
    Q_OBJECT
public:
    /// Default constuctor.
    IScriptInstance() {}

    /// Destructor.
    virtual ~IScriptInstance() {}

    /// Loads this script instance.
    virtual void Load() = 0;

    /// Unloads this script instance.
    virtual void Unload() = 0;

    /// Starts this script instance.
    virtual void Run() = 0;

protected:
    //! Whether this instance executed trusted code or not. 
    /** By default everything loaded remotely (with e.g. http) is untrusted,
        and not exposed anything with system access.
        With qt/javascript means that can not load qt dlls to get qt networking etc.,
        and with python loading remote code is not allowed at all (cpython always has system access). */
    bool trusted_;

};

#endif
