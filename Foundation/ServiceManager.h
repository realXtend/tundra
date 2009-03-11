// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_ServiceManager_h
#define incl_Foundation_ServiceManager_h

#include "ServiceInterfaces.h"

namespace Foundation
{
    class Framework;

    //! Manages services. All modules offering a service must register with this manager
    class ServiceManager
    {
        
    public:
        ServiceManager(Framework *framework);
        ~ServiceManager() {}

        //! register specified service
        void RegisterService(Service::Type type, ServiceInterface *service);

        //! Unregister the specified service. The service should be registered before unregistering.
        void UnregisterService(ServiceInterface *service);

        //! Returns service from service type.
        /*!
            \note The pointer may invalidate between frames, always reacquire at begin of frame update
        */
        template <class T>
        __inline T *GetService(Service::Type type)
        {
            ServicesMap::iterator it = services_.find(type);
            if (it == services_.end())
            {
                std::string what("Service type ");
                what += boost::lexical_cast<std::string>(type) + " not registered!";
                throw Core::Exception(what.c_str());
            }

            return (static_cast<T*>(it->second));
        }

        //! Returns service from service type.
        /*!
            \note The pointer may invalidate between frames, always reacquire at begin of frame update
        */
        template <class T>
        __inline const T *GetService(Service::Type type) const
        {
            ServicesMap::const_iterator it = services_.find(type);
            if (it == services_.end())
            {
                std::string what("Service type " + boost::lexical_cast<std::string>(type) + " not registered!");
                throw Core::Exception(what.c_str());
            }

            return (static_cast<T*>(it->second));
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

