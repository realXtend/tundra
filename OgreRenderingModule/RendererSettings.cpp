// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RendererSettings.h"
#include "OgreRenderingModule.h"
#include "ModuleManager.h"
#include "ServiceManager.h"
#include "Framework.h"
#include "UiServiceInterface.h"
#include "../Input/Input.h"

#include <QUiLoader>
#include <QFile>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QKeyEvent>
#include <QApplication>

namespace OgreRenderer
{
    RendererSettings::RendererSettings(Foundation::Framework* framework) :
        framework_(framework),
        settings_widget_(0)
    {
        InitWindow();
    }
    
    RendererSettings::~RendererSettings()
    {
        SAFE_DELETE(settings_widget_);
    }

    void RendererSettings::InitWindow()
    {
        UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
        if (!ui)
            return;

        QUiLoader loader;
        QFile file("./data/ui/renderersettings.ui");

        if (!file.exists())
        {
            OgreRenderingModule::LogError("Cannot find renderer settings .ui file.");
            return;
        }

        settings_widget_ = loader.load(&file); 
        if (!settings_widget_)
            return;

        ui->AddSettingsWidget(settings_widget_, "Rendering");

        QDoubleSpinBox* spin = settings_widget_->findChild<QDoubleSpinBox*>("spinbox_viewdistance");
        boost::shared_ptr<Renderer> renderer = framework_->GetServiceManager()->GetService<Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!spin || !renderer)
            return;
        spin->setValue(renderer->GetViewDistance());
        QCheckBox* cbox = settings_widget_->findChild<QCheckBox*>("fullscreen_toggle");
        if(cbox)
        {
            cbox->setChecked(renderer->IsFullScreen());
            QObject::connect(cbox, SIGNAL(toggled(bool)), this, SLOT(SetFullScreenMode(bool)));
        }
        QObject::connect(spin, SIGNAL(valueChanged(double)), this, SLOT(ViewDistanceChanged(double)));

        QComboBox* combo = settings_widget_->findChild<QComboBox*>("combo_shadows");
        if (combo)
        {
            combo->setCurrentIndex((int)renderer->GetShadowQuality());
            QObject::connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(ShadowQualityChanged(int)));
        }
        
        combo = settings_widget_->findChild<QComboBox*>("combo_texture");
        if (combo)
        {
            combo->setCurrentIndex((int)renderer->GetTextureQuality());
            QObject::connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(TextureQualityChanged(int)));
        }
        
        //fullscreen shortcut key
        input_context_ = framework_->GetInput()->RegisterInputContext("Renderer", 90);
        if(input_context_.get())
            connect(input_context_.get(), SIGNAL(KeyPressed(KeyEvent*)), this, SLOT(KeyPressed(KeyEvent*)));
    }

    void RendererSettings::KeyPressed(KeyEvent* e)
    {
        Renderer *renderer = framework_->GetService<Renderer>();
        if (!renderer)
            return;
        if(e->HasCtrlModifier() && e->KeyCode() == Qt::Key_F)
        {
            renderer->SetFullScreen(!renderer->IsFullScreen());
            QCheckBox* cbox = settings_widget_->findChild<QCheckBox*>("fullscreen_toggle");
            if(cbox)
                cbox->setChecked(!cbox->isChecked());
        }
    }

    void RendererSettings::ViewDistanceChanged(double value)
    {
        Renderer *renderer = framework_->GetService<Renderer>();
        if (renderer)
            renderer->SetViewDistance(value);
    }

    void RendererSettings::SetFullScreenMode(bool value)
    {
         Renderer *renderer = framework_->GetService<Renderer>();
        if (renderer)
            renderer->SetFullScreen(value);
    }
    
    void RendererSettings::ShadowQualityChanged(int value)
    {
        if ((value < 0) || (value > 2))
            return;
            
        Renderer *renderer = framework_->GetService<Renderer>();
        if (!renderer)
            return;
        renderer->SetShadowQuality((ShadowQuality)value);
        QLabel* restart_text = settings_widget_->findChild<QLabel*>("label_restartmessage");
        if (restart_text)
            restart_text->setText(QApplication::translate("SettingsWidget", "Setting will take effect after viewer restart."));
    }
    
    void RendererSettings::TextureQualityChanged(int value)
    {
        if ((value < 0) || (value > 1))
            return;
            
        Renderer *renderer = framework_->GetService<Renderer>();
        if (!renderer)
            return;
        renderer->SetTextureQuality((TextureQuality)value);
        QLabel* restart_text = settings_widget_->findChild<QLabel*>("label_restartmessage");
        if (restart_text)
            restart_text->setText(QApplication::translate("SettingsWidget", "Setting will take effect after viewer restart."));
    }
}
