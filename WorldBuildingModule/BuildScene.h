// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_BuildScene_h
#define incl_WorldBuildingModule_BuildScene_h

#include <QGraphicsScene>

namespace WorldBuilding
{
    class BuildScene : public QGraphicsScene
    {

    Q_OBJECT

    public:
        BuildScene(QObject *parent);
        virtual ~BuildScene();
    };
}

#endif