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

        It is recommended to access other modules through service interfaces, it generally
        leads to less trouble. It is however possible to access modules directly, but it
        leads to a direct dependency between the modules. See ModuleManager for more info.

        How modules can access other modules' services? There are two ways:
           - Access the module directly using ModuleManager.
           - Use service manager to query for registered services. Each service is associated
             with a type, and only one type of service can be registered at any one time.
             If two modules attempt to register same type of service, results are undefined.
        
        How module can offer a (new) service to other modules?
           - Create new interface for the service in Interfaces-sub project, subclassing ServiceInterface - class.
           - Include the new .h file in ServiceInterfaces.h
           - Add the service type to Foundation::Service::Type enumeration.
           - Subclass the service class from the new service interface
           - Add the service registering and unregistering to the parent module.
    */
    class ServiceManager
    {
        
    public:
        ServiceManager(Framework *framework);
        ~ServiceManager() {}

        //! register specified service
        /*!
            \note if service of type type is already registered, results are undefined.

            \param type type of the service
            \param service service to register
        */
        void RegisterService(Service::Type type, ServiceInterface *service);

        //! Unregister the specified service. The service should be registered before unregistering.
        void UnregisterService(ServiceInterface *service);

        //! Returns service from service type.
        /*! Throws Core::Exception if the service type is not registered

            \note The pointer may (in theory) invalidate between frames, always reacquire at begin of frame update
            \param type type of the service to return
            \return the service, or NULL if the template parameters doesn't match the service or if the service was not registered
        */
        template <class T>
        __inline T *GetService(Service::Type type)
        {
            ServicesMap::iterator it = services_.find(type);
            if (it == services_.end())
            {
                std::string what("Service type ");
                what += boost::lexical_cast<std::string>(type) + " not registered!";
                Foundation::RootLogDebug(what);

                return NULL;
            }

            return (dynamic_cast<T*>(it->second));
        }

        //! Returns service from service type.
        /*! Throws Core::Exception if the service type is not registered

            \note The pointer may (in theory) invalidate between frames, always reacquire at begin of frame update
            \param type type of the service to return
            \return the service, or NULL if the template parameters doesn't match the service or if the service was not registered
        */
        template <class T>
        __inline const T *GetService(Service::Type type) const
        {
            ServicesMap::const_iterator it = services_.find(type);
            if (it == services_.end())
            {
                std::string what("Service type " + 
                    boost::lexical_cast<std::string>(type) + " not registered!");
                Foundation::RootLogDebug(what);

                return NULL;
            }

            return (dynamic_cast<T*>(it->second));
        }

        //! Returns true if service type is already registered, false otherwise
        bool IsRegistered(Service::Type type) const
        {
            return (services_.find(type) != services_.end());
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

