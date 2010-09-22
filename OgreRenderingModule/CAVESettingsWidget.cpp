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
		QObject::connect(vcave_button, SIGNAL(clicked(bool)), this, SLOT(VCAVE()));
		QObject::connect(bnave_button, SIGNAL(clicked(bool)), this, SLOT(BNAVE()));
		QObject::connect(minicave_button, SIGNAL(clicked(bool)), this, SLOT(MiniCAVE()));
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
                emit NewCAVEViewRequested(GetNextName(), tl ,bl, br, eye); 
            }
        }

    }

    void CAVESettingsWidget::AddNewCAVEView()
    {
        if(settings_dialog_.exec() == QDialog::Accepted)
        {
            Ogre::Vector3 bl, br, tl, eye;
            settings_dialog_.getCaveProjectionSettings(eye,bl,tl,br);
            emit NewCAVEViewRequested(GetNextName(), tl ,bl, br, eye);
           

        }
    }

	void CAVESettingsWidget::BNAVE()
	{
		emit ToggleCAVE(false);
		Ogre::Vector3 bl, br, tl, eye;
		eye = Ogre::Vector3(0,0,0);
		CAVEViewSettings::ConvertToVectors(80.5f,0.f,2.f,80.5f,80.5f*(3.f/4.f),bl,tl,br);
        emit NewCAVEViewRequested(GetNextName(), tl ,bl, br, eye);
		CAVEViewSettings::ConvertToVectors(-80.5f,0.f,2.f,80.5f,80.5f*(3.f/4.f),bl,tl,br);
        emit NewCAVEViewRequested(GetNextName(), tl ,bl, br, eye);
		CAVEViewSettings::ConvertToVectors(0.f,0.f,2.f,80.5f,80.5f*(3.f/4.f),bl,tl,br);
        emit NewCAVEViewRequested(GetNextName(), tl ,bl, br, eye);
		toggle_CAVE->setChecked(true);

	}
	void CAVESettingsWidget::VCAVE()
	{
		emit ToggleCAVE(false);
		Ogre::Vector3 bl, br, tl, eye;
		eye = Ogre::Vector3(0,0,0);
		CAVEViewSettings::ConvertToVectors(45.f,0.f,2.f,90.f,90.f*(3.f/4.f),bl,tl,br);
        emit NewCAVEViewRequested(GetNextName(), tl ,bl, br, eye);
		CAVEViewSettings::ConvertToVectors(-45.f,0.f,2.f,90.f,90.f*(3.f/4.f),bl,tl,br);
        emit NewCAVEViewRequested(GetNextName(), tl ,bl, br, eye);
		toggle_CAVE->setChecked(true);
	}
	void CAVESettingsWidget::MiniCAVE()
	{
		emit ToggleCAVE(false);
		Ogre::Vector3 bl, br, tl, eye;
		eye = Ogre::Vector3(0,0,0);
		CAVEViewSettings::ConvertToVectors(60.f,0.f,2.f,60.f,60.f*(3.f/4.f),bl,tl,br);
        emit NewCAVEViewRequested(GetNextName(), tl ,bl, br, eye);
		CAVEViewSettings::ConvertToVectors(-60.f,0.f,2.f,60.f,60.f*(3.f/4.f),bl,tl,br);
        emit NewCAVEViewRequested(GetNextName(), tl ,bl, br, eye);
		CAVEViewSettings::ConvertToVectors(0.f,0.f,2.f,60.f,60.f*(3.f/4.f),bl,tl,br);
        emit NewCAVEViewRequested(GetNextName(), tl ,bl, br, eye);
		toggle_CAVE->setChecked(true);

	}


    void CAVESettingsWidget::CAVEButtonToggled(bool v)
    {
        emit ToggleCAVE(v);
    }

	QString CAVESettingsWidget::GetNextName()
	{
		 QString view_name = view_prefix_;
         view_name += QString::number(next_view_num_);
		 next_view_num_++;
		 return view_name;
	}

}