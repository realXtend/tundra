// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_QtModule_h
#define incl_QtModule_QtModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "QtModuleApi.h"

#include <QList>
#include "UICanvas.h"
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

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

    /// @return The QGraphicsScene that corresponds to the main fullscreen 2D UI canvas.
    QGraphicsScene *GetUIScene() const;

	/**
	 * Creates a canvas which is tracked by QtModule. 
	 * @return pointer to newly created canvas.
	 */
    boost::weak_ptr<UICanvas> CreateCanvas(UICanvas::Mode mode) { canvases_.append(boost::shared_ptr<UICanvas>(new UICanvas(mode))); return canvases_.last(); }
   	
    void DeleteCanvas(UICanvas* canvas) {} 

private:
    ///\todo Currently the QtModule only holds a single main 2D scene.
    /// Expand this functionality to allow multiple scenes to be created,
    /// this becomes more current when the 3D scene system is implemented.
    /// Currently it feels like we should make each QWidget its own scene,
    /// and perform compositing on a per-widget basis to minimize redundant
    /// redraws.
    QGraphicsScene *main_scene_;
    UICanvas *main_view_;

    // The event categories this module subscribes to.
    Core::event_category_id_t input_event_category_;
    Core::event_category_id_t renderer_event_category_;

    /// Polls any changes to input state during this frame from OIS Input Module.
    ///\todo Make event-based and have RexLogic or some other piece of code arbitrate
    /// whether input is routed into the world or whether it goes to the UI.
    void PollMouseInput();

    bool mouse_left_button_down_;
    QList<boost::shared_ptr<UICanvas> > canvases_;

	
};

}

#endif

