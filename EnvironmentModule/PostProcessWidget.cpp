// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PostProcessWidget.h"
#include "EnvironmentModule.h"

#include "CompositionHandler.h"

#include <QApplication>

namespace Environment
{
    PostProcessWidget::PostProcessWidget(OgreRenderer::CompositionHandler *handler) :
        handler_(handler)
    {
        widget_.setupUi(this);
        AddEffects(handler_->GetAvailableCompositors());
        setWindowTitle(tr("Post-processing"));
    }

    PostProcessWidget::~PostProcessWidget()
    {
    }

    void PostProcessWidget::DisableAllEffects()
    {
        for(int i=0; i<widget_.checkboxlayout->count(); i++)
        {
            NamedCheckBox* c_box = dynamic_cast<NamedCheckBox*> (widget_.checkboxlayout->itemAt(i)->widget());
            if (c_box && c_box->isChecked())
                c_box->setChecked(false);
        }
    }

    void PostProcessWidget::changeEvent(QEvent *e)
    {
        if (e->type() == QEvent::LanguageChange)
        {
            setWindowTitle("Post-processing");
            // Then set this widget to right state for each widget which is created from ui - file this must be called! 
            widget_.retranslateUi(this);
        }
        else
           QWidget::changeEvent(e);
    }

    void PostProcessWidget::SetHandler(OgreRenderer::CompositionHandler *handler)
    {
        handler_ = handler;
    }

    void PostProcessWidget::EnableEffect(const QString &effect_name, bool enable)
    {
        for(int i=0; i<widget_.checkboxlayout->count(); i++)
        {
            NamedCheckBox *c_box = dynamic_cast<NamedCheckBox*>(widget_.checkboxlayout->itemAt(i)->widget());
            if (c_box && c_box->objectName() == effect_name)
            {
                c_box->setChecked(enable);
                break;
            }
            else
                EnvironmentModule::LogDebug("Casting error: tried to cast to NamedCheckBox");
        }
    }

    void PostProcessWidget::HandleSelection(bool checked, const QString &name)
    {
        if (checked)
            handler_->AddCompositorForViewport(name.toStdString());
        else
            handler_->RemoveCompositorFromViewport(name.toStdString());
    }

    void PostProcessWidget::AddEffects(QVector<QString> &effects)
    {
        for(int i=0; i< effects.size();i++)
        {
            QString effect_name = effects.at(i);
            NamedCheckBox* c_box = new NamedCheckBox(effect_name, this);
            c_box->setObjectName(effect_name);

            connect(c_box, SIGNAL(Toggled(bool, const QString &)), this, SLOT(HandleSelection(bool, const QString &)));
            widget_.checkboxlayout->addWidget(c_box);
        }
    }

    NamedCheckBox::NamedCheckBox(const QString &text, QWidget *parent) : QCheckBox(text, parent)
    {
        connect(this, SIGNAL(toggled(bool)), this, SLOT(ButtonToggled(bool)));
    }

    NamedCheckBox::~NamedCheckBox()
    {
    }

    void NamedCheckBox::ButtonToggled(bool checked)
    {
        emit Toggled(checked, objectName());
    }

    void NamedCheckBox::changeEvent(QEvent *e)
    {
        if (e->type() == QEvent::LanguageChange) 
            setText(QApplication::translate("CompositionHandler", objectName().toStdString().c_str()));
        else
           QCheckBox::changeEvent(e);
    }
}
