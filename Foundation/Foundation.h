// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_h
#define incl_Foundation_h

#include "ForwardDefines.h"
#include "Framework.h"
//#include "Application.h"
//#include "Declare_EC.h"
//#include "ComponentManager.h"
//#include "ModuleManager.h"
//#include "ConfigurationManager.h"
//#include "ServiceManager.h"
//#include "EventManager.h"
//#include "ThreadTaskManager.h"
#include "Platform.h"
#include "Profiler.h"

/*! \defgroup Foundation_group Foundation Client Interface
    \copydoc Foundation
*/

//! Foundation is the main helper in your quest to create new or improved functionality for the realXtend viewer.
/*! To use all the services offered by the foundation, simply include "Foundation.h" in your own module.

    - ModuleManager is there to help you create new pluggable modules for the
      viewer. It works mostly transparently so when you create a new module you
      don't need to worry about it, it does most of the worrying for you!
    For creating new modules, see \ref ModuleArchitecture "Module architecture".

    - EventManager helps you to communicate between modules using events.
    See \ref EventSystem "Event system".

    - ServiceManager helps you to communicate between modules when simple events
      do not suffice.
    For using services, see \ref ModuleServices "Services".

    - ComponentManager handles extending scene objects with new functionality.
      When you create a new component for an entity, ComponentManager makes sure
      the component is usable by any module in the viewer, including the ones
      you create yourself.
    For creating new components, see ComponentManager.

    - ThreadTaskManager handles threaded background tasks. For usage information, see
      \ref ThreadTask "Threaded task system"
      
    - ConfigurationManager provides access to name-value pairs defined
      in an external file suitable for defining various settings.

    - Application provides convenient shortcuts for accessing application
      specific configuration options, such as the name of the application.

    - Platform provides platform-specific functionality, such as accessing
      application data folder, or displaying a message box to the user.

    - Framework class is the entry point for the viewer. It contains the main
    loop and is responsible for all the other parts of the foundation.
*/
namespace Foundation {}



#endif


