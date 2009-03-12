// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ServiceManager_h
#define incl_Foundation_ServiceManager_h

#include "ServiceInterfaces.h"

namespace Foundation
{
    class Framework;

    //! Provides access to services provided by modules.
    /*! All modules offering services to other modules must register with this manager.
        The services must be registered when module is initialized and unregistered
        when they are uninitialized. Each module is responsible for managing the actual
        classes behind the service.

        How modules can access other modules' services? There are two ways:
           - Access the module directly using ModuleManager.
           - Use service manager to query for registered services. Each service is associated
             with a type, and only one type of service can be registered at any one time.
             If two modules attempt to register same type of service, results are undefined.
        
        How module can offer a (new) service to other modules?
           - Create new interface for the service in Interfaces-sub project, subclassing ServiceInterface - class.
           - Add the .h file to ServiceInterfaces.h
           - Add the service type to Foundation::Service::Type enumeration.
           - Subclass the service class from the service interface
           - Add the service registering and unregistering to the parent module.
    */
    class ServiceManager
    {
        
    public:
        ServiceManager(Framework *framework);
        ~ServiceManager() {}

        //! register specified service
        void RegisterService(Service::Type type, ServiceInterface *service);

        //! Unregister the specified service. The service should be registered before unregistering.
        void UnregisterService(ServiceInterface *service);

        //! Returns service from service type.
        /*!
            \note The pointer may invalidate between frames, always reacquire at begin of frame update
        */
        template <class T>
        __inline T *GetService(Service::Type type)
        {
            ServicesMap::iterator it = services_.find(type);
            if (it == services_.end())
            {
                std::string what("Service type ");
                what += boost::lexical_cast<std::string>(type) + " not registered!";
                throw Core::Exception(what.c_str());
            }

            return (static_cast<T*>(it->second));
        }

        //! Returns service from service type.
        /*!
            \note The pointer may invalidate between frames, always reacquire at begin of frame update
        */
        template <class T>
        __inline const T *GetService(Service::Type type) const
        {
            ServicesMap::const_iterator it = services_.find(type);
            if (it == services_.end())
            {
                std::string what("Service type " + boost::lexical_cast<std::string>(type) + " not registered!");
                throw Core::Exception(what.c_str());
            }

            return (static_cast<T*>(it->second));
        }
    private:
        typedef std::map<Service::Type, ServiceInterface*> ServicesMap;
        
        //! parent framework
        Framework *framework_;

        //! Contains all registered services
        ServicesMap services_;
    };
}

#endif

