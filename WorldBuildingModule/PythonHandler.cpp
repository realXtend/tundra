// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PythonHandler.h"
#include "WorldBuildingModule.h"

#include <EC_OpenSimPrim.h>

namespace WorldBuilding
{
    PythonHandler::PythonHandler(QObject *parent) :
        QObject(parent),
        current_mode_(PythonParams::FREEMOVE)
    {
    }

    PythonHandler::~PythonHandler()
    {
    }

    void PythonHandler::EmitManipulationModeChange(int mode)
    {
        current_mode_ = (PythonParams::ManipulationMode)mode;
        emit ManipulationMode(mode);
    }

    void PythonHandler::EmitEditingActivated(bool activated)
    {
        emit ActivateEditing(activated);
    }

    void PythonHandler::EmitRemoveHightlight()
    {
        emit RemoveHightlight();
    }
}