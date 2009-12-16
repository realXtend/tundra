#include "StableHeaders.h"
#include "PostProcessWidget.h"
#include <CompositionHandler.h>
#include <QString>

#include <UiModule.h>
#include <UiProxyWidget.h>
#include <UiWidgetProperties.h>


namespace Environment
{
	PostProcessWidget::PostProcessWidget(std::vector<std::string> &effects)
	:QWidget(),
    handler_(0)

	{
		widget_.setupUi(this);
		AddEffects(effects);
        
		
	}

    void PostProcessWidget::AddSelfToScene(EnvironmentModule *env_module)
    {
        boost::shared_ptr<UiServices::UiModule> ui_module = env_module->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        ui_module->GetSceneManager()->AddWidgetToCurrentScene(this, UiServices::UiWidgetProperties("Post-processing"));
    }

    void PostProcessWidget::AddHandler(OgreRenderer::CompositionHandler* handler)
	{
        this->handler_ = handler;
	}

	void PostProcessWidget::handleSelection(bool checked, std::string name)
	{
		if(checked)
		{
            handler_->AddCompositorForViewport(name);
		}else{
            handler_->RemoveCompositorFromViewport(name);
		}
	}

	void PostProcessWidget::AddEffects(std::vector<std::string> &effects)
	{
		for(int i=0; i< effects.size();i++)
		{
			std::string effect_name = effects.at(i);
			NamedCheckBox* c_box = new NamedCheckBox(effect_name.c_str(), this);
			c_box->setObjectName(effect_name.c_str());
            
			QObject::connect(c_box, SIGNAL(Toggled(bool, std::string)), this, SLOT(handleSelection(bool, std::string)));
			widget_.checkboxlayout->addWidget(c_box);
		}
	}

	PostProcessWidget::~PostProcessWidget(void)
	{
	}






	//! NamedCheckBox

	NamedCheckBox::NamedCheckBox(const QString & text, QWidget * parent):QCheckBox(text, parent)
	{
		QObject::connect(this, SIGNAL(toggled(bool)), this, SLOT(ButtonToggled(bool)));
        setStyleSheet("");
        setStyleSheet("QLabel, QLabel:hover { background-color: rgba(0,0,0,0); color: rgb(255,255,255); } QCheckBox { background-color: rgba(0,0,0,0); color: rgb(255,255,255); } QCheckBox:hover { background-color: rgba(0,0,0,0); color: rgb(255,255,255); }");
	}

	NamedCheckBox::~NamedCheckBox()
	{

	}
	void NamedCheckBox::ButtonToggled(bool checked)
	{
		emit Toggled(checked, this->objectName().toStdString());
	}

}
