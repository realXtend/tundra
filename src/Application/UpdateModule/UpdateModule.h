// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UpdateModule_h
#define incl_UpdateModule_h

#include "IModule.h"
#include <QObject>

class UpdateModule : public IModule
{

Q_OBJECT

public:
    UpdateModule();
    virtual ~UpdateModule();

    void PostInitialize();

public slots:
    void RunUpdater(QString parameter = "/checknow");

private:
    QString updateExecutable;
    QString updateConfig;
};

#endif
