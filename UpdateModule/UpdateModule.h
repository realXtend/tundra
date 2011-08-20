// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UpdateModule_h
#define incl_UpdateModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"

#include <QObject>

namespace Update
{
    class UpdateModule : public QObject, public IModule
    {

    Q_OBJECT

    static const std::string &NameStatic();

    public:
        UpdateModule();
        virtual ~UpdateModule();

        void Load();
        void PostInitialize();
        MODULE_LOGGING_FUNCTIONS

    public slots:
        void RunUpdater(QString parameter = "/checknow");

    private:
        QString updateExecutable;
        QString updateConfig;
    };
}
#endif
