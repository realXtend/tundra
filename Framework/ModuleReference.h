/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file ModuleReference.h
 *  @brief This file defines types that are used when directly accessing modules using the ModuleManager query interface.
 */

#ifndef incl_Foundation_ModuleReference_h
#define incl_Foundation_ModuleReference_h

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class IModule;

typedef boost::shared_ptr<IModule> ModuleSharedPtr;
typedef boost::weak_ptr<IModule> ModuleWeakPtr;

#endif
