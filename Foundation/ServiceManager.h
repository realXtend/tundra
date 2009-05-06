// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ServiceManager_h
#define incl_Foundation_ServiceManager_h

#include "ServiceInterfaces.h"

namespace Foundation
{
    class Framework;

    //! Provides access to services provided by modules.
    /*! See \ref ModuleServices "Module services" -page for details.

       \ingroup Foundation_group
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
        void RegisterService(Core::service_type_t type, ServiceInterface *service);

        //! Unregister the specified service. The service should be registered before unregistering.
        void UnregisterService(ServiceInterface *service);

        //! Returns service from service type.
        /*! Throws Core::Exception if the service type is not registered

            \note The pointer may (in theory) invalidate between frames, always reacquire at begin of frame update
            \param type type of the service to return
            \return the service, or NULL if the template parameters doesn't match the service or if the service was not registered
        */
        template <class T>
        __inline T *GetService(Core::service_type_t type)
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
        __inline const T *GetService(Core::service_type_t type) const
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
        bool IsRegistered(Core::service_type_t type) const
        {
            return (services_.find(type) != services_.end());
        }
    private:
        typedef std::map<Core::service_type_t, ServiceInterface*> ServicesMap;
        
        //! parent framework
        Framework *framework_;

        //! Contains all registered services
        ServicesMap services_;
    };
}

#endif

