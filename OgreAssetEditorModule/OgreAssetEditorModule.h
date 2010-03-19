/**
 * For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreAssetEditorModule.h
 *  @brief  OgreAssetEditorModule.provides editing and previewing tools for
 *          OGRE assets such as meshes and material scripts.
 */

#ifndef incl_OgreAssetEditorModule_h
#define incl_OgreAssetEditorModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

#include <QObject>

namespace Foundation
{
    class EventDataInterface;
}

namespace UiServices
{
    class UiModule;
}

namespace Naali
{
    class MaterialWizard;
    class EditorManager;

    class OgreAssetEditorModule : public QObject, public Foundation::ModuleInterfaceImpl
    {
        Q_OBJECT

    public:
        /// Default constructor.
        OgreAssetEditorModule();

        /// Destructor.
        ~OgreAssetEditorModule();

        /// ModuleInterfaceImpl overrides.
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

    private:
        OgreAssetEditorModule(const OgreAssetEditorModule &);
        void operator=(const OgreAssetEditorModule &);

        /// UiModule pointer.
        boost::weak_ptr<UiServices::UiModule> uiModule_;

        /// Event manager pointer.
        Foundation::EventManagerPtr eventManager_;

        /// Inventory event category.
        event_category_id_t frameworkEventCategory_;

        /// Inventory event category.
        event_category_id_t inventoryEventCategory_;

        /// Asset event category.
        event_category_id_t assetEventCategory_ ;

        /// Resource event category.
        event_category_id_t resourceEventCategory_;

        /// NetworkState event category.
        event_category_id_t networkStateEventCategory_;

        /// Editor manager.
        EditorManager *editorManager_;

        /// Material wizard.
        MaterialWizard *materialWizard_;
    };
}

#endif
