//$ HEADER_MOD_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_InworldSceneController_h
#define incl_UiModule_InworldSceneController_h

#include "UiModuleApi.h"
#include "UiModuleFwd.h"

#include <QObject>
#include <QList>
#include <QPair>
#include <QRectF>
#include <QMap>

namespace Foundation
{
    class Framework;
}

class UiProxyWidget;

namespace UiServices
{
	typedef QPair<QGraphicsProxyWidget*, int> internal_element;

    class UI_MODULE_API InworldSceneController : public QObject
    {
        Q_OBJECT

    public:
        //! Constructor.
        //! \param framework Framework pointer.
        //! \param ui_view UI view for this scene manager.
        InworldSceneController(Foundation::Framework *framework, QGraphicsView *ui_view);

        //! Destructor.
        ~InworldSceneController();

		//! Framework pointer.
        Foundation::Framework *framework_;

    public slots:

	    /* Adds a widget in a layout in the scene in the position and orientation selected
	    * @param widget widget to be placed in the layout
	    * @param corner Corner of the screen: Could be enumCorner { TopLeftCorner, TopRightCorner, BottomLeftCorner, BottomRightCorner }
	    * @param orientation orientation arround the corner, could be { Horizontal, Vertical }
	    * @param priority higher priority means closer to the corner selected
	    *
	    * @return true if evertything allright
	    */
		bool AddAnchoredWidgetToScene(QWidget *widget, Qt::Corner corner, Qt::Orientation orientation, int priority = 0, bool persistence = true);
        bool AddAnchoredWidgetToScene(QGraphicsProxyWidget *qgrap, Qt::Corner corner, Qt::Orientation orientation, int priority = 0, bool persistence = 0);
        
        /* Removes a widget from layout in the scene 
	    * @param widget widget to be removed in the layout
    	*
	    * @return true if evertything allright
	    */
        bool RemoveAnchoredWidgetFromScene(QWidget *widget);
        bool RemoveAnchoredWidgetFromScene(QGraphicsProxyWidget *qgrap) ;

        //! Get the inworld ui scene
        QGraphicsScene *GetInworldScene() const { return inworld_scene_; }

        //! Get ControlPanelManager pointer
        CoreUi::ControlPanelManager *GetControlPanelManager()  const { return control_panel_manager_; }

		void worldDisconnected();

    private:
        Q_DISABLE_COPY(InworldSceneController);

        //! Pointer to main QGraphicsView
        QGraphicsView *ui_view_;

        //! Pointer to inworld widget scene
        QGraphicsScene *inworld_scene_;

        //! Layout manager
        CoreUi::AnchorLayoutManager *layout_manager_;

        //! Control panel manager
        CoreUi::ControlPanelManager *control_panel_manager_;

        //! Internal list of proxy widgets in scene.
        QList<QGraphicsProxyWidget *> all_proxy_widgets_in_scene_;

        //! Internal list of all docked proxy widgets.
        QList<QGraphicsProxyWidget *> all_docked_proxy_widgets_;


        //Store last scene rectangle 
        QRectF last_scene_rect;

        //! QGraphicsProxyWidget from dock_w widget.
        QGraphicsProxyWidget *docking_widget_proxy_;

		//Qlists to place internal_widgets
		QList<internal_element> bottomleft_horiz_;
		QList<internal_element> bottomleft_vert_;
		QList<internal_element> bottomright_horiz_;
		QList<internal_element> bottomright_vert_;
		QList<internal_element> topleft_horiz_;
		QList<internal_element> topleft_vert_;
		QList<internal_element> topright_horiz_;
		QList<internal_element> topright_vert_;

		//Qlist with widgets non-persistente
		QList<QGraphicsProxyWidget*> non_persistent_widgets;


    private slots:
		void AddInternalWidgets();
    };
}

#endif // incl_UiModule_InworldSceneController_h
