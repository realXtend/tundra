// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_PythonHanler_h
#define incl_WorldBuildingModule_PythonHanler_h

#include <QObject>

class EC_OpenSimPrim;

namespace WorldBuilding
{
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
        PythonHandler(QObject *parent);
        virtual ~PythonHandler();

    public slots:
        void EmitManipulationModeChange(int mode);
        void EmitEditingActivated(bool activated);
        void EmitRemoveHightlight();
        void SetRotateValues(int x, int y, int z);
        void EmitRotateChange(int x, int y, int z);
        void EmitObjectAction(PythonParams::ObjectAction action);

        void PassWidget(const QString &type, QWidget *widget);

        PythonParams::ManipulationMode GetCurrentManipulationMode() { return current_mode_; }     

    private:
        PythonParams::ManipulationMode current_mode_;

    signals:
        void HelloFromPython();
        void ManipulationMode(int);
        void ActivateEditing(bool);
        void RemoveHightlight();
        void RotateValuesChangedToUi(int x, int y, int z);
        void RotateValuesChangedToNetwork(int x, int y, int z);
        void CreateObject();
        void DuplicateObject();
        void DeleteObject();

        void WidgetRecieved(const QString &type, QWidget *widget);
    };
}

#endif