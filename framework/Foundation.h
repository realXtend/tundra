// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __inc_Foundation_h__
#define __inc_Foundation_h__

#include "ForwardDefines.h"
#include "Framework.h"
#include "Declare_EC.h"
#include "ComponentManager.h"
#include "ModuleManager.h"
#include "EntityManager.h"
#include "Entity.h"
#include "ConfigurationManager.h"
#include "ServiceManager.h"

//! Foundation is the main helper in your quest to create new or improved functionality for the realXtend viewer.
/*! To use all the services offered by the foundation, simply include "Foundation.h" in your own module.


    - ModuleManager is there to help you create new pluggable modules for the
    viewer. It works mostly transparently so when you create a new module you
    don't need to worry about it, it does most of the worrying for you!

    For creating new modules, see ModuleManager.


    - ComponentManager handles extending scene objects with new functionality.
    When you create a new component for an entity, ComponentManager makes sure
    the component is usable by any module in the viewer, including the ones
    you create yourself.

    For creating new components, see ComponentManager.

    - ConfigurationManager provides access to immutable name-value pairs
    defined in an external file.


    - Framework class is the entry point for the viewer. It contains the main
    loop and is responsible for all the other parts of the foundation.
*/
namespace Foundation {}

#endif


