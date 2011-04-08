// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AssetModule_h
#define incl_AssetModule_h

#include "IModule.h"


#include "ConsoleCommandServiceInterface.h"
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

    /// Asset module.
    class ASSET_MODULE_API AssetModule : public IModule
    {
    public:
        AssetModule();
        virtual ~AssetModule();

        virtual void Initialize();
        virtual void PostInitialize();

        /// callback for console command
        Console::CommandResult ConsoleRequestAsset(const StringVector &params);

        Console::CommandResult AddHttpStorage(const StringVector &params);

        /// returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return type_name_static_; }

    private:
        void ProcessCommandLineOptions();

        /// Type name of the module.
        static std::string type_name_static_;
    };
}

#endif
