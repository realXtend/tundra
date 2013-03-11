/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   IModule.h
    @brief  Interface for module objects. When creating new modules, inherit from this class. */

#pragma once

#include "TundraCoreApi.h"
#include "CoreDefines.h"
#include "CoreTypes.h"
#include "FrameworkFwd.h"

#include <QObject>

/// Interface for modules. When creating new modules, inherit from this class.
/** See @ref ModuleArchitecture for details. */
class TUNDRACORE_API IModule : public QObject, public enable_shared_from_this<IModule>
{
    Q_OBJECT

public:
    /// Constructor.
    /** @param moduleName Module name. */
    explicit IModule(const QString &moduleName) : name(moduleName), framework_(0) {}
    virtual ~IModule() {}

    /// Called when module is loaded into memory.
    /** Override in your own module. Do not call.
        Components and asset types exposed by the module should be registered here using SceneAPI and AssetAPI respectively. */
    virtual void Load() {}

    /// Called when module is taken in use.
    /** Override in your own module. Do not call. */
    virtual void Initialize() {}

    /// Called when module is removed from use.
    /** Override in your own module. Do not call. */
    virtual void Uninitialize() {}

    /// Called when module is unloaded from memory.
    /** Override in your own module. Do not call. */
    virtual void Unload() {}

    /// Synchronized update for the module
    /** Override in your own module if you want to perform synchronized update. Do not call.
        @param frametime elapsed time in seconds since last frame */
    virtual void Update(f64 UNUSED_PARAM(frametime)) {}

    /// Returns the name of the module.
    const QString &Name() const { return name; }

    /// Returns parent framework.
    Framework *GetFramework() const { return framework_; }

protected:
    Framework *framework_; ///< Parent framework

private:
    Q_DISABLE_COPY(IModule)
    friend class Framework;

    /// Only for internal use.
    void SetFramework(Framework *framework) { framework_ = framework; assert (framework_); }

    const QString name; ///< Name of the module
};
