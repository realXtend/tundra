// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   MaterialWizard.cpp
 *  @brief  Utitility tool for choosing right material script for your purpose from
 *          the Naali material script template library.
 */

#include "StableHeaders.h"
#include "MaterialWizard.h"
#include "OgreAssetEditorModule.h"

#include <Framework.h>
#include <UiModule.h>
#include <UiProxyWidget.h>
#include <UiWidgetProperties.h>

#include <QUiLoader>
#include <QFile>

namespace OgreAssetEditor
{

MaterialWizard::MaterialWizard(Foundation::Framework *framework) :
    framework_(framework)
{
    InitWindow();
}

MaterialWizard::~MaterialWizard()
{
}

void MaterialWizard::InitWindow()
{
    // Get QtModule and create canvas
    boost::shared_ptr<UiServices::UiModule> ui_module = 
        framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
        return;

    // Create widget from ui file
    QUiLoader loader;
    QFile file("./data/ui/materialwizard.ui");
    if (!file.exists())
    {
        OgreAssetEditorModule::LogError("Cannot find Material Wizard .ui file.");
        return;
    }

    mainWidget_ = loader.load(&file, 0);
    file.close();

    proxyWidget_ = ui_module->GetSceneManager()->AddWidgetToCurrentScene(
        mainWidget_, UiServices::UiWidgetProperties(QPointF(10.0, 60.0), mainWidget_->size(), Qt::Dialog, "Material Wizard"));
}

}
