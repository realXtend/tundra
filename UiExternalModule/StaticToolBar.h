//$ HEADER_NEW_FILE $ 
// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_UiExternalModule_StaticToolBar_h
#define incl_UiExternalModule_StaticToolBar_h

#include "Framework.h"
#include "SceneManager.h"
#include "Entity.h"

#include <QObject>
#include <QToolBar>
#include <QMenu>

namespace UiExternalServices
{
	//! StaticToolBar has the static functionality of toolbar
    /*! This class inherits of QToolBar and build the static actions which has this toolbar. 
	*	The actions are:
	*		Switch between flying and walking mode
	*		Switch between free camera and avatar camera
	*		Switch between play and edit mode.
	*/
	 
	class StaticToolBar : public QToolBar
	 {
		Q_OBJECT

	public:

		/*! Constructor
		*	\param title Name of toolbar
		*	\param parent Pointer to QWidget parent of toolbar
		*	\param framework Pointer to framework
		*/
		StaticToolBar(const QString &title, QWidget *parent, Foundation::Framework* framework);
		
		//! Destructor
		~StaticToolBar();
		
		//! Handle the scene events to take the entity which has been clicked when the edit mode is active
		void HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

	public slots:

		/*! Enable the toolbar */
		void Enabled();

		/*! Disable the toolbar */
		void Disabled();

		/*!Slot used when the scene is changed, if we "go" to ether Scene, then disable the toolbar
		 * \param old_name Old Scene
		 * \param new_name New Scene
		 */
		void SceneChanged(const QString &old_name, const QString &new_name);

	private slots:

		//! Switch between flying and walking mode
		void flyMode();
		//! Switch between free camera and avatar camera
		void freeCameraMode();
		//! Switch between play and edit mode.
		void editMode();
		//! Open the EC Editor Window.
		void openECEditor();
		//! Open the Build Window..
		void openBuild();

	private:

		//! Create and configure actions
		void createActions();
		//! Add actions to this class (toolbar)
		void addActions();
		void CreateMenu();

		//! Fly action of toolbar
		QAction *flyAction_;
		//! Camera action of toolbar
		QAction *cameraAction_;
		//! Edit action of toolbar
		QAction *editAction_;

		//! Framework pointer
		Foundation::Framework *framework_;

		//! Query category of events
		 event_category_id_t scene_event_category_;
	
		//! Entity selected
		 Scene::Entity* entitySelected_;

		 QMenu* menu_asset;

		 QString currentScene;
	};
}

#endif