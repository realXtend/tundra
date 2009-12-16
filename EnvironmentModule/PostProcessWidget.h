// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OgreRenderingModule_PostProcessWidget_h
#define incl_OgreRenderingModule_PostProcessWidget_h

#include <Foundation.h>
#include "qwidget.h"
#include "ui_postprocesswidget.h"
#include <QCheckBox>
#include "EnvironmentModule.h"

namespace OgreRenderer
{
    class CompositionHandler;
}

namespace Environment
{   

    
    //! Dialog for postprocessing effects
	class PostProcessWidget :
		public QWidget
	{
		Q_OBJECT
	public:
		PostProcessWidget(std::vector<std::string> &effects);
        //! Widget adds itself to the scene
        void AddSelfToScene(EnvironmentModule *env_module);
        //! Adds handler to handle the postprocess requests. Note that this class doesn't own the handler and is not responsible for deleting it
        void AddHandler(OgreRenderer::CompositionHandler* handler);
        //! Add effect names that will be shown in dialog with radiobuttons
		void AddEffects(std::vector<std::string> &effects);
		virtual ~PostProcessWidget(void);

	public slots:
		void handleSelection(bool checked, std::string name);

	private:
        //! Widget that this QWidget contains
		Ui::SelectionWidget widget_;
        //! CompositionHandler that is notified when radiobutton is clicked
        OgreRenderer::CompositionHandler* handler_;
	};
	
	//! inherited to get name of the checkbox with toggle signal
	class NamedCheckBox: public QCheckBox
	{
		Q_OBJECT
	signals:
		void Toggled(bool checked, std::string name);

	public:
		NamedCheckBox(const QString & text, QWidget * parent = 0);
		virtual ~NamedCheckBox();

	public slots:
		void ButtonToggled(bool checked);
	};
}

#endif