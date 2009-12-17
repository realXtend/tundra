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

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

namespace Foundation
{
    class EventDataInterface;
}

namespace OgreAssetEditor
{
    class MaterialWizard;

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
        void Update(f64 frametime);
        bool HandleEvent(
            event_category_id_t category_id,
            event_id_t event_id,
            Foundation::EventDataInterface* data);

        MODULE_LOGGING_FUNCTIONS

        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        /// Returns type of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_OgreAssetEditor;

        typedef QMap<QPair<RexUUID, request_tag_t>, QObject *> AssetEditorMap;
        typedef QMapIterator<QPair<RexUUID, request_tag_t>, QObject *> AssetEditorMapIter;

    private:
        OgreAssetEditorModule(const OgreAssetEditorModule &);
        void operator=(const OgreAssetEditorModule &);

        /// Event manager pointer.
        Foundation::EventManagerPtr eventManager_;

        /// Inventory event category.
        event_category_id_t inventoryEventCategory_;

        /// Asset event category.
        event_category_id_t assetEventCategory_ ;

        /// Resource event category.
        event_category_id_t resourceEventCategory_;

        /// Asset editors
        AssetEditorMap assetEditors_;

        /// Material wizard.
        MaterialWizard *materialWizard_;
    };
}

#endif
