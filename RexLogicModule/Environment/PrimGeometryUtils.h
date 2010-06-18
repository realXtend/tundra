// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_PrimGeometryUtils_h
#define incl_RexLogicModule_PrimGeometryUtils_h

#include "RexLogicModuleApi.h"

class EC_OpenSimPrim;

namespace Ogre
{
    class ManualObject;
}

namespace RexLogic
{
    //! Generates prim geometry into an Ogre manual object from prim parameters and returns it or 0 if something went wrong
    /*! Note that the same manual object is returned for each call, so you should immediately CommitChanges() into an
        EC_OgreCustomObject before calling CreatePrimGeometry again.
     */
    REXLOGIC_MODULE_API Ogre::ManualObject* CreatePrimGeometry(Foundation::Framework* framework, EC_OpenSimPrim& primitive, bool optimisations_enabled = true);
}

#endif