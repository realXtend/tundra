// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ServiceManager_h
#define incl_Foundation_ServiceManager_h

#include "ServiceInterface.h"
#include "CoreTypes.h"

#include <map>

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
        void RegisterService(service_type_t type, const ServiceWeakPtr &service);

        //! Unregister the specified service. The service should be registered before unregistering.
        void UnregisterService(const ServiceWeakPtr &service);

        //! Returns un-casted service from service type
        ServiceWeakPtr GetService(service_type_t type);

        //! Returns service from service type.
        /*! Returns empty weak pointer if the service is not registered

            \note The pointer may (in theory) invalidate between frames, always reacquire at begin of frame update
            \param type type of the service to return
            \return the service, or empty weak pointer if the template parameters doesn't match the service or if the service was not registered
        */
        template <class T>
        __inline boost::weak_ptr<T> GetService(service_type_t type)
        {
            ServicesMap::iterator it = services_.find(type);
            if (it == services_.end())
            {
                //std::string what("Service type ");
                //what += boost::lexical_cast<std::string>(type) + " not registered!";
                //Foundation::RootLogDebug(what);

                return boost::weak_ptr<T>();
            }

            return boost::dynamic_pointer_cast<T>(it->second.lock());
        }

        //! Returns service from service type.
        /*! 

            \note The pointer may (in theory) invalidate between frames, always reacquire at begin of frame update
            \param type type of the service to return
            \return the service, or empty weak pointer if the template parameters doesn't match the service or if the service was not registered
        */
        template <class T>
        __inline const boost::weak_ptr<T> GetService(service_type_t type) const
        {
            ServicesMap::const_iterator it = services_.find(type);
            if (it == services_.end())
            {
                //std::string what("Service type " + 
                //    boost::lexical_cast<std::string>(type) + " not registered!");
                //Foundation::RootLogDebug(what);

                return boost::weak_ptr<T>();
            }

            return boost::dynamic_pointer_cast<T>(it->second.lock());
        }

        //! Returns true if service type is already registered, false otherwise
        bool IsRegistered(service_type_t type) const
        {
            return (services_.find(type) != services_.end());
        }
    private:
        typedef std::map<service_type_t, ServiceWeakPtr> ServicesMap;
        typedef std::map<service_type_t, int> ServicesUsageMap;
        
        //! parent framework
        Framework *framework_;

        //! Contains all registered services
        ServicesMap services_;

        //! Number of shared ptr uses when registering a service, for debug purposes only!
        ServicesUsageMap services_usage_;
    };
}

#endif

