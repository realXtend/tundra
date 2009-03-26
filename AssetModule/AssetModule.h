// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AssetModule_h
#define incl_AssetModule_h

#include "ModuleInterface.h"

namespace Foundation
{
    class Framework;
}

namespace Asset
{
    class AssetManager;
    typedef boost::shared_ptr<AssetManager> AssetManagerPtr;
    
    class MODULE_API AssetModule : public Foundation::ModuleInterface_Impl
    {
    public:
        AssetModule();
        virtual ~AssetModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void Uninitialize();

        virtual void Update();

        //! returns framework
        Foundation::Framework *GetFramework() { return framework_; }

        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Asset;

    private:
        //! asset manager
        AssetManagerPtr manager_;
    };
}

#endif
