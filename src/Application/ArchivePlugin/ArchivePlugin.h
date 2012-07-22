// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IModule.h"
#include "AssetFwd.h"

class ArchivePlugin : public IModule
{

Q_OBJECT

public:
    ArchivePlugin();
    ~ArchivePlugin();

    /// IModule override.
    void Initialize();

    /// IModule override.
    void Uninitialize();
};
