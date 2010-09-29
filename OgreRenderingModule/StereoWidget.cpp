#include "StableHeaders.h"
#include "StereoWidget.h"
//#include "ModuleManager.h"
#include "ServiceManager.h"
#include "UiServiceInterface.h"
#include "Framework.h"
#include <QColorDialog>

namespace OgreRenderer
{

    StereoWidget::StereoWidget(Foundation::Framework* framework, QWidget* parent)
        :QWidget(parent)

    {
        setupUi(this);

        framework_ = framework;
        UiServiceInterface *ui = framework->GetService<UiServiceInterface>();
            if (!ui)
                return;
        ui->AddSettingsWidget(this, "Stereo");
		QObject::connect(this->enable, SIGNAL(clicked()), this, SLOT(StereoOn()));
		QObject::connect(this->disable, SIGNAL(clicked()), this, SLOT(StereoOff()));
		QObject::connect(this->left_color, SIGNAL(clicked()), this, SLOT(ColorLeftChanged()));
		QObject::connect(this->right_color, SIGNAL(clicked()), this, SLOT(ColorRightChanged()));
    }


	void StereoWidget::StereoOn()
	{
		QString tech_name;
		qreal eye = this->eye_spacing->value();
		qreal focal = this->focal_length->value();
		qreal offset = this->pixel_shift->value();
		qreal scrn_width = this->screen_width->value();
		if(this->anaglyph->isChecked())
		{
			tech_name = "anaglyph";
		}else if(this->active->isChecked())
		{
			tech_name = "active";

		}else if(this->passive->isChecked())
		{
			tech_name = "passive";
		}

		emit EnableStereo(tech_name, eye,focal,offset, scrn_width);

	}
	void StereoWidget::StereoOff()
	{
		emit DisableStereo();
	}

	void StereoWidget::ColorLeftChanged()
	{
		QColorDialog dialog(QColor(1,0,0));
		QColor col = dialog.getColor();
		if(col.isValid())
		{	
			emit ChangeColorLeft(col.redF(), col.greenF(), col.blueF());
		}
	}
	void StereoWidget::ColorRightChanged()
	{
		QColorDialog dialog(QColor(0,1,1));
		QColor col = dialog.getColor();
		if(col.isValid())
		{	
			emit ChangeColorRight(col.redF(), col.greenF(), col.blueF());
		}
	}
}