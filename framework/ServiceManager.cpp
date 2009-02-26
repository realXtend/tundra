#include "StableHeaders.h"
#include "ServiceManager.h"


namespace Foundation
{
    ServiceManager::ServiceManager(Framework *framework) : mFramework(framework)
    {
    }

    void ServiceManager::registerService(ServiceType type, ServiceInterface *service)
    {
        assert(service != NULL);

        if (mServices.find(type) != mServices.end())
        {
            //! \todo how to handle, replace old service provider, throw exception, ignore?
            throw std::exception("Service provider already registered!");
        }
        mServices[type] = service;
    }

    void ServiceManager::unregisterService(ServiceInterface *service)
    {
        assert(service != NULL);

        ServicesMap::iterator iter = mServices.begin();
        for ( ; iter != mServices.end() ; ++iter)
        {
            if (iter->second == service)
            {
                mServices.erase(iter);
                return;
            }
        }
    }
}
