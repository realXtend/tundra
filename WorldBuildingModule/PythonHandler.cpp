// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PythonHandler.h"
#include "WorldBuildingModule.h"

#include <EC_OpenSimPrim.h>

namespace WorldBuilding
{
    PythonHandler::PythonHandler(QObject *parent) :
        QObject(parent),
        current_mode_(PythonParams::MANIP_FREEMOVE)
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

    void PythonHandler::EmitRotateChange(int x, int y, int z)
    {
        emit RotateValuesChangedToNetwork(x, y, z);
    }

    void PythonHandler::SetRotateValues(int x, int y, int z)
    {
        emit RotateValuesChangedToUi(x, y, z);
    }

    void PythonHandler::EmitObjectAction(PythonParams::ObjectAction action)
    {
        switch (action)
        {
            case PythonParams::OBJ_NEW:
                emit CreateObject();
                break;
            case PythonParams::OBJ_CLONE:
                emit DuplicateObject();
                break;
            case PythonParams::OBJ_DELETE:
                emit DeleteObject();
                break;
            default:
                break;
        }
    }
}