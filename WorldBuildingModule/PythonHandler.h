// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_PythonHanler_h
#define incl_WorldBuildingModule_PythonHanler_h

#include <QObject>

class EC_OpenSimPrim;

namespace WorldBuilding
{
    class BuildSceneManager;
    namespace PythonParams
    {
        enum ManipulationMode
        {
            MANIP_FREEMOVE = 0,
            MANIP_MOVE,
            MANIP_SCALE,
            MANIP_ROTATE,
            MANIP_NONE
        };

        enum ObjectAction
        {
            OBJ_None = 0,
            OBJ_NEW,
            OBJ_CLONE,
            OBJ_DELETE
        };
    }

    class PythonHandler : public QObject
    {
        
    Q_OBJECT

    public:
        PythonHandler(BuildSceneManager *parent);
        virtual ~PythonHandler();

    public slots:
        void CleanPyWidgets();
        void EmitManipulationModeChange(int mode);
        void EmitEditingActivated(bool activated);
        void EmitRemoveHightlight();
        void SetRotateValues(int x, int y, int z);
        void SetScaleValues(double x, double y, double z);
        void SetPosValues(double x, double y, double z);
        void EmitRotateChange(int x, int y, int z);
        void EmitScaleChange(double x, double y, double z);
        void EmitPosChange(double x, double y, double z);
        void EmitObjectAction(PythonParams::ObjectAction action);

        void PassWidget(const QString &type, QWidget *widget);

        PythonParams::ManipulationMode GetCurrentManipulationMode() { return current_mode_; }     

    private:
        PythonParams::ManipulationMode current_mode_;
        BuildSceneManager *build_manager_;

    signals:
        void HelloFromPython();
        void ManipulationMode(int);
        void ActivateEditing(bool);
        void RemoveHightlight();
        void RotateValuesToUi(int x, int y, int z);
        void ScaleValuesToUi(double x, double y, double z);
        void PosValuesToUi(double x, double y, double z);
        void RotateValuesToNetwork(int x, int y, int z);
        void ScaleValuesToNetwork(double x, double y, double z);
        void PosValuesToNetwork(double x, double y, double z);
        void CreateObject();
        void DuplicateObject();
        void DeleteObject();

        void WidgetRecieved(const QString &type, QWidget *widget);
    };
}

#endif