// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_WorldBuildingServiceInterface_h
#define incl_Interfaces_WorldBuildingServiceInterface_h

#include "IService.h"
#include <QObject>

namespace Foundation
{   
    class WorldBuildingServiceInterface : public QObject, public IService
    {

    Q_OBJECT

    public:
        WorldBuildingServiceInterface() {}
        virtual ~WorldBuildingServiceInterface() {}

    public slots:
        virtual QObject *GetPythonHandler() const = 0;

    signals:
        void SetOverrideTime(int);
        void OverrideServerTime(int);
    };

    typedef boost::shared_ptr<Foundation::WorldBuildingServiceInterface> WorldBuildingServicePtr;
}

#endif