// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UpdateModuleMac_h
#define incl_UpdateModuleMac_h

#include "IModule.h"
#include <QObject>

#include "CocoaInitializer.h"
#include "SparkleAutoUpdater.h"

class UpdateModule : public IModule
{

Q_OBJECT

public:
    UpdateModule();
    virtual ~UpdateModule();

    void Initialize();

public slots:
    void RunUpdater(QString paramter);

private:
	AutoUpdater* updater_;
};

#endif
