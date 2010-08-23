// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ServiceInterface_h
#define incl_Interfaces_ServiceInterface_h

#include "CoreModuleApi.h"
#include "boost/shared_ptr.hpp"

namespace Foundation
{
    //! Common interface for all services
    /*!
        \ingroup Foundation_group
        \ingroup Services_group
    */
    class MODULE_API ServiceInterface
    {
    public:
        ServiceInterface() {}
        virtual ~ServiceInterface() {}
    };

    typedef boost::shared_ptr<ServiceInterface> ServicePtr;
    typedef boost::weak_ptr<ServiceInterface> ServiceWeakPtr;
    
    namespace Service
    {
        //! Service types. When creating a new core service, add it here.
        /*! If you can't or don't want to alter the framework, an arbitrary
            int can be used as service type. For details, see \ref creating_services.

            \ingroup Services_group

            \todo Get rid of this as we did with module enumeration. Low priority.
        */
        enum Type 
        {
            ST_Renderer,
            ST_Physics,
            ST_Gui,
            ST_WorldLogic,
            ST_PythonScripting,
            ST_JavascriptScripting,
            ST_Console,
            ST_ConsoleCommand,
            ST_Asset,
            ST_Texture,
            ST_Sound,
            ST_Input,
            ST_Communications,
            ST_UiSettings,
            ST_Player,
            ST_WorldBuilding,
            ST_Login,
            ST_Unknown
        };
    }
}

#endif
