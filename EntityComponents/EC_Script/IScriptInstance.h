/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IScriptInterface.h
 *  @brief  Interface for different script instances, e.g. Javascript of Python.
 */

#ifndef incl_EC_Clone_IScriptInstance_h
#define incl_EC_Clone_IScriptInstance_h

/// Interface for different script instances, e.g. Javascript of Python.
class IScriptInstance
{
public:
    /// Default constuctor.
    IScriptInstance() {}

    /// Destructor.
    virtual ~IScriptInstance() {}

    /// Loads/reload this script instance.
    virtual bool Reload() = 0;

    /// Unloads this script instance.
    virtual bool Unload() = 0;
};

#endif
