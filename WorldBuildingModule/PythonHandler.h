// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_PythonHanler_h
#define incl_WorldBuildingModule_PythonHanler_h

#include <QObject>

class EC_OpenSimPrim;

namespace WorldBuilding
{
    class PythonHandler : public QObject
    {
        
    Q_OBJECT
    
    public:
        PythonHandler(QObject *parent);
        virtual ~PythonHandler();

    public slots:
        void HelloPython();
        void ObjectSelected(EC_OpenSimPrim *prim);

    signals:
        void HelloFromPython();
    };
}

#endif