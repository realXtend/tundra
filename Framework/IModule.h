/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IModule.h
 *  @brief  Interface for module objects.
 *          See @ref ModuleArchitecture for details.
 */

#ifndef incl_Framework_IModule_h
#define incl_Framework_IModule_h

// Disable C4251 warnings in MSVC: 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif

#include "CoreTypes.h"
#include "FrameworkFwd.h"
#include <boost/enable_shared_from_this.hpp>

/// Interface for modules. When creating new modules, inherit from this class.
/** See @ref ModuleArchitecture for details.
    @ingroup Foundation_group
    @ingroup Module_group
*/
class IModule : public QObject, public boost::enable_shared_from_this<IModule>
{
    Q_OBJECT

public:
    /// Constructor. Creates logger for the module.
    /** @param name Module name.
    */
    explicit IModule(const std::string &name);

    /// Destructor. Destroys logger of the module.
    virtual ~IModule();

    /// Called when module is loaded into memory. Do not trust that framework can be used.
    /** Override in your own module. Do not call.
        Components in the module should be registered here using the SceneApi component type factory registration functions.
        Component is the class of the component.
    */
    virtual void Load() {}

    /// Pre-initialization for the module. Called before modules are initializated.
    /// Only override if you need. Do not call.
    virtual void PreInitialize() {}

    /// Initializes the module. Called when module is taken in use.
    /// Override in your own module. Do not call.
    virtual void Initialize() {}

    /// Post-initialization for the module. At this point Initialize() has been called for all enabled modules.
    /// Only override if you need. Do not call.
    virtual void PostInitialize() {}

    /** Uninitialize the module. Called when module is removed from use
        Override in your own module. Do not call.
    */
    virtual void Uninitialize() {}

    /** Called when module is unloaded from memory. Do not trust that framework can be used.
        Override in your own module. Do not call.
    */
    virtual void Unload() {}

    /** Synchronized update for the module
        Override in your own module if you want to perform synchronized update. Do not call.
        @param frametime elapsed time in seconds since last frame
    */
    virtual void Update(f64 frametime) {}

    /// Returns the name of the module. Each module also has a static accessor for the name, it's needed by the logger.
    const std::string &Name() const { return name; }

    /// Returns parent framework.
    Framework *GetFramework() const;

protected:
    Framework *framework_; ///< Parent framework

private:
    // Modules are noncopyable.
    IModule(const IModule &);
    void operator=(const IModule &);

    /// Only for internal use.
    void SetFramework(Framework *framework) { framework_ = framework; assert (framework_); }

    /// name of the module
    const std::string name;

    friend class Framework;
};

#ifdef _MSC_VER
#pragma warning( pop )
///\todo Try to find a way not disable C4275 warnings for good
// Disable C4275 warnings in MSVC for good: non � DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
#pragma warning( disable : 4275 )
#endif

#endif
