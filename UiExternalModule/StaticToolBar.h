//$ HEADER_NEW_FILE $ 
// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_UiExternalModule_StaticToolBar_h
#define incl_UiExternalModule_StaticToolBar_h

#include "Framework.h"
#include "SceneManager.h"

#include <QObject>
#include <QToolBar>

namespace UiExternalServices
{
	//! StaticToolBar has the static functionality of toolbar
    /*! This class inherits of QToolBar and build the static actions which has this toolbar. 
	*	The actions are:
	*		Switch between flying and walking mode
	*		Switch between free camera and avatar camera
	*		Switch between play and edit mode.
	*
	*	This class also provides right button click 
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

	public slots:


	private slots:

		//! Switch between flying and walking mode
		void flyMode();
		//! Switch between free camera and avatar camera
		void freeCameraMode();
		//! Switch between play and edit mode.
		void editMode();

	private:

		//! Create and configure actions
		void createActions();
		//! Add actions to this class (toolbar)
		void addActions();

		//! Fly action of toolbar
		QAction *flyAction_;
		//! Camera action of toolbar
		QAction *cameraAction_;
		//! Edit action of toolbar
		QAction *editAction_;

		//! Query category of events
		 event_category_id_t scene_event_category_;

		 //! Name of the current scene
		 QString currentScene;

		//! Framework pointer
		Foundation::Framework *framework_;
	};
}

#endif