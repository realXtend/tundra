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
            FREEMOVE = 0,
            MOVE,
            SCALE,
            ROTATE
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
        
        PythonParams::ManipulationMode GetCurrentManipulationMode() { return current_mode_; }

    private:
        PythonParams::ManipulationMode current_mode_;

    signals:
        void HelloFromPython();
        void ManipulationMode(int);
        void ActivateEditing(bool);
        void RemoveHightlight();
    };
}

#endif