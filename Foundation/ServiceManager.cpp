// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ServiceManager.h"

namespace Foundation
{
    ServiceManager::ServiceManager(Framework *framework) : framework_(framework)
    {
    }

    void ServiceManager::RegisterService(Core::service_type_t type, ServiceInterface *service)
    {
        assert(service != NULL);
        Foundation::RootLogInfo("Registering service. Service_type: " + boost::lexical_cast<std::string>(type) + ".");

        if (services_.find(type) != services_.end())
        {
            Foundation::RootLogWarning("Service provider already registered!");
            return;
        }
        services_[type] = service;
    }

    void ServiceManager::UnregisterService(ServiceInterface *service)
    {
        assert(service != NULL);
        Foundation::RootLogInfo("Unregistering service.");

        ServicesMap::iterator iter = services_.begin();
        for ( ; iter != services_.end() ; ++iter)
        {
            if (iter->second == service)
            {
                services_.erase(iter);
                return;
            }
        }
        Foundation::RootLogWarning("Unregistering service provider that was not registered!");
    }
}
