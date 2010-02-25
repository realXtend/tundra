// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PostProcessWidget.h"
#include "EnvironmentModule.h"

#include <CompositionHandler.h>

#include <UiModule.h>
#include <UiProxyWidget.h>
#include <UiWidgetProperties.h>

#include <QString>

namespace Environment
{
    PostProcessWidget::PostProcessWidget(std::vector<std::string> &effects) :
        QWidget(), handler_(0)
    {
        widget_.setupUi(this);
        AddEffects(effects);
    }

    PostProcessWidget::~PostProcessWidget()
    {
    }

    void PostProcessWidget::DisableAllEffects()
    {
        for(int i=0; i<widget_.checkboxlayout->count(); i++)
        {
            NamedCheckBox* c_box = dynamic_cast<NamedCheckBox*> (widget_.checkboxlayout->itemAt(i)->widget());
            if(c_box && c_box->isChecked())
                c_box->setChecked(false);
        }
    }

    void PostProcessWidget::AddSelfToScene(EnvironmentModule *env_module)
    {
        boost::shared_ptr<UiServices::UiModule> ui_module = env_module->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>(
            Foundation::Module::MT_UiServices).lock();
        ui_module->GetSceneManager()->AddWidgetToScene(this, UiServices::UiWidgetProperties("Post-processing", UiServices::ModuleWidget));
    }

    void PostProcessWidget::AddHandler(OgreRenderer::CompositionHandler *handler)
    {
        handler_ = handler;
    }

    void PostProcessWidget::EnableEffect(const std::string &effect_name, bool enable)
    {
        for(int i=0; i<widget_.checkboxlayout->count(); i++)
        {
            try
            {
                NamedCheckBox* c_box = dynamic_cast<NamedCheckBox*> (widget_.checkboxlayout->itemAt(i)->widget());
                if(c_box && c_box->objectName().toStdString() == effect_name)
                {
                    c_box->setChecked(enable);
                    break;
                }
            } catch(...)
            {
                EnvironmentModule::LogDebug("Casting error: tried to cast to NamedCheckBox");
            }
        }
    }

    void PostProcessWidget::HandleSelection(bool checked, const QString &name)
    {
        if (checked)
            handler_->AddCompositorForViewport(name.toStdString());
        else
            handler_->RemoveCompositorFromViewport(name.toStdString());
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

    NamedCheckBox::NamedCheckBox(const QString &text, QWidget *parent) : QCheckBox(text, parent)
    {
        QObject::connect(this, SIGNAL(toggled(bool)), this, SLOT(ButtonToggled(bool)));
    }

    NamedCheckBox::~NamedCheckBox()
    {
    }

    void NamedCheckBox::ButtonToggled(bool checked)
    {
        emit Toggled(checked, objectName());
    }
}
