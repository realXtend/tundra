// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ServiceManager.h"


namespace Foundation
{
    ServiceManager::ServiceManager(Framework *framework) : mFramework(framework)
    {
    }

    void ServiceManager::registerService(Service::Type type, ServiceInterface *service)
    {
        assert(service != NULL);

        if (mServices.find(type) != mServices.end())
        {
            //! \todo how to handle, replace old service provider, throw exception, ignore?
            LOGWARNING("Service provider already registered!");
            //throw Core::Exception("Service provider already registered!");
            return;
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
