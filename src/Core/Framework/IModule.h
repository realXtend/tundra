/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   IModule.h
 *  @brief  Interface for module objects. When creating new modules, inherit from this class.
 */

#pragma once

// Disable C4251 warnings in MSVC: 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif

#include "CoreTypes.h"
#include "FrameworkFwd.h"
#include <boost/enable_shared_from_this.hpp>

/// Interface for modules. When creating new modules, inherit from this class.
/** See @ref ModuleArchitecture for details. */
class IModule : public QObject, public boost::enable_shared_from_this<IModule>
{
    Q_OBJECT

public:
    /// Constructor.
    /** @param name Module name. */
    explicit IModule(const QString &name);
    virtual ~IModule();

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
    virtual void Update(f64 frametime) {}

    /// Returns the name of the module.
    const QString &Name() const { return name; }

    /// Returns parent framework.
    Framework *GetFramework() const;

protected:
    Framework *framework_; ///< Parent framework

private:
    Q_DISABLE_COPY(IModule)
    friend class Framework;

    /// Only for internal use.
    void SetFramework(Framework *framework) { framework_ = framework; assert (framework_); }

    const QString name; ///< Name of the module
};

#ifdef _MSC_VER
#pragma warning( pop )
///\todo Try to find a way not disable C4275 warnings for good
// Disable C4275 warnings in MSVC for good: non � DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
#pragma warning( disable : 4275 )
#endif
