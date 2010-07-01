// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PythonHandler.h"
#include "WorldBuildingModule.h"

#include <EC_OpenSimPrim.h>

namespace WorldBuilding
{
    PythonHandler::PythonHandler(QObject *parent) :
        QObject(parent)
    {
        connect(this, SIGNAL(HelloFromPython()), SLOT(HelloPython()));
    }

    PythonHandler::~PythonHandler()
    {
    }

    void PythonHandler::HelloPython()
    {
        WorldBuildingModule::LogDebug("Python says hello!");
    }

    void PythonHandler::ObjectSelected(EC_OpenSimPrim *prim)
    {

    }
}