// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AssetModule_h
#define incl_AssetModule_h

#include "IModule.h"

#include <QObject>
#include "ConsoleCommandUtils.h"

#include "IAssetProvider.h"
#include "AssetModuleApi.h"
#include "ConsoleCommand.h"

namespace Asset
{
    /** \defgroup AssetModuleClient AssetModule Client Interface
        This page lists the public interface of the AssetModule,
        which consists of implementing AssetServiceInterface and
        AssetInterface

        For details on how to use the public interface, see \ref AssetModule "Using the asset module".
    */

    /// Asset module.
    class ASSET_MODULE_API AssetModule : public QObject, public IModule
    {
        Q_OBJECT

    public:
        AssetModule();
        virtual ~AssetModule();

        virtual void Initialize();
        virtual void PostInitialize();

        /// callback for console command
        ConsoleCommandResult ConsoleRequestAsset(const StringVector &params);

        ConsoleCommandResult AddHttpStorage(const StringVector &params);

    public slots:
        /// Loads from all the registered local storages all assets that have the given suffix.
        void LoadAllLocalAssetsWithSuffix(const QString &suffix);

    private:
        void ProcessCommandLineOptions();
    };
}

#endif
