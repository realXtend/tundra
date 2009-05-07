// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ServiceManager.h"

namespace Foundation
{
    ServiceManager::ServiceManager(Framework *framework) : framework_(framework)
    {
    }

    void ServiceManager::RegisterService(Core::service_type_t type, const ServiceWeakPtr &service)
    {
        assert(service.expired() == false);

        Foundation::RootLogInfo("Registering service. Service_type: " + boost::lexical_cast<std::string>(type) + ".");

        if (services_.find(type) != services_.end())
        {
            Foundation::RootLogWarning("Service provider already registered!");
            return;
        }
        services_[type] = service;
    }

    void ServiceManager::UnregisterService(const ServiceWeakPtr &service)
    {
        assert(service.expired() == false);

        Foundation::RootLogInfo("Unregistering service.");

        ServicePtr upped_service = service.lock();

        ServicesMap::iterator iter = services_.begin();
        for ( ; iter != services_.end() ; ++iter)
        {
            assert (iter->second.expired() == false);
            if (iter->second.lock().get() == upped_service.get())
            {
                services_.erase(iter);
                return;
            }
        }
        //! \todo Any possibility to have the service type here nicely, so it can be included with the log warning?
        Foundation::RootLogWarning("Unregistering service provider type that was not registered!");
    }
}
