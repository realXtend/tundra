/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   MaterialWizard.cpp
 *  @brief  Utitility tool for choosing right material script from the material script template library.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MaterialWizard.h"
#include "OgreAssetEditorModule.h"
#include "LoggingFunctions.h"
#include "Application.h"

#include <QUiLoader>

#include "MemoryLeakCheck.h"

// Useful defines
#define ENABLE(p) p->setEnabled(true);
#define DISABLE(p) p->setEnabled(false);

MaterialWizard::MaterialWizard(QWidget *parent) :
    QWidget(parent),
    mainWidget_(0),
    currentOptions_(Material_None),
    scriptName_("")
{
    QUiLoader loader;
    QFile file(Application::InstallationDirectory() + "data/ui/materialwizard.ui");
    if (!file.exists())
    {
        LogError("Cannot find Material Wizard .ui file.");
        return;
    }

    // Get pointers to widgets.
    mainWidget_ = loader.load(&file, this);
    file.close();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mainWidget_);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // Connect parameter widgets' clicked signal to RefreshWidgets slot.
    QList<QObject *> widgetList = mainWidget_->findChildren<QObject *>();
    QListIterator<QObject *> iter(widgetList);
    while(iter.hasNext())
    {
        QObject *obj = iter.next();
        if (obj->objectName().indexOf("Param") != -1)
            QObject::connect(obj, SIGNAL(clicked(bool)), this, SLOT(RefreshWidgets()));
    }

    QPushButton *buttonCreate = mainWidget_->findChild<QPushButton *>("buttonCreate");
    QPushButton *buttonCancel = mainWidget_->findChild<QPushButton *>("buttonCancel");
    QLineEdit *lineEditName = mainWidget_->findChild<QLineEdit *>("lineEditName");

    QObject::connect(buttonCreate, SIGNAL(clicked(bool)), this, SLOT(Create()));
    QObject::connect(buttonCancel, SIGNAL(clicked(bool)), this, SLOT(Close()));
    QObject::connect(lineEditName, SIGNAL(textChanged(const QString &)), this, SLOT(ValidateScriptName(const QString &)));

    buttonCreate->setEnabled(false);

    setWindowTitle(tr("Material Wizard"));

    RefreshWidgets();
}

MaterialWizard::~MaterialWizard()
{
}

void MaterialWizard::Create()
{
    QString filename(Application::InstallationDirectory() + "media/materials/templates/");
    filename.append(GetCurrentMaterialFilename());
    filename.append(".material");

    QFile file(filename);
    if (!file.exists())
    {
        LogError("Could create appropriate material script for these parameters!");
        return;
    }

/*
    Inventory::InventoryUploadEventData event_data;
    event_data.filenames.push_back(file.fileName());
    event_data.names.push_back(scriptName_);
    emit NewMaterial(&event_data);
*/

    graphicsProxyWidget()->hide();
    ClearSelections();
}

void MaterialWizard::Close()
{
    graphicsProxyWidget()->hide();
    ClearSelections();
}

void MaterialWizard::RefreshWidgets()
{
    ///\todo Solid not supported yet.
    //QRadioButton *radioDiffSolidParam = mainWidget_->findChild<QRadioButton *>("radioDiffSolidParam");
    QRadioButton *radioDiffRgbTexParam = mainWidget_->findChild<QRadioButton *>("radioDiffRgbTexParam");

    // Specular params.
    QRadioButton *radioSpecNoneParam = mainWidget_->findChild<QRadioButton *>("radioSpecNoneParam");
    QRadioButton *radioSpecSolidParam = mainWidget_->findChild<QRadioButton *>("radioSpecSolidParam");
    QRadioButton *radioSpecRgbTexParam = mainWidget_->findChild<QRadioButton *>("radioSpecRgbTexParam");

    // Alpha params.
    QRadioButton *radioAlphaNoneParam = mainWidget_->findChild<QRadioButton *>("radioAlphaNoneParam");
    QRadioButton *radioAlphaSolidParam = mainWidget_->findChild<QRadioButton *>("radioAlphaSolidParam");
    QRadioButton *radioAlphaSeparateParam = mainWidget_->findChild<QRadioButton *>("radioAlphaSeparateParam");
    QRadioButton *radioAlphaUseChanFromDiffMapParam = mainWidget_->findChild<QRadioButton *>("radioAlphaUseChanFromDiffMapParam");
    QCheckBox *checkBoxAlphaMaskingParam = mainWidget_->findChild<QCheckBox *>("checkBoxAlphaMaskingParam");

    // Misc. params.
    QCheckBox *checkBoxAnimParam = mainWidget_->findChild<QCheckBox *>("checkBoxAnimParam");
    QCheckBox *checkBoxLightParam = mainWidget_->findChild<QCheckBox *>("checkBoxLightParam");
    QCheckBox *checkBoxLumParam = mainWidget_->findChild<QCheckBox *>("checkBoxLumParam");
    QCheckBox *checkBoxNormalParam = mainWidget_->findChild<QCheckBox *>("checkBoxNormalParam");
    QCheckBox *checkBoxReflParam = mainWidget_->findChild<QCheckBox *>("checkBoxReflParam");
    QCheckBox *checkBoxShadowParam = mainWidget_->findChild<QCheckBox *>("checkBoxShadowParam");

/*
    // Enable and disable widgets according to the current material parameter combination
    ENABLE(radioDiffRgbTexParam) // always on for now

    if (currentOptions_ == Material_Diff)
    {
        ENABLE(radioAlphaUseChanFromDiffMapParam)
        ENABLE(checkBoxAlphaMaskingParam)
        ENABLE(checkBoxAnimParam)
        ENABLE(checkBoxReflParam)
    }
    else
    {
        DISABLE(radioAlphaUseChanFromDiffMapParam)
        DISABLE(checkBoxAlphaMaskingParam)
        DISABLE(checkBoxAnimParam)
        DISABLE(checkBoxReflParam)
    }

    if (currentOptions_ == Material_DiffNormal)
    {
        ENABLE(checkBoxLightParam)
        ENABLE(checkBoxShadowParam)
    }

    if (SSDiffReflAlpha)
    {
        //DISABLE_ALL_BUT()
    }

    if (ssdiffspecflat)
    {
        ENABLE(normal)
        ENABLE(shadow)
        ENABLE(lum)
        DISABLE_REST();
    }

    if (SSDiffSpecmap)
    {
        ENABLE(normal)
        ENABLE(shadow)
        ENABLE(lum)
        ENABLE(opa)
        ENABLE(refl)

    }
*/
    currentOptions_ = Material_None;

    // Diffuse
    if (radioDiffRgbTexParam->isChecked())
        currentOptions_ |= MWO_DiffuseMap;
    ///\todo "Modulate diffuse with vertex color" not supported yet. checkBoxDiffModulateParam

    // Specular
    if (radioSpecNoneParam->isEnabled() && radioSpecNoneParam->isChecked())
        currentOptions_ |= MWO_SpecularNone;
    if (radioSpecSolidParam->isEnabled() && radioSpecSolidParam->isChecked())
        currentOptions_ |= MWO_SpecularSolid;
    if (radioSpecRgbTexParam->isEnabled() && radioSpecRgbTexParam->isChecked())
        currentOptions_ |= MWO_SpecularMap;

    // Alpha
    if (radioAlphaNoneParam->isEnabled() && radioAlphaNoneParam->isChecked())
        currentOptions_ |= MWO_AlphaNone;
    if (radioAlphaSolidParam->isEnabled() && radioAlphaSolidParam->isChecked())
        currentOptions_ |= MWO_AlphaSolid;
    if (radioAlphaSeparateParam->isEnabled() && radioAlphaSeparateParam->isChecked())
        currentOptions_ |= MWO_Opacity;
    if (radioAlphaUseChanFromDiffMapParam->isEnabled() && radioAlphaUseChanFromDiffMapParam->isChecked())
        currentOptions_ |= MWO_DiffuseAlpha;
    if (checkBoxAlphaMaskingParam->isEnabled() && checkBoxAlphaMaskingParam->isChecked())
        currentOptions_ |= MWO_AlphaMasking;

    // Misc
    if (checkBoxAnimParam->isEnabled() && checkBoxAnimParam->isChecked())
        currentOptions_ |= MWO_Animation;
    if (checkBoxLightParam->isEnabled() && checkBoxLightParam->isChecked())
        currentOptions_ |= MWO_LightMap;
    if (checkBoxLumParam->isEnabled() && checkBoxLumParam->isChecked())
        currentOptions_ |= MWO_LuminanceMap;
    if (checkBoxNormalParam->isEnabled() && checkBoxNormalParam->isChecked())
        currentOptions_ |= MWO_NormalMap;
    if (checkBoxReflParam->isEnabled() && checkBoxReflParam->isChecked())
        currentOptions_ |= MWO_ReflectionMap;
    if (checkBoxShadowParam->isEnabled() && checkBoxShadowParam->isChecked())
        currentOptions_ |= MWO_ReceivesShadows;

    QLabel *labelCurrent = mainWidget_->findChild<QLabel *>("labelCurrent");
    labelCurrent->setText(GetCurrentMaterialFilename());

#if 0
    // OLD SHIT STARTS HERE

    // Diffuse
    if (radioDiffRgbTexParam->isChecked())
    {
        currentOptions_ |= MWO_DiffuseMap;
        std::cout << "MWO_DiffuseMap" << std::endl;
    }

    ///\todo "Modulate diffuse with vertex color" not supported yet.
    //QCheckBox *checkBoxDiffModulateParam = mainWidget_->findChild<QCheckBox *>("checkBoxDiffModulateParam");

    // Specular
    if (radioSpecNoneParam->isChecked())
    {
        currentOptions_ |= MWO_SpecularNone;
        std::cout << "MWO_SpecularNone" << std::endl;
    }
    else if(radioSpecSolidParam->isChecked())
    {
        currentOptions_ |= MWO_SpecularSolid;
        std::cout << "MWO_SpecularSolid" << std::endl;
    }
    else if(radioSpecRgbTexParam->isChecked())
    {
        currentOptions_ |= MWO_SpecularMap;
        std::cout << "MWO_SpecularMap" << std::endl;
    }

    if (currentOptions_ & MWO_SpecularSolid)
    {
        checkBoxLumParam->setEnabled(true);
        checkBoxNormalParam->setEnabled(true);
        checkBoxShadowParam->setEnabled(true);
    }


    if (currentOptions_ == Material_Diff)
    {
    
    }

    // Alpha
    if (radioAlphaNoneParam->isChecked())
    {
        currentOptions_ |= MWO_AlphaNone;
        std::cout << "MWO_AlphaNone" << std::endl;
    }
    else if(radioAlphaSolidParam->isChecked())
    {
        currentOptions_ |= MWO_AlphaSolid;
        std::cout << "MWO_AlphaSolid" << std::endl;
    }
    else if(radioAlphaSeparateParam->isChecked())
    {
        currentOptions_ |= MWO_Opacity;
        std::cout << "MWO_Opacity" << std::endl;
        
        SpecRgbTex + Normal
        SpecRgbTex + Normal + Shadow + Lum 
        SpecRgbTex + Shadow +Lum
        radioSpecRgbTexParam
        skip_rest = true;
    }

    if (currentOptions_ &

    else if(radioAlphaUseChanFromDiffMapParam->isChecked())
    {
        currentOptions_ |= MWO_DiffuseAlpha;
        std::cout << "MWO_DiffuseAlpha" << std::endl;
    }

    // Alpha masking not usable if we are using MWO_AlphaNone or MWO_AlphaSolid.
    if (currentOptions_ & MWO_AlphaNone || currentOptions_ & MWO_AlphaSolid)
    {
        checkBoxAlphaMaskingParam->setEnabled(false);
        std::cout << "checkBoxAlphaMaskingParam->setEnabled(false);" << std::endl;
    }
    else
    {
        checkBoxAlphaMaskingParam->setEnabled(true);
        std::cout << "checkBoxAlphaMaskingParam->setEnabled(true);" << std::endl;
    }

    if (checkBoxAlphaMaskingParam->isEnabled() && checkBoxAlphaMaskingParam->isChecked())
    {
        currentOptions_ |= MWO_AlphaMasking;
        std::cout << "MWO_AlphaMasking" << std::endl;
    }

    // Misc
    // Animation available only if DIFFUSE_MAPPING and nothing else.
    if (currentOptions_ == Material_Diff || currentOptions_ == Material_DiffAnim)
    {
        checkBoxAnimParam->setEnabled(true);
        std::cout << "checkBoxAnimParam->setEnabled(true);" << std::endl;
    }
    else
    {
        checkBoxAnimParam->setEnabled(false);
        std::cout << "checkBoxAnimParam->setEnabled(false);" << std::endl;
    }

    if (checkBoxAnimParam->isEnabled() && checkBoxAnimParam->isChecked())
    {
        currentOptions_ |= MWO_Animation;
        std::cout << "MWO_Animation" << std::endl;
    }

    // Lightmap choosable with 1)normal, 2)normal+shadow, 3)shadow
    if (currentOptions_ == Material_DiffNormal || currentOptions_ == Material_DiffNormalShadow || currentOptions_ == Material_DiffShadow)
        checkBoxLightParam->setEnabled(true)
    else
        checkBoxLightParam->setEnabled(false)

    if (checkBoxLightParam->isEnabled() && checkBoxLightParam->isChecked())
    {
        currentOptions_ |= MWO_LightMap;
        std::cout << "MWO_LightMap" << std::endl;
    }

    // Luminance mapping choosable with 1)Specflat+Normal+Shadow 2) Specmap+Normal+Shadow
    if (currentOptions_ == Material_DiffSpecflatNormalShadowLum || currentOptions_ == DiffSpecmapNormalShadow ||
        currentOptions_ == Material_DiffSpecmapShadow)
        checkBoxLumParam->setEnabled(true)
    else
        checkBoxLumParam->setEnabled(false)

    if (checkBoxLumParam->isEnabled() && checkBoxLumParam->isChecked())
    {
        currentOptions_ |= MWO_LuminanceMap;
        std::cout << "MWO_LuminanceMap" << std::endl;
    }

    // Opa Choosable with 2)DiffSpecmapNormal 2)DiffSpecmapNormalShadowLum 3)DiffSpecmapShadowLum
    // 4)DiffSpecmapShadowOp
    if (
        checkBoxNormalParam->setEnabled(true)
    else
        checkBoxNormalParam->setEnabled(false)

    // Normal choosable with 1)DiffNormal 2)DiffNormalLightmap 3)DiffNormalShadow 4)DiffNormalShadowLightmap
    // 5)DiffSpecflatNormalShadowLum 6)DiffSpecmapNormal 7)DiffSpecmapNormalOpa 8)DiffSpecmapNormalShadow 9)DiffSpecmapNormalShadowLum
    // 10)DiffSpecmapNormalShadowLumOpa

    if (checkBoxNormalParam->isEnabled() && checkBoxNormalParam->isChecked())
    {
        currentOptions_ |= MWO_NormalMap;
        std::cout << "MWO_NormalMap" << std::endl;
    }

    // Reflection map choosable with 1)DiffReflAlpha 2)SpecmapRefl 3)DiffSpecmapShadowRefl
    if (checkBoxReflParam->isChecked())
    {
        currentOptions_ |= MWO_ReflectionMap;
        std::cout << "MWO_ReflectionMap" << std::endl;
    }

/*
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
*/
/*
    if (checkBoxShadowParam->isChecked())
    {
        currentOptions_ |= MWO_ReceivesShadows;
        std::cout << "MWO_ReceivesShadows" << std::endl;
    }
*/
/*
    // available only with DiffAlphamask
    QCheckBox *alphaMasking = mainWidget_->findChild<QCheckBox *>("checkBoxAlphaMasking");
    if (1)
        alphaMasking->setEnabled(false);
    else
        alphaMasking->setEnabled(true);
*/
/*
    //if (specSolid)
    //SSDiffSpecflatNormalShadowLum.material  SpecSolid + Normal + Shadow + Lum
    //SSDiffSpecflatShadow.material           SpecSolid + Shadow

    QLabel *labelCurrent = mainWidget_->findChild<QLabel *>("labelCurrent");
    labelCurrent->setText(GetCurrentMaterialFilename());
    */
    // OLD SHIT ENDS HERE
#endif
}

void MaterialWizard::ClearSelections()
{
    QList<QObject *> widgetList = mainWidget_->findChildren<QObject *>();
    QListIterator<QObject *> iter(widgetList);
    while(iter.hasNext())
    {
        QObject *obj = iter.next();
        QString name = obj->objectName();
        if (name.indexOf("Param") != -1)
        {
           QAbstractButton *button = dynamic_cast<QAbstractButton *>(obj);
            if (button)
            {
                // Set defaults checked and everything else unchecked.
                if (name == "radioDiffRgbTexParam" || name == "radioSpecNoneParam" || name == "radioAlphaNoneParam")
                    button->setChecked(true);
                else
                    button->setChecked(false);
            }
        }
    }

    QLineEdit *lineEditName = mainWidget_->findChild<QLineEdit *>("lineEditName");
    lineEditName->setText("");
}

QString MaterialWizard::GetCurrentMaterialFilename() const
{
    QString filename;

    switch(currentOptions_)
    {
    case Material_Diff:
        filename = "SSDiff";
        break;
    case Material_Diffa:
        filename = "SSDiffa";
        break;
    case Material_DiffAlphamask:
        filename = "SSDiffAlphamask";
        break;
    case Material_DiffAnim:
        filename = "SSDiffAnim";
        break;
    case Material_DiffNormal:
        filename = "SSDiffNormal";
        break;
    case Material_DiffNormalLightmap:
        filename = "SSDiffNormalLightmap";
        break;
    case Material_DiffNormalShadow:
        filename = "SSDiffNormalShadow";
        break;
    case Material_DiffNormalShadowLightmap:
        filename = "SSDiffNormalShadowLightmap";
        break;
    case Material_DiffReflAlpha:
        filename = "SSDiffReflAlpha";
        break;
    case Material_DiffShadow:
        filename = "SSDiffShadow";
        break;
    case Material_DiffShadowLightmap:
        filename = "SSDiffShadowLightmap";
        break;
    case Material_DiffSpecflatShadow:
        filename = "SSDiffSpecflatShadow";
        break;
    case Material_DiffSpecflatNormalShadowLum:
        filename = "SSDiffSpecflatNormalShadowLum";
        break;
    case Material_DiffSpecmap:
        filename = "SSDiffSpecmap";
        break;
    case Material_DiffSpecmapNormal:
        filename = "SSDiffSpecmapNormal";
        break;
    case Material_DiffSpecmapNormalOpa:
        filename = "SSDiffSpecmapNormalOpa";
        break;
    case Material_DiffSpecmapNormalShadow:
        filename = "SSDiffSpecmapNormalShadow";
        break;
    case Material_DiffSpecmapNormalShadowLum:
        filename = "SSDiffSpecmapNormalShadowLum";
        break;
    case Material_DiffSpecmapNormalShadowLumOpa:
        filename = "SSDiffSpecmapNormalShadowLumOpa";
        break;
    case Material_DiffSpecmapRefl:
        filename = "SSDiffSpecmapRefl";
        break;
    case Material_DiffSpecmapShadow:
        filename = "SSDiffSpecmapShadow";
        break;
    case Material_DiffSpecmapShadowLum:
        filename = "SSDiffSpecmapShadowLum";
        break;
    case Material_DiffSpecmapShadowLumOpa:
        filename = "SSDiffSpecmapShadowLumOpa";
        break;
    case Material_DiffSpecmapShadowOpa:
        filename = "SSDiffSpecmapShadowOpa";
        break;
    case Material_DiffSpecmapShadowRefl:
        filename = "SSDiffSpecmapShadowRefl";
        break;
    case Material_None:
    default:
        filename = "This combination is not currently supported!";
        break;
    }

    return filename;
}

void MaterialWizard::ValidateScriptName(const QString &name)
{
    QPushButton *buttonCreate = mainWidget_->findChild<QPushButton *>("buttonCreate");
    if (name.isEmpty() || name.isNull() || GetCurrentMaterialFilename().isEmpty())
        buttonCreate->setEnabled(false);
    else
        buttonCreate->setEnabled(true);

    scriptName_ = name;
}
