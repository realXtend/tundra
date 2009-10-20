// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_UICanvasManager_h
#define incl_QtModule_UICanvasManager_h

#include "Foundation.h"
#include "QtModuleApi.h"
#include "UICanvas.h"

#include <QtGui>
#include <QObject>

namespace QtUI
{
	class QtModule;
	class UICanvas;

	class QT_MODULE_API UICanvasManager : public QObject
	{

	Q_OBJECT

	public:
		UICanvasManager(Foundation::Framework *framework);
		virtual ~UICanvasManager(void);

		//! Adds a canvas to the ControlBar
		/// @param boost shared pointer to the UICanvas
		const void AddCanvasToControlBar(boost::shared_ptr<QtUI::UICanvas> canvas, QString buttonTitle);
		
		//! removes a canvas from the ControlBar
		/// @param boost shared pointer to the UICanvas
		const bool RemoveCanvasFromControlBar(boost::shared_ptr<QtUI::UICanvas> canvas);

	private:
		//! Init internal widgets
		void InitManagedWidgets();

		QWidget *controlBarWidget_;
		QHBoxLayout *controlBarLayout_;

		boost::shared_ptr<QtUI::UICanvas> controlBarCanvas_;
		boost::weak_ptr<QtUI::QtModule> qtModule_;
		Foundation::Framework *framework_;

	};

	class ControlBarButton : public QPushButton
	{

	Q_OBJECT

	public:
		ControlBarButton(QWidget *parent, boost::shared_ptr<QtUI::UICanvas> canvas, const QString buttonTitle);
		boost::shared_ptr<QtUI::UICanvas> GetCanvas();

	public slots:
		void toggleShow();
	
	private:
		QString myTitle_;
		boost::shared_ptr<QtUI::UICanvas> myCanvas_;

	};
}

#endif // incl_QtModule_UICanvasManager_h