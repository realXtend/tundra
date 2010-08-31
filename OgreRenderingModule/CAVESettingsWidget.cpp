#include "StableHeaders.h"
#include "CAVESettingsWidget.h"
#include "ModuleManager.h"
#include "ServiceManager.h"
#include "UiServiceInterface.h"
#include "Framework.h"
#include <Ogre.h>
#include <QDebug>

namespace OgreRenderer
{

    CAVESettingsWidget::CAVESettingsWidget(Foundation::Framework* framework, QWidget* parent)
        :QWidget(parent),
        settings_dialog_(this),
        next_view_num_(0),
        view_prefix_("View")
    {
        setupUi(this);

        framework_ = framework;
        Foundation::UiServiceInterface *ui = framework->GetService<Foundation::UiServiceInterface>();
            if (!ui)
                return;
        ui->AddSettingsWidget(this, "CAVE");
        QObject::connect(toggle_CAVE, SIGNAL(toggled(bool)),this, SLOT(CAVEButtonToggled(bool)));
        QObject::connect(addView, SIGNAL(clicked(bool)),this,SLOT(AddNewCAVEView()));
        QObject::connect(addViewAdvanced, SIGNAL(clicked(bool)), this, SLOT(AddNewCAVEViewAdvanced()));
        
    }

    void CAVESettingsWidget::AddNewCAVEViewAdvanced()
    {
        if(settings_dialog_.exec() == QDialog::Accepted)
        {
            Ogre::Vector3 bl, br, tl, eye;
            settings_dialog_.getCaveProjectionSettings(eye,bl,tl,br);
            settings_dialog_advanced_.setCaveProjectionSettings(eye,bl,tl,br);
            if(settings_dialog_advanced_.exec() == QDialog::Accepted)
            {
				settings_dialog_advanced_.getCaveProjectionSettings(eye,bl,tl,br);
                QString view_name = view_prefix_;
                view_name += QString::number(next_view_num_);
                emit NewCAVEViewRequested(view_name, tl ,bl, br, eye);
                next_view_num_++; 
            }
        }

    }

    void CAVESettingsWidget::AddNewCAVEView()
    {
        if(settings_dialog_.exec() == QDialog::Accepted)
        {
            Ogre::Vector3 bl, br, tl, eye;
            settings_dialog_.getCaveProjectionSettings(eye,bl,tl,br);
            QString view_name = view_prefix_;
            view_name += QString::number(next_view_num_);
            emit NewCAVEViewRequested(view_name, tl ,bl, br, eye);
            next_view_num_++;

        }
    }


    void CAVESettingsWidget::CAVEButtonToggled(bool v)
    {
        emit ToggleCAVE(v);
    }

}