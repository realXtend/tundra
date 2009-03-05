// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ServiceManager_h
#define incl_Foundation_ServiceManager_h

#include "ServiceInterfaces.h"

namespace Foundation
{
    class Framework;

    //! Manages services. All systems offering a service must register with this manager
    class ServiceManager
    {
        
    public:
        ServiceManager(Framework *framework);
        ~ServiceManager() {}

        //! register specified service
        void registerService(Service::Type type, ServiceInterface *service);

        //! Unregister the specified service. The service should be registered before unregistering.
        void unregisterService(ServiceInterface *service);

        //! Returns service from service type.
        template <class T>
        T *getService(Service::Type type)
        {
            ServicesMap::iterator it = mServices.find(type);
            if (it == mServices.end())
            {
                std::string what("Service type ");
                what += boost::lexical_cast<std::string>(type) + " not registered!";
                throw Core::Exception(what.c_str());
            }

            return (static_cast<T*>(it->second));
        }

        //! Returns service from service type.
        template <class T>
        const T *getService(Service::Type type) const
        {
            ServicesMap::const_iterator it = mServices.find(type);
            if (it == mServices.end())
            {
                std::string what("Service type ") + boost::lexical_cast<std::string>(type) + " not registered!";
                throw Core::Exception(what.c_str());
            }

            return (static_cast<T*>(it->second));
        }
    private:
        typedef std::map<Service::Type, ServiceInterface*> ServicesMap;
        
        //! parent framework
        Framework *mFramework;

        //! Contains all registered services
        ServicesMap mServices;
    };
}

#endif

