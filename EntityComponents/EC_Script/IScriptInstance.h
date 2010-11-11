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
};

#endif
