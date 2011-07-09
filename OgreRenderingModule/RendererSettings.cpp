// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "RendererSettings.h"
#include "Renderer.h"
#include "OgreRenderingModule.h"
#include "Framework.h"
#include "Application.h"
#include "InputAPI.h"

#include <QUiLoader>
#include <QFile>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QKeyEvent>
#include <QApplication>

#include "MemoryLeakCheck.h"

namespace OgreRenderer
{

RendererSettingsWindow::RendererSettingsWindow(Framework* fw, QWidget *parent) :
    QWidget(parent),
    framework_(fw)
{
    QUiLoader loader;
    QFile file(Application::InstallationDirectory() + "data/ui/renderersettings.ui");
    if (!file.exists())
    {
        LogError("Cannot find renderer settings .ui file.");
        return;
    }

    settings_widget_ = loader.load(&file, this);
    if (!settings_widget_)
        return;
    file.close();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(settings_widget_);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    setWindowTitle(tr("Renderer Settings"));

    QDoubleSpinBox* spin = settings_widget_->findChild<QDoubleSpinBox*>("spinbox_viewdistance");
    OgreRenderer::RendererPtr renderer = framework_->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
    if (!spin || !renderer)
        return;
    spin->setValue(renderer->GetViewDistance());
    QCheckBox* cbox = settings_widget_->findChild<QCheckBox*>("fullscreen_toggle");
    if(cbox)
    {
        cbox->setChecked(renderer->IsFullScreen());
        connect(cbox, SIGNAL(toggled(bool)), this, SLOT(SetFullScreenMode(bool)));
    }
    connect(spin, SIGNAL(valueChanged(double)), this, SLOT(ViewDistanceChanged(double)));

    QComboBox* combo = settings_widget_->findChild<QComboBox*>("combo_shadows");
    if (combo)
    {
        combo->setCurrentIndex((int)renderer->GetShadowQuality());
        connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(ShadowQualityChanged(int)));
    }
    
    combo = settings_widget_->findChild<QComboBox*>("combo_texture");
    if (combo)
    {
        combo->setCurrentIndex((int)renderer->GetTextureQuality());
        connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(TextureQualityChanged(int)));
    }

    //fullscreen shortcut key
    input_context_ = framework_->Input()->RegisterInputContext("Renderer", 90);
    if (input_context_)
        connect(input_context_.get(), SIGNAL(KeyPressed(KeyEvent*)), this, SLOT(KeyPressed(KeyEvent*)));
}

RendererSettingsWindow::~RendererSettingsWindow()
{
    SAFE_DELETE(settings_widget_);
}

void RendererSettingsWindow::KeyPressed(KeyEvent* e)
{
    OgreRenderer::RendererPtr renderer = framework_->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
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

void RendererSettingsWindow::ViewDistanceChanged(double value)
{
    OgreRenderer::RendererPtr renderer = framework_->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
    if (renderer)
        renderer->SetViewDistance((float)value);
}

void RendererSettingsWindow::SetFullScreenMode(bool value)
{
    OgreRenderer::RendererPtr renderer = framework_->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
    if (renderer)
        renderer->SetFullScreen(value);
}

void RendererSettingsWindow::ShadowQualityChanged(int value)
{
    if ((value < 0) || (value > 2))
        return;
        
    OgreRenderer::RendererPtr renderer = framework_->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
    if (!renderer)
        return;
    renderer->SetShadowQuality((ShadowQuality)value);
    QLabel* restart_text = settings_widget_->findChild<QLabel*>("label_restartmessage");
    if (restart_text)
        restart_text->setText(tr("Setting will take effect after viewer restart."));
}

void RendererSettingsWindow::TextureQualityChanged(int value)
{
    if ((value < 0) || (value > 1))
        return;
        
    OgreRenderer::RendererPtr renderer = framework_->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
    if (!renderer)
        return;
    renderer->SetTextureQuality((TextureQuality)value);
    QLabel* restart_text = settings_widget_->findChild<QLabel*>("label_restartmessage");
    if (restart_text)
        restart_text->setText(tr("SettingsWidget", "Setting will take effect after viewer restart."));
}

} //~namespace OgreRenderer
