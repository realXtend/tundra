// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IModule.h"


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

    public slots:
        void RunUpdater(QString parameter = "/checknow");

    private:
        QString updateExecutable;
        QString updateConfig;
    };
}
