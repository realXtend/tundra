// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AssetModule_h
#define incl_AssetModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"

#include <QObject>
#include "ConsoleCommandUtils.h"

#include "IAssetProvider.h"
#include "AssetModuleApi.h"

namespace Asset
{
    /** \defgroup AssetModuleClient AssetModule Client Interface
        This page lists the public interface of the AssetModule,
        which consists of implementing Foundation::AssetServiceInterface and
        Foundation::AssetInterface

        For details on how to use the public interface, see \ref AssetModule "Using the asset module".
    */

    //! Asset module.
    class ASSET_MODULE_API AssetModule : public QObject, public IModule
    {
        Q_OBJECT

    public:
        AssetModule();
        virtual ~AssetModule();

        virtual void Initialize();
        virtual void PostInitialize();

        MODULE_LOGGING_FUNCTIONS

        //! callback for console command
        ConsoleCommandResult ConsoleRequestAsset(const StringVector &params);

        ConsoleCommandResult AddHttpStorage(const StringVector &params);

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return type_name_static_; }

    public slots:
        /// Loads from all the registered local storages all assets that have the given suffix.
        void LoadAllLocalAssetsWithSuffix(const QString &suffix);

    private:
        void ProcessCommandLineOptions();

        //! Type name of the module.
        static std::string type_name_static_;
    };
}

#endif
