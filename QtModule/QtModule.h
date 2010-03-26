// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_QtModule_h
#define incl_QtModule_QtModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "RenderServiceInterface.h"
#include "QtModuleApi.h"
#include "UIController.h"
#include <QMap>
#include <QPoint>

class QGraphicsScene;

namespace Input
{
    class InputModuleOIS;
}

namespace QtUI
{
	class UICanvasManager;
	class UICanvasTestEdit;
	
    /** QtModule provides other modules with the ability to maintain their own
        UI canvases, onto which widgets are loaded.
        These canvases can be then shown on the 2D screen or placed into the 3D world
        as textures on world geometry. The bitmap surfaces of each canvas are stored
        as textures residing on the GPU. */
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
        void Update(f64 frametime);

        MODULE_LOGGING_FUNCTIONS;

        /// @return Module name. Needed for logging.
        static const std::string &NameStatic();

        bool HandleEvent(event_category_id_t category_id,
            event_id_t event_id, 
            Foundation::EventDataInterface* data);
          
        /** Creates a new empty UI canvas.
            @param mode Pass UICanvas::Internal to have the UI canvas be composited on screen 
                through Ogre, and to allow it be used as a texture for 3D surfaces. Pass
                UICanvas::External to use the native OS windowing system to show the canvas.
            @return A weak_ptr to newly created canvas. In the calling module, store only 
                this weak_ptr as a member. Do not elevate this to a shared_ptr and store that,
                since that will lead to incorrect reference counting and problems with deleting
                canvases. */
        boost::weak_ptr<UICanvas> CreateCanvas(UICanvas::DisplayMode mode);

      
        boost::weak_ptr<UICanvas> GetCanvas(const QString& id);

        /** 
         *  Deletes the canvas. 
         *  @param The ID of the canvas to be deleted. 
         *  @throws a Core exception if canvas were not found. 
         *  @note before canvas is removed Hide() -will be called automatically.
         */
        void DeleteCanvas(const QString& id) { if  (controller_ != 0) controller_->RemoveCanvas(id); }

        /**
         * Deletes the canvas. 
         * @param canvas is canvas which will be deleted.
         * @throws a Core-exception if canvas were not found.
         * @note before canvas is removed Hide() -will be called automatically.
         */
        void DeleteCanvas(const UICanvas& canvas) { if (controller_!= 0) controller_->RemoveCanvas(canvas.GetID()); }

		/**
		 * Adds the canvas to canvasManagers_ ControlBar
		 * @param boost shared pointer to canvas
		 * @param qstring button name
		 */
		void AddCanvasToControlBar(boost::shared_ptr<QtUI::UICanvas> canvas, const QString &buttonTitle);
		
		/**
		 * Adds the canvas to canvasManagers_ ControlBar
		 * @param id of the canvas
		 * @param qstring button name
		 */
		void AddCanvasToControlBar(const QString& id, const QString &buttonTitle);
	
		/**
		 * Removes the canvas from canvasManagers_ ControlBar
		 * @param id of the canvas
		 */
		bool RemoveCanvasFromControlBar(const QString& id);
        
        //! Creates an EC_UICanvas to an entity using a UICanvas previously created. UICanvas must be in internal mode.
        /*! If entity already has EC_UICanvas component, just sets the canvas to use.
            @return Pointer to EC_UICanvas component, or null if the component could not be created out of some reason.
         */
        Foundation::ComponentPtr CreateEC_UICanvasToEntity(Scene::Entity* entity, boost::shared_ptr<QtUI::UICanvas> canvas);

        /**
         * Show/hide for the control bar with widget buttons
         * @param true if you want to show, false otherwise
         */
        void SetShowControlBar(bool show);

        /**
         * Return list of all UICanvases. Do not misuse!
         */
        const QList<boost::shared_ptr<UICanvas> >& GetCanvases() { return controller_->GetCanvases(); }

    private:
        
        /// Initializes a mapping table between OIS and Qt keyboard codes.
        void InitializeKeyCodes();
        
        /// Refreshes an entity containing EC_UICanvas, when a scene event telling of its modification is posted
        void RefreshEC_UICanvas(Foundation::EventDataInterface* data);

        Qt::KeyboardModifier GetCurrentModifier(const boost::shared_ptr<Input::InputModuleOIS>& input) const;

        boost::shared_ptr<UICanvas> GetCanvas(const Foundation::RaycastResult& result);

        // The event categories this module subscribes to.
        event_category_id_t input_event_category_;
        event_category_id_t renderer_event_category_;
        event_category_id_t scene_event_category_;

        bool mouse_left_button_down_;
        QPoint lastPos_;
        UIController* controller_;
		UICanvasManager *canvasManager_;
		UICanvasTestEdit *uicanvastestedit_;
        QMap<int, Qt::Key> converterMap_;
        QPoint lastLocation_;
    };
}

#endif

