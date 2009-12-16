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
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>

namespace OgreAssetEditor
{

MaterialWizard::MaterialWizard(Foundation::Framework *framework) :
    framework_(framework),
    proxyWidget_(0),
    mainWidget_(0)
{
    InitWindow();
}

MaterialWizard::~MaterialWizard()
{
}

void MaterialWizard::Create()
{
    Foundation::EventManagerPtr event_mgr = framework_->GetEventManager();
    Core::event_category_id_t event_cat = event_mgr->QueryEventCategory("Inventory");
    if (event_cat == 0)
    {
        OgreAssetEditorModule::LogError("Could not query event category \"Inventory\".");
        return;
    }

/*
    QString filename("../media/material/templates/");
    switch(parameters)
    {
    case Material_Diff:
        filename = ;
        break;
    case Material_Diffa:
        filename = ;
        break;
    case Material_DiffAlphamask:
        filename = ;
        break;
    case Material_DiffAnim:
        filename = ;
        break;
    case Material_DiffNormal:
        filename = ;
        break;
    case Material_DiffNormalLightmap:
        filename = ;
        break;
    case Material_DiffNormalShadow:
        filename = ;
        break;
    case Material_DiffNormalShadowLightmap:
        filename = ;
        break;
    case Material_DiffReflAlpha:
        filename = ;
        break;
    case Material_DiffShadow:
        filename = ;
        break;
    case Material_DiffShadowLightmap:
        filename = ;
        break;
    case Material_DiffSpecflatNormalShadowLum:
        filename = ;
        break;
    case Material_DiffSpecflatShadow:
        filename = ;
        break;
    case Material_DiffSpecmap:
        filename = ;
        break;
    case Material_DiffSpecmapNormal:
        filename = ;
        break;
        filename = ;
    case Material_DiffSpecmapNormalOpa:
        filename = ;
        break;
    case Material_DiffSpecmapNormalShadow:
        filename = ;
        break;
    case Material_DiffSpecmapNormalShadowLum:
        filename = ;
        break;
    case Material_DiffSpecmapNormalShadowLumOpa:
        filename = ;
        break;
    case Material_SpecmapRefl:
        filename = ;
        break;
    case Material_DiffSpecmapShadow:
        filename = ;
        break;
    case Material_DiffSpecmapShadowLum:
        filename = ;
        break;
    case Material_DiffSpecmapShadowLumOpa:
        filename = ;
        break;
    case Material_DiffSpecmapShadowOpa:
        filename = ;
        break;
    case Material_DiffSpecmapShadowRefl:
        filename = ;
        break;
    Material_None:
    default:
        break;
    }

    filename.append();
    QFile file(filename);
    if (!file.exists())
    {
        OgreAssetEditorModule::LogError("Could not appropriate material script for these parameters!");
        return;
    }

    // Create event data.
    Inventory::InventoryUploadEventData event_data;
    event_data.filenames.push_back(file.filename);

    event_mgr->SendEvent(event_cat, Inventory::Events::EVENT_INVENTORY_UPLOAD_BUFFER, &event_data);
}
    */
}

void MaterialWizard::Cancel()
{
    proxyWidget_->close();
}

void MaterialWizard::InitWindow()
{
    boost::shared_ptr<UiServices::UiModule> ui_module = 
        framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
        return;

    QUiLoader loader;
    QFile file("./data/ui/materialwizard.ui");
    if (!file.exists())
    {
        OgreAssetEditorModule::LogError("Cannot find Material Wizard .ui file.");
        return;
    }

    mainWidget_ = loader.load(&file, 0);
    file.close();

    QList<QObject *> widgetList = mainWidget_->findChildren<QObject *>();
    QListIterator<QObject *> iter(widgetList);
    while(iter.hasNext())
    {
        QObject *obj = iter.next();
        QString objName = obj->objectName();
        if (objName.indexOf("Param") != -1)
            QObject::connect(obj, SIGNAL(clicked(bool)), this, SLOT(RefreshWidgets()));
    }

    QPushButton *buttonCreate = mainWidget_->findChild<QPushButton *>("buttonCreate");
    QPushButton *buttonCancel = mainWidget_->findChild<QPushButton *>("buttonCancel");
    QObject::connect(buttonCreate, SIGNAL(clicked(bool)), this, SLOT(Create()));
    QObject::connect(buttonCancel, SIGNAL(clicked(bool)), this, SLOT(Cancel()));
//    QObject::connect(lineEditName_, SIGNAL(textChanged(const QString &)), this, SLOT(ValidateScriptName(const QString &)));

    proxyWidget_ = ui_module->GetSceneManager()->AddWidgetToCurrentScene(
        mainWidget_, UiServices::UiWidgetProperties(QPointF(10.0, 60.0), mainWidget_->size(), Qt::Dialog, "Material Wizard", false));
}

void MaterialWizard::RefreshWidgets()
{
//    MaterialWizardOptions options;
    
/*
    int count

    // Diffuse
    radioDiffSolidParam
    radioDiffRgbTexParam
    checkBoxDiffModulateParam

    // Specular
    radioSpecNoneParam
    radioSpecRgbTexParam
    radioSpecRgbTexParam

    // Alpha
    radioAlphaNoneParam
    radioAlphaSolidParam
    radioAlphaSeparateParam
    radioAplhaUseChanFromDiffMapParam
    checkBoxAlphaMaskingParam

    // Misc
    // Anim only if DIFFUSE_MAPPING and no nothing else.
    checkBoxAnimParam

        Material_None                           = 0,
        Material_Diff                           = Material_None | MWO_DiffuseMap,
        Material_Diffa                          = Material_Diff | MWO_DiffuseAlpha,
        Material_DiffAlphamask                  = Material_Diff | MWO_AlphaMasking,
        Material_DiffAnim                       = Material_Diff | MWO_Animation,
        Material_DiffNormal                     = Material_Diff | MWO_NormalMap,
        Material_DiffNormalLightmap             = Material_DiffNormal | MWO_LightMap,
        Material_DiffNormalShadow               = Material_DiffNormal | MWO_ReceivesShadows,
        Material_DiffNormalShadowLightmap       = Material_DiffNormalShadow | MWO_LightMap,
        Material_DiffReflAlpha                  = Material_Diff | MWO_ReflectionMap | MWO_AlphaSolid,
        Material_DiffShadow                     = Material_Diff | MWO_ReceivesShadows,
        Material_DiffShadowLightmap             = Material_DiffShadow | MWO_LightMap,
        Material_DiffSpecflatNormalShadowLum    = Material_Diff | MWO_SpecularSolid | MWO_NormalMap | MWO_ReceivesShadows | MWO_LuminanceMap,
        Material_DiffSpecflatShadow             = Material_Diff | MWO_SpecularSolid | MWO_ReceivesShadows,
        Material_DiffSpecmap                    = Material_Diff | MWO_SpecularMap,
        Material_DiffSpecmapNormal              = Material_DiffSpecmap | MWO_NormalMap,
        Material_DiffSpecmapNormalOpa           = Material_DiffSpecmapNormal | MWO_Opacity,
        Material_DiffSpecmapNormalShadow        = Material_DiffSpecmapNormal | MWO_ReceivesShadows,
        Material_DiffSpecmapNormalShadowLum     = Material_DiffSpecmap | MWO_ReceivesShadows | MWO_LuminanceMap,
        Material_DiffSpecmapNormalShadowLumOpa  = Material_DiffSpecmap | MWO_NormalMap | MWO_ReceivesShadows | MWO_LuminanceMap | MWO_Opacity,
        Material_SpecmapRefl                    = Material_DiffSpecmap | MWO_ReflectionMap,
        Material_DiffSpecmapShadow              = Material_DiffSpecmap | MWO_ReceivesShadows,
        Material_DiffSpecmapShadowLum           = Material_DiffSpecmapShadow | MWO_LuminanceMap,
        Material_DiffSpecmapShadowLumOpa        = Material_DiffSpecmapShadowLum | MWO_Opacity,
        Material_DiffSpecmapShadowOpa           = Material_DiffSpecmapShadow | MWO_Opacity,
        Material_DiffSpecmapShadowRefl          = Material_DiffSpecmapShadow | MWO_ReflectionMap,

    // Lightmap choosable with 1)normal, 2)normal+shadow, 3)shadow
    //checkBoxLightParam

    // Lum choosable with 1)Specflat+Normal+Shadow 2) Specmap+Normal+Shadow
    //checkBoxLumParam

    // Opa Choosable with 2)DiffSpecmapNormal 2)DiffSpecmapNormalShadowLum 3)DiffSpecmapShadowLum
    // 4)DiffSpecmapShadowOp

    // Normal choosable with 1)DiffNormal 2)DiffNormalLightmap 3)DiffNormalShadow 4)DiffNormalShadowLightmap
    // 5)DiffSpecflatNormalShadowLum 6)DiffSpecmapNormal 7)DiffSpecmapNormalOpa 8)DiffSpecmapNormalShadow 9)DiffSpecmapNormalShadowLum
    // 10)DiffSpecmapNormalShadowLumOpa
    //checkBoxNormalParam

    // Refl choosable with 1)DiffReflAlpha 2)SpecmapRefl 3)DiffSpecmapShadowRefl
    //checkBoxReflParam
    

    // 1)DiffNormalShadow 2)DiffNormalShadowLightmap
    DiffShadow
    DiffShadowLightmap
    DiffSpecflatNormalShadowLum
    DiffSpecflatShadow
    DiffSpecmapNormalOpa
    DiffSpecmapNormalShadow
    DiffSpecmapNormalShadowLum
    DiffSpecmapNormalShadowLumOpa
    DiffSpecmapShadow
    DiffSpecmapShadowLum
    DiffSpecmapShadowLumOpa
    DiffSpecmapShadowOpa
    DiffSpecmapShadowRefl
    checkBoxShadowParam

/*
    // available only with DiffAlphamask
    QCheckBox *alphaMasking = mainWidget_->findChild<QCheckBox *>("checkBoxAlphaMasking");
    if (1)
        alphaMasking->setEnabled(false);
    else
        alphaMasking->setEnabled(true);
*/


    
    //if (specSolid)
    //SSDiffSpecflatNormalShadowLum.material  SpecSolid + Normal + Shadow + Lum
    //SSDiffSpecflatShadow.material           SpecSolid + Shadow

/*
    QPushButton *buttonCreate = mainWidget_->findChild<QPushButton *>("buttonCreate");
    QLineEdit *lineEditName = mainWidget_->findChild<QLineEdit *>("lineEditName");
    QString name = lineEditName->text();
    if (name.isEmpty() || name.isNull())
        buttonCreate->setEnabled(false);
    else
        buttonCreate->setEnabled(true);
*/
}

}
