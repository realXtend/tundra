// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_QtModule_h
#define incl_QtModule_QtModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "QtModuleApi.h"
#include "UIController.h"
#include <QMap>

class QGraphicsScene;
namespace QtUI
{

/// QtModule provides other modules with the ability to load widgets on canvases.
/// These canvases can be then shown on the 2D screen or placed into the 3D world
/// as textures on world geometry. The bitmap surfaces of each canvas are stored
/// as textures residing on the GPU.

class QT_MODULE_API QtModule : public Foundation::ModuleInterfaceImpl
{
public:
    QtModule();
    ~QtModule();

    void Load();
    void Unload();
    void PreInitialize();
    void Initialize();
    void PostInitialize();
    void Uninitialize();
    void Update(Core::f64 frametime);

    MODULE_LOGGING_FUNCTIONS;

    //! returns name of this module. Needed for logging.
    static const std::string &NameStatic();

    bool HandleEvent(Core::event_category_id_t category_id,
        Core::event_id_t event_id, 
        Foundation::EventDataInterface* data);
   
    /**
	 * Creates a canvas which is tracked by UIController. 
	 * @return weak_ptr to newly created canvas.
	 */
    boost::weak_ptr<UICanvas> CreateCanvas(UICanvas::Mode mode) { return controller_->CreateCanvas(mode); }

    /**
     * Deletes canvas. Because UICanvas is shared pointer, it actually removes a reference from a controller lists. 
     * 
     */

    void DeleteCanvas(const boost::shared_ptr<UICanvas>& canvas) { controller_->RemoveCanvas(canvas->GetID()); }

private:

    void InitializeKeyCodes();

    // The event categories this module subscribes to.
    Core::event_category_id_t input_event_category_;
    Core::event_category_id_t renderer_event_category_;

    

    bool mouse_left_button_down_;
    QPoint lastPos_;

    UIController* controller_;

    QMap<int, Qt::Key> converterMap_;
};

}

#endif

