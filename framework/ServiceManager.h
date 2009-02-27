// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __inc_Foundation_ServiceManager_h__
#define __inc_Foundation_ServiceManager_h__

#include "ServiceInterfaces.h"

namespace Foundation
{
    class Framework;

    enum ServiceType { Service_Renderer, Service_Physics, Service_Gui, Service_Script };

    //! Manages services. All systems offering a service must register with this manager
    class ServiceManager
    {
        
    public:
        ServiceManager(Framework *framework);
        ~ServiceManager() {}

        //! register renderer service
        void registerService(ServiceType type, ServiceInterface *service);

        //! Unregister the specified service. The service should be registered before unregistering.
        void unregisterService(ServiceInterface *service);

        //! Returns service of type type. Do not use directly, use functions from Framework class instead.
        /*! 
            \note Thou shalt not use this function directly, lest beasts from deepest pit taketh thee.
        */
        template <class T>
        T *_getService(ServiceType type)
        {
            assert (mServices.find(type) != mServices.end());

            return (static_cast<T*>(mServices[type]));
        }
    private:
        typedef std::map<ServiceType, ServiceInterface*> ServicesMap;
        
        Framework *mFramework;
        ServicesMap mServices;
    };
}

#endif

