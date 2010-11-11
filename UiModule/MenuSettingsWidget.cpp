//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "MenuSettingsWidget.h"

#include "UiSceneService.h"
#include <QApplication>

namespace UiServices
{
    MenuSettingsWidget::MenuSettingsWidget(UiSceneService *owner) :
        owner_(owner),
		panels_()
    {
        widget_.setupUi(this);
		setWindowTitle(tr("Menu Config"));
    }

    MenuSettingsWidget::~MenuSettingsWidget()
    {
    }


    void MenuSettingsWidget::changeEvent(QEvent *e)
    {
        if (e->type() == QEvent::LanguageChange)
        {
            setWindowTitle("Menu Config");
            // Then set this widget to right state for each widget which is created from ui - file this must be called! 
            widget_.retranslateUi(this);
        }
        else
           QWidget::changeEvent(e);
    }

    void MenuSettingsWidget::SetPanelsList(QList<QString> &panels)
    {
        panels_ = panels;
		//We only use them if is the first time that we use them
		if (widget_.checkboxlayout->count()== 0)
			AddEffects();
    }

    void MenuSettingsWidget::HandleSelection(bool checked, const QString &name)
    {
		//QSettings saved for next time Naali starts, if is this var exists, the panel will be placed where here has been configured
		QSettings settings("Naali UIExternal", "UiExternal Settings");		
		if (checked){
			//Transfer inside the scene
			owner_->TransferWidgetOut(name, false);
			settings.setValue(name, "inside");
		}            
		else{
			//Transfer to the external widget
			owner_->TransferWidgetOut(name, true);
			settings.setValue(name, "outside");
		}

    }

    void MenuSettingsWidget::AddEffects()
    {
		for (int i = 0; i < panels_.size(); i++) {
			QString effect_name = panels_.at(i);
			NamedCheckBox* c_box = new NamedCheckBox(effect_name, this);
			c_box->setObjectName(effect_name);
			//Control if it has to be checked (menu is scene) and if it has to be disabled (non moveable)
			if (owner_->IsMenuInside(effect_name))
				c_box->setChecked(true);
			if (!owner_->IsMenuMoveable(effect_name))
				c_box->setDisabled(true);

			//Connect and put in widget
			connect(c_box, SIGNAL(Toggled(bool, const QString &)), this, SLOT(HandleSelection(bool, const QString &)));
            widget_.checkboxlayout->addWidget(c_box);				
		}
    }

	//NAMEDCHECKBOX
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
