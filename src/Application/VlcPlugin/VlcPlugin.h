// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "FrameworkFwd.h"
#include "VlcFwd.h"

#include "IModule.h"

class VlcPlugin : public IModule
{

Q_OBJECT

public:
    /// Constructor
    VlcPlugin();

    // IModule override
    void Load();

    // IModule override
    void Uninitialize();

    // IModule override
    void Unload();

};