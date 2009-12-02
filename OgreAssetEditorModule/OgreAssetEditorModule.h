// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreAssetEditorModule.h
 *  @brief  Ogre asset editor module.provides editing and previewing tools for
 *          OGRE assets such as mesh, material & particle scripts.
 */

#ifndef incl_OgreAssetEditorModule_h
#define incl_OgreAssetEditorModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "RexUUID.h"

#include <QMap>
#include <QPair>

class QObject;

namespace Foundation
{
    class EventDataInterface;
}

namespace OgreAssetEditor
{
    class OgreAssetEditorModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        /// Default constructor.
        OgreAssetEditorModule();

        /// Destructor.
        ~OgreAssetEditorModule();

        /// ModuleInterfaceImpl overrides.
        void Load();
        void Unload();
        void Initialize();
        void PostInitialize();
        void Uninitialize();
        void Update(Core::f64 frametime);
        bool HandleEvent(
            Core::event_category_id_t category_id,
            Core::event_id_t event_id,
            Foundation::EventDataInterface* data);

        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        /// Returns type of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_OgreAssetEditor;

        typedef QMap<QPair<RexTypes::RexUUID, Core::request_tag_t>, QObject *> AssetEditorMap;
        typedef QMapIterator<QPair<RexTypes::RexUUID, Core::request_tag_t>, QObject *> AssetEditorMapIter;

    private:
        OgreAssetEditorModule(const OgreAssetEditorModule &);
        void operator=(const OgreAssetEditorModule &);

        /// Event manager pointer.
        Foundation::EventManagerPtr eventManager_;

        /// Inventory event category.
        Core::event_category_id_t inventoryEventCategory_;

        /// Asset event category.
        Core::event_category_id_t assetEventCategory_ ;

        /// Resource event category.
        Core::event_category_id_t resourceEventCategory_;

        /// Asset editors
        AssetEditorMap assetEditors_;
    };
}

#endif
