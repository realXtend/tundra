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
    REXLOGIC_MODULE_API void CreatePrimGeometry(Foundation::Framework* framework, Ogre::ManualObject* object, EC_OpenSimPrim& primitive, bool optimisations_enabled = true);
}

#endif