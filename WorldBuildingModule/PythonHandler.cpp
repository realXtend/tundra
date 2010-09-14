// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PythonHandler.h"
#include "WorldBuildingModule.h"
#include "BuildSceneManager.h"

#include <EC_OpenSimPrim.h>

namespace WorldBuilding
{
    PythonHandler::PythonHandler(BuildSceneManager *parent) :
        QObject(parent),
        build_manager_(parent),
        current_mode_(PythonParams::MANIP_FREEMOVE)
    {
    }

    PythonHandler::~PythonHandler()
    {
    }

    void PythonHandler::CleanPyWidgets()
    {
        build_manager_->CleanPyWidgets();
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
        emit RotateValuesToNetwork(x, y, z);
    }

    void PythonHandler::EmitScaleChange(double x, double y, double z)
    {
        emit ScaleValuesToNetwork(x, y, z);
    }

    void PythonHandler::EmitPosChange(double x, double y, double z)
    {
        emit PosValuesToNetwork(x, y, z);
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

    void PythonHandler::SetRotateValues(int x, int y, int z)
    {
        emit RotateValuesToUi(x, y, z);
    }

    void PythonHandler::SetScaleValues(double x, double y, double z)
    {
        emit ScaleValuesToUi(x, y, z);
    }

    void PythonHandler::SetPosValues(double x, double y, double z)
    {
        emit PosValuesToUi(x, y, z);
    }

    void PythonHandler::PassWidget(const QString &type, QWidget *widget)
    {
        emit WidgetRecieved(type, widget);
    }
}