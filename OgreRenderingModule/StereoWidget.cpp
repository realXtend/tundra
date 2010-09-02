#include "StableHeaders.h"
#include "StereoWidget.h"
//#include "ModuleManager.h"
#include "ServiceManager.h"
#include "UiServiceInterface.h"
#include "Framework.h"
namespace OgreRenderer
{

    StereoWidget::StereoWidget(Foundation::Framework* framework, QWidget* parent)
        :QWidget(parent)

    {
        setupUi(this);

        framework_ = framework;
        Foundation::UiServiceInterface *ui = framework->GetService<Foundation::UiServiceInterface>();
            if (!ui)
                return;
        ui->AddSettingsWidget(this, "Stereo");
        
    }
}