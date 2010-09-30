#include "StableHeaders.h"
#include "CAVESettingsWidget.h"
#include "ModuleManager.h"
#include "ServiceManager.h"
#include "UiServiceInterface.h"
#include "Framework.h"
#include <Ogre.h>
#include <QDebug>

namespace CAVEStereo
{

    CAVESettingsWidget::CAVESettingsWidget(Foundation::Framework* framework, QWidget* parent)
        :QWidget(parent),
        settings_dialog_(this),
        next_view_num_(0),
        view_prefix_("View")
    {
        setupUi(this);

        framework_ = framework;
        UiServiceInterface *ui = framework->GetService<UiServiceInterface>();
            if (!ui)
                return;
        ui->AddSettingsWidget(this, "CAVE");
		QObject::connect(this, SIGNAL(NewCAVEViewRequested(const QString&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&)), this, SLOT(AddViewToUi(const QString&)));
        QObject::connect(toggle_CAVE, SIGNAL(toggled(bool)),this, SLOT(CAVEButtonToggled(bool)));
        QObject::connect(addView, SIGNAL(clicked(bool)),this,SLOT(AddNewCAVEView()));
        QObject::connect(addViewAdvanced, SIGNAL(clicked(bool)), this, SLOT(AddNewCAVEViewAdvanced()));
		QObject::connect(vcave_button, SIGNAL(clicked(bool)), this, SLOT(VCAVE()));
		QObject::connect(bnave_button, SIGNAL(clicked(bool)), this, SLOT(BNAVE()));
		QObject::connect(minicave_button, SIGNAL(clicked(bool)), this, SLOT(MiniCAVE()));
		QObject::connect(&modmapper_, SIGNAL(mapped(QString)), this, SLOT(ModifyViewPressed(QString)));
		QObject::connect(&remmapper_, SIGNAL(mapped(QString)), this, SLOT(DeleteViewPressed(QString)));
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

	void CAVESettingsWidget::AddViewToUi(const QString& name)
	{
		
		QLabel* label = new QLabel(name);
		label->setObjectName(name);
		QHBoxLayout* layout = new QHBoxLayout();

		QPushButton *modb = new QPushButton("Modify View Params", this);
		modb->setObjectName(name+"b1");
		modmapper_.setMapping(modb, modb->objectName());
		QObject::connect(modb, SIGNAL(clicked()), &modmapper_, SLOT(map()));

		QPushButton *remb = new QPushButton("Remove View", this);
		remb->setObjectName(name+"b2");
		remmapper_.setMapping(remb, remb->objectName());
		QObject::connect(remb, SIGNAL(clicked()), &remmapper_, SLOT(map()));
		
		layout->addWidget(label);
		layout->addWidget(modb);
		layout->addWidget(remb);
		viewslayout->addLayout(layout);
		
	}


	void CAVESettingsWidget::ModifyViewPressed(QString name)
	{
		name.chop(2);
		Ogre::Vector3 bl, tl, br, ep;
		 
		emit GetCAVEViewProjParams(name,tl, bl, br,ep);
		
		settings_dialog_advanced_.setCaveProjectionSettings(ep,bl,tl,br);
        if(settings_dialog_advanced_.exec() == QDialog::Accepted)
        {
			settings_dialog_advanced_.getCaveProjectionSettings(ep,bl,tl,br);
			emit ModifyCAVEViewProjParams(name, tl,bl,br,ep); 
        }

	}
	void CAVESettingsWidget::DeleteViewPressed(QString name)
	{
		QPushButton *button = this->findChild<QPushButton*>(name);
		
		if(button)
		{
			delete button->layout();
			remmapper_.removeMappings(button);
			button->deleteLater();
			name.chop(2);
			
			button = this->findChild<QPushButton*>(name+"b1");
			if(button)
			{
				modmapper_.removeMappings(button);
				button->deleteLater();
			}



			emit RemoveCAVEView(name);
			QLabel* label = this->findChild<QLabel*>(name);

			if(label)
			{
				label->deleteLater();
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
		toggle_CAVE->setChecked(false);
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
		toggle_CAVE->setChecked(false);
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
		toggle_CAVE->setChecked(false);
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