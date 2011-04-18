// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Scene_IComponentRegistrar_h
#define incl_Scene_IComponentRegistrar_h

#include <boost/shared_ptr.hpp>
#include <vector>

class IModule;
class Framework;

/// Base class for all objects that register their entity-component factory services to the framework.
/**
    \note It is not necessary to use this class if DECLARE_EC -macro is used.
*/
class IComponentRegistrar
{
public:
    IComponentRegistrar()  {}
    virtual ~IComponentRegistrar() {}

    /// registers component to the framework
    virtual void Register(Framework *framework, IModule* module) = 0;

    /// unregisters component to framework
    virtual void Unregister(Framework *framework) = 0;
};

typedef boost::shared_ptr<IComponentRegistrar> ComponentRegistrarPtr;
typedef std::vector<ComponentRegistrarPtr> RegistrarVector;

#endif
