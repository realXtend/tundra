// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ServiceManager.h"


namespace Foundation
{
    ServiceManager::ServiceManager(Framework *framework) : framework_(framework)
    {
    }

    void ServiceManager::RegisterService(Service::Type type, ServiceInterface *service)
    {
        assert(service != NULL);

        if (services_.find(type) != services_.end())
        {
            //! \todo how to handle, replace old service provider, throw exception, ignore?
            LOGWARNING("Service provider already registered!");
            //throw Core::Exception("Service provider already registered!");
            return;
        }
        services_[type] = service;
    }

    void ServiceManager::UnregisterService(ServiceInterface *service)
    {
        assert(service != NULL);

        ServicesMap::iterator iter = services_.begin();
        for ( ; iter != services_.end() ; ++iter)
        {
            if (iter->second == service)
            {
                services_.erase(iter);
                return;
            }
        }
    }
}
