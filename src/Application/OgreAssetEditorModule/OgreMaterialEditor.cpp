/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   OgreMaterialEditor.cpp
    @brief  Non-text high-level editing tool for OGRE material scripts. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "OgreMaterialEditor.h"

#include "LoggingFunctions.h"
#include "Application.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "IAssetTransfer.h"
#include "OgreMaterialAsset.h"
#include "OgreMaterialUtils.h"
#include "PropertyTableWidget.h"
#include "Math/MathFunc.h"

#include <QUiLoader>

#include "MemoryLeakCheck.h"

static const char *cNoShader = "NoShader/Unknown";

OgreMaterialEditor::OgreMaterialEditor(const AssetPtr &materialAsset, Framework *fw, QWidget *parent) :
    QWidget(parent),
    framework(fw),
    asset(materialAsset),
    techniqueTabWidget(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    setLayout(layout);
    resize(450, 550);

    SetMaterialAsset(materialAsset);
}

OgreMaterialEditor::~OgreMaterialEditor()
{
}

void OgreMaterialEditor::SetMaterialAsset(const AssetPtr &scriptAsset)
{
    asset = scriptAsset;
    assert(asset.lock());
    AssetPtr assetPtr = asset.lock();
    if (!assetPtr)
    {
        LogError("OgreMaterialEditor: null asset given.");
        return;
    }
    if (assetPtr->Type() != "OgreMaterial")
    {
        LogError("OgreMaterialEditor::SetMaterialAsset: Unsupported asset type " + assetPtr->Type() + ".");
        return;
    }

    setWindowTitle(tr("Material Editor: ") + (assetPtr?assetPtr->Name():QString()));

    if (assetPtr && !assetPtr->IsLoaded())
    {
        AssetTransferPtr transfer = framework->Asset()->RequestAsset(assetPtr->Name(), assetPtr->Type(), true);
        connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), SLOT(OnAssetTransferSucceeded(AssetPtr)));
        connect(transfer.get(), SIGNAL(Failed(IAssetTransfer *, QString)), SLOT(OnAssetTransferFailed(IAssetTransfer *, QString)));
    }
    else
        Populate();
}

void OgreMaterialEditor::Populate()
{
    AssetPtr assetPtr = asset.lock();
    if (assetPtr && !assetPtr->IsLoaded())
    {
        LogError("OgreScriptEditor::Populate: asset not loaded.");
        return;
    }

    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(assetPtr.get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    const int numTechniques = mat->GetNumTechniques();
    if (numTechniques < 1)
    {
        LogError("OgreMaterialEditor: Invalid number of techiques (" + QString::number(numTechniques) + ") for material " + assetPtr->Name() + ".");
        return;
    }

    // Init technique tab widget
    SAFE_DELETE(techniqueTabWidget);
    techniqueTabWidget = new QTabWidget(this);
    static_cast<QVBoxLayout *>(layout())->addWidget(techniqueTabWidget);
    QVBoxLayout *techLayout = new QVBoxLayout(techniqueTabWidget);
    techLayout->setContentsMargins(5, 5, 5, 5);
    //techniqueTabWidget->setLayout(techLayout);

    // Init pass tab widget

//    static_cast<QVBoxLayout *>(layout())->addWidget(tabWidget);

//    techniqueTabWidget->addWidget(passTabWidget);

    QUiLoader loader;
    loader.setLanguageChangeEnabled(true);
    QFile passFile(Application::InstallationDirectory() + "data/ui/MaterialEditorPassTab.ui");
    if (!passFile.exists())
    {
        LogError("OgreMaterialEditor: Cannot find material editor pass ui file!");
        return;
    }
    QFile tuFile(Application::InstallationDirectory() + "data/ui/MaterialEditorTuTab.ui");
    if (!tuFile.exists())
    {
        LogError("OgreMaterialEditor: Cannot find material editor texture unit ui file!");
        return;
    }

    for(int techIndex = 0; techIndex < numTechniques; ++techIndex)
    {
        QTabWidget *passTabWidget = 0;
        const int numPasses = mat->GetNumPasses(techIndex);
        if (numPasses > 0)
        {
            passTabWidget = new QTabWidget(techniqueTabWidget);
            QVBoxLayout *passLayout = new QVBoxLayout(passTabWidget);
            passLayout->setContentsMargins(5, 5, 5, 5);
            passTabWidget->setLayout(passLayout);
        }
        for(int passIndex = 0; passIndex < numPasses; ++passIndex)
        {
            QFile passFile(Application::InstallationDirectory() + "data/ui/MaterialEditorPassTab.ui");
            QWidget *passWidget = loader.load(&passFile, passTabWidget);
            passFile.close();

            techniqueTabWidget->addTab(passTabWidget, "Technique" + QString::number(techIndex));
            passTabWidget->addTab(passWidget, "Pass" + QString::number(passIndex));

            const QString techniquePassId = ";" + QString::number(techIndex) + ";" + QString::number(passIndex);
            // Gather the initial values.
            Color ambientColor = mat->AmbientColor(techIndex, passIndex);
            Color diffuseColor = mat->DiffuseColor(techIndex, passIndex);
            Color specularColor = mat->SpecularColor(techIndex, passIndex);
            Color emissiveColor = mat->EmissiveColor(techIndex, passIndex);
            unsigned srcFactor = mat->SourceSceneBlendFactor(techIndex, passIndex);
            unsigned dstFactor = mat->DestinationSceneBlendFactor(techIndex, passIndex);
            bool depthCheck = mat->IsDepthCheckEnabled(techIndex, passIndex);
            bool depthWrite = mat->IsDepthWriteEnabled(techIndex, passIndex);
            float depthBias = mat->DepthBias(techIndex, passIndex);
            bool lighting = mat->IsLightingEnabled(techIndex, passIndex);
            unsigned shadingMode = mat->ShadingMode(techIndex, passIndex);
            unsigned fillMode = mat->FillMode(techIndex, passIndex);
            unsigned cullingMode = mat->HardwareCullingMode(techIndex, passIndex);
            bool colorWrite = mat->IsColorWriteEnabled(techIndex, passIndex);
            //QString pixelShader = mat->PixelShader(techIndex, passIndex);
            QString vertexShader = mat->VertexShader(techIndex, passIndex);

            // Ambient color
            QPixmap colorIcon(24,24);
            colorIcon.fill(ambientColor);
            QPushButton *btn = passWidget->findChild<QPushButton *>("ambientColor");
            btn->setObjectName(btn->objectName() + techniquePassId);
            btn->setIcon(colorIcon);
            connect(btn, SIGNAL(clicked()), SLOT(OpenColorPicker()), Qt::UniqueConnection);
            ///\todo SuperShaders don't currently support color. Disable color widgets for them.
            if (vertexShader.contains("rex/"))
            {
                btn->hide();
                passWidget->findChild<QLabel *>("ambientLabel")->hide();
            }

            // Diffuse color
            colorIcon.fill(diffuseColor);
            btn = passWidget->findChild<QPushButton *>("diffuseColor");
            btn->setObjectName(btn->objectName() + techniquePassId);
            btn->setIcon(colorIcon);
            connect(btn, SIGNAL(clicked()), SLOT(OpenColorPicker()), Qt::UniqueConnection);
            ///\todo SuperShaders don't currently support color. Disable color widgets for them.
            if (vertexShader.contains("rex/"))
            {
                btn->hide();
                passWidget->findChild<QLabel *>("diffuseLabel")->hide();
            }

            // Specular color
            colorIcon.fill(specularColor);
            btn = passWidget->findChild<QPushButton *>("specularColor");
            btn->setObjectName(btn->objectName() + techniquePassId);
            btn->setIcon(colorIcon);
            connect(btn, SIGNAL(clicked()), SLOT(OpenColorPicker()), Qt::UniqueConnection);
            ///\todo SuperShaders don't currently support color. Disable color widgets for them.
            if (vertexShader.contains("rex/"))
            {
                btn->hide();
                passWidget->findChild<QLabel *>("specularLabel")->hide();
            }

            // Emissive color
            colorIcon.fill(emissiveColor);
            btn = passWidget->findChild<QPushButton *>("emissiveColor");
            btn->setObjectName(btn->objectName() + techniquePassId);
            btn->setIcon(colorIcon);
            connect(btn, SIGNAL(clicked()), SLOT(OpenColorPicker()), Qt::UniqueConnection);
            ///\todo SuperShaders don't currently support color. Disable color widgets for them.
            if (vertexShader.contains("rex/"))
            {
                btn->hide();
                passWidget->findChild<QLabel *>("emissiveLabel")->hide();
            }

            // Append technique and pass indices to the object names so we can keep track of them when setting new values.
            QComboBox *srcBlendComboBox = passWidget->findChild<QComboBox *>("srcBlendComboBox");
            srcBlendComboBox->setObjectName(srcBlendComboBox->objectName() + techniquePassId);
            QComboBox *dstBlendComboBox = passWidget->findChild<QComboBox *>("dstBlendComboBox");
            dstBlendComboBox->setObjectName(dstBlendComboBox->objectName() + techniquePassId);

            QStringList blendFactors(QStringList() << "one" << "zero" << "dest_colour" << "src_colour" << "one_minus_dest_colour" <<
                "one_minus_src_colour" << "dest_alpha" << "src_alpha" << "one_minus_dest_alpha" << "one_minus_src_alpha");
            srcBlendComboBox->addItems(blendFactors);
            dstBlendComboBox ->addItems(blendFactors);
            srcBlendComboBox->setCurrentIndex(srcFactor); // For these the Ogre enum values map directly to the index in the combo box.
            dstBlendComboBox->setCurrentIndex(dstFactor);

            QCheckBox *depthTestCheckBox = passWidget->findChild<QCheckBox *>("depthTestCheckBox");
            depthTestCheckBox->setObjectName(depthTestCheckBox->objectName() + techniquePassId);
            depthTestCheckBox->setChecked(depthCheck);

            QCheckBox *depthWriteCheckBox = passWidget->findChild<QCheckBox *>("depthWriteCheckBox");
            depthWriteCheckBox->setObjectName(depthWriteCheckBox->objectName() + techniquePassId);
            depthTestCheckBox->setChecked(depthWrite);

            QDoubleSpinBox *depthBiasSpinBox = passWidget->findChild<QDoubleSpinBox *>("depthBiasSpinBox");
            depthBiasSpinBox->setObjectName(depthBiasSpinBox->objectName() + techniquePassId);
            depthBiasSpinBox->setValue(depthBias);

            QComboBox *cullingComboBox = passWidget->findChild<QComboBox *>("cullingComboBox");
            cullingComboBox->setObjectName(cullingComboBox->objectName() + techniquePassId);
            QStringList hwCullingModes(QStringList() << "clockwise" << "anticlockwise" << "none");
            cullingComboBox->addItems(hwCullingModes);
            cullingComboBox->setCurrentIndex(cullingMode-1); // Ogre odditities #n: enum values are 1-3

            QCheckBox *lightingCheckBox = passWidget->findChild<QCheckBox *>("lightingCheckBox");
            lightingCheckBox->setObjectName(lightingCheckBox->objectName() + techniquePassId);
            lightingCheckBox->setChecked(lighting);

            QComboBox *shadingComboBox = passWidget->findChild<QComboBox *>("shadingComboBox");
            shadingComboBox->setObjectName(shadingComboBox->objectName() + techniquePassId);
            QStringList shadingModes(QStringList() << "flat" << "gouraud" << "phong");
            shadingComboBox->addItems(shadingModes);
            shadingComboBox->setCurrentIndex(shadingMode);

            QComboBox *fillComboBox = passWidget->findChild<QComboBox *>("fillComboBox");
            fillComboBox->setObjectName(fillComboBox->objectName() + techniquePassId);
            QStringList fillModes(QStringList() << "points" << "wireframe" << "solid" );
            fillComboBox->addItems(fillModes);
            fillComboBox->setCurrentIndex(fillMode-1); // Ogre odditities #n: enum values are 1-3

            QCheckBox *colorWriteCheckBox = passWidget->findChild<QCheckBox *>("colorWriteCheckBox");
            colorWriteCheckBox->setObjectName(colorWriteCheckBox->objectName() + techniquePassId);
            colorWriteCheckBox->setChecked(colorWrite);

            QComboBox *shaderComboBox = passWidget->findChild<QComboBox *>("shaderComboBox");
             ///\todo Enable
            shaderComboBox->setDisabled(true);
            shaderComboBox->setObjectName(shaderComboBox->objectName() + techniquePassId);
            ///\todo Currently we handle VP and FP as inseparable pairs. Support for choosing different VP than FP will be added later on.
            ///\todo Currently uses the Ogre resources as source for shader. Use ShaderAsset if/when it exists.
            QSet<QString> shaders;
            Ogre::ResourceManager::ResourceMapIterator iter = Ogre::HighLevelGpuProgramManager::getSingleton().getResourceIterator();
            while(iter.hasMoreElements())
            {
                Ogre::ResourcePtr resource = iter.getNext();
                if (QString(resource->getOrigin().c_str()).contains("SuperShader.program"))
                {
                    QString shader = QString(resource->getName().c_str());
                    shader.replace("rex/", "");
                    shader.replace("VP", "");
                    shader.replace("FP", "");
                    shaders << shader;
                }
            }
            QStringList shaderList = shaders.toList();
            shaderList.push_front(cNoShader);

            vertexShader.replace("rex/", "");
            vertexShader.replace("VP", "");
            shaderComboBox->addItems(shaderList);
            for(int idx = 0; idx < shaderComboBox->count(); ++idx)
                if (shaderComboBox->itemText(idx) == vertexShader)
                {
                    shaderComboBox->setCurrentIndex(idx);
                    break;
                }

            // Populate shader attributes (if any)
            PopulateShaderAttributes(techIndex, passIndex);

            connect(srcBlendComboBox, SIGNAL(currentIndexChanged(int)), SLOT(SetSrcBlendFactor(int)), Qt::UniqueConnection);
            connect(dstBlendComboBox, SIGNAL(currentIndexChanged(int)), SLOT(SetDstBlendFactor(int)), Qt::UniqueConnection);
            connect(depthTestCheckBox, SIGNAL(stateChanged(int)), SLOT(SetDepthTest(int)), Qt::UniqueConnection);
            connect(depthWriteCheckBox, SIGNAL(stateChanged(int)), SLOT(SetDepthWrite(int)), Qt::UniqueConnection);
            connect(depthBiasSpinBox, SIGNAL(valueChanged(double)), SLOT(SetDepthBias(double)), Qt::UniqueConnection);
            connect(cullingComboBox, SIGNAL(currentIndexChanged(int)), SLOT(SetCullingMode(int)), Qt::UniqueConnection);
            connect(lightingCheckBox, SIGNAL(stateChanged(int)), SLOT(SetLighting(int)), Qt::UniqueConnection);
            connect(shadingComboBox, SIGNAL(currentIndexChanged(int)), SLOT(SetShadingMode(int)), Qt::UniqueConnection);
            connect(fillComboBox, SIGNAL(currentIndexChanged(int)), SLOT(SetFillMode(int)), Qt::UniqueConnection);
            connect(colorWriteCheckBox, SIGNAL(stateChanged(int)), SLOT(SetColorWrite(int)), Qt::UniqueConnection);
            connect(shaderComboBox, SIGNAL(currentIndexChanged(const QString &)), SLOT(SetShader(const QString &)), Qt::UniqueConnection);

            // Populate texture units (if any)
            PopulateTextureUnits(techIndex, passIndex);
        }
    }

    static_cast<QVBoxLayout *>(layout())->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

void OgreMaterialEditor::OpenColorPicker()
{
    assert(sender());
    if (!sender())
        return;

    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 3)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    Color initialColor;
    if (indices.first().contains("ambient"))
        initialColor = mat->AmbientColor(techIndex, passIndex);
    else if (indices.first().contains("diffuse"))
        mat->DiffuseColor(techIndex, passIndex);
    else if (indices.first().contains("specular"))
        mat->SpecularColor(techIndex, passIndex);
    else if (indices.first().contains("emissive"))
        mat->EmissiveColor(techIndex, passIndex);

    // Qt bug: if initial color is black the dialog doens't emit currentColorChanged until some color
    // is selected from the ready palette of colors instead of the "free selection" of colors.
    QColorDialog *dialog = 0;
    if (initialColor == Color())
        dialog = new QColorDialog(this);
    else
        dialog = new QColorDialog(initialColor, this);
    dialog->setObjectName(sender()->objectName());
    dialog->setOption(QColorDialog::ShowAlphaChannel);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, SIGNAL(currentColorChanged(const QColor &)), SLOT(SetColor(const QColor &)), Qt::UniqueConnection);
    dialog->show();
}

void OgreMaterialEditor::SetColor(const QColor &color)
{
    if (!qobject_cast<QColorDialog *>(sender()))
        return;

    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 3)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    if (indices.first().contains("ambient"))
    {
        mat->SetAmbientColor(techIndex, passIndex, color);
        LogDebug("SetEmissiveColor: " + mat->AmbientColor(techIndex, passIndex));
    }
    else if (indices.first().contains("diffuse"))
    {
        mat->SetDiffuseColor(techIndex, passIndex, color);
        LogDebug("DiffuseColor: " + mat->DiffuseColor(techIndex, passIndex));
    }
    else if (indices.first().contains("specular"))
    {
        mat->SetSpecularColor(techIndex, passIndex, color);
        LogDebug("SpecularColor: " + mat->SpecularColor(techIndex, passIndex));
    }
    else if (indices.first().contains("emissive"))
    {
        mat->SetEmissiveColor(techIndex, passIndex, color);
        LogDebug("SetEmissiveColor: " + mat->EmissiveColor(techIndex, passIndex));
    }

    QPushButton *colorButton = findChild<QPushButton *>(sender()->objectName());
    if (!colorButton)
        return;

    QPixmap colorIcon(24,24);
    colorIcon.fill(color);
    colorButton->setIcon(colorIcon);
}

void OgreMaterialEditor::SetSrcBlendFactor(int mode)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 3)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    mat->SetSceneBlend(techIndex, passIndex, mode, mat->DestinationSceneBlendFactor(techIndex, passIndex));
    LogDebug("SetSrcBlendFactor: " + QString::number(mat->SourceSceneBlendFactor(techIndex, passIndex)));
}

void OgreMaterialEditor::SetDstBlendFactor(int mode)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 3)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    mat->SetSceneBlend(techIndex, passIndex, mat->SourceSceneBlendFactor(techIndex, passIndex), mode);
    LogDebug("SetDstBlendFactor: " + QString::number(mat->DestinationSceneBlendFactor(techIndex, passIndex)));
}

void OgreMaterialEditor::SetDepthTest(int enable)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 3)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    mat->SetDepthCheck(techIndex, passIndex, (bool)enable);
    LogDebug("SetDepthTest: " + QString::number(mat->IsDepthCheckEnabled(techIndex, passIndex)));
}

void OgreMaterialEditor::SetDepthWrite(int enable)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 3)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    mat->SetDepthWrite(techIndex, passIndex, (bool)enable);
    LogDebug("SetDepthWrite: " + QString::number(mat->IsDepthWriteEnabled(techIndex, passIndex)));
}

void OgreMaterialEditor::SetDepthBias(double value)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 3)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    mat->SetDepthBias(techIndex, passIndex, (float)value);
    LogDebug("SetDepthBias: " + QString::number((float)mat->DepthBias(techIndex, passIndex)));
}

void OgreMaterialEditor::SetCullingMode(int mode)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 3)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    // Ogre odditities #n: enum values are 1-3
    mat->SetHardwareCullingMode(techIndex, passIndex, mode+1);
    LogDebug("SetCullingMode: " + QString::number(mat->HardwareCullingMode(techIndex, passIndex)));
}

void OgreMaterialEditor::SetLighting(int enable)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 3)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    mat->SetLighting(techIndex, passIndex, (bool)enable);
    LogDebug("SetLighting: " + QString::number(mat->IsLightingEnabled(techIndex, passIndex)));
}

void OgreMaterialEditor::SetShadingMode(int mode)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 3)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    mat->SetShadingMode(techIndex, passIndex, mode);
    LogDebug("SetShadingMode: " + QString::number(mat->ShadingMode(techIndex, passIndex)));
}

void OgreMaterialEditor::SetFillMode(int mode)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 3)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    // Ogre odditities #n: enum values are 1-3
    mat->SetFillMode(techIndex, passIndex, mode+1);
    LogDebug("SetFillMode: " + QString::number(mat->FillMode(techIndex, passIndex)));
}

void OgreMaterialEditor::SetColorWrite(int enable)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 3)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    mat->SetColorWrite(techIndex, passIndex, (bool)enable);
    LogDebug("SetColorWrite: " + QString::number(mat->IsColorWriteEnabled(techIndex, passIndex)));
}

void OgreMaterialEditor::SetShader(const QString &shader)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 3)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    if (shader != cNoShader)
    {
        QString vertexShader = shader;
        QString pixelShader = shader;
        vertexShader.prepend("rex/");
        vertexShader.append("VP");
        pixelShader.prepend("rex/");
        pixelShader.append("FP");
        mat->SetVertexShader(techIndex, passIndex, vertexShader);
        mat->SetPixelShader(techIndex, passIndex, pixelShader);

        LogDebug("SetShader vertex: " + mat->VertexShader(techIndex, passIndex));
        LogDebug("SetShader pixel: " + mat->PixelShader(techIndex, passIndex));
    }

    PopulateShaderAttributes(techIndex, passIndex);

    ///\todo
    // Shader template changed, so so will texture units. Remove the old ones and generate new ones.
/*
    const int numOldTus = mat->GetNumTextureUnits(techIndex, passIndex);
    for(int tuIndex = 0; tuIndex < numOldTus; ++tuIndex)
        mat->RemoveTextureUnit(techIndex, passIndex, tuIndex);
    mat->CreateTextureUnit(
    const int ti = 0;
    ///\todo Currently only one technique supported
//    for(int techIndex = 0; techIndex < numTechniques; ++techIndex)
    {
        const int numPasses = mat->GetNumPasses(techIndex);
        for(int pi = 0; passIndex < numPasses; ++passIndex)
            PopulateTextureUnits(ti, pi);
    }
*/
}

void OgreMaterialEditor::SetShaderAttributeValue()
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 4)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }

    QString attrName = indices[0].split(" ")[0];
    QString type = indices[0].split(" ")[1];
    int row = indices[1].toInt(), techIndex = indices[2].toInt(), passIndex = indices[3].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    QWidget *parentPassTab = static_cast<QTabWidget *>(techniqueTabWidget->widget(techIndex))->widget(passIndex);
    PropertyTableWidget *shaderAttributeTable = parentPassTab->findChild<PropertyTableWidget *>("shaderAttributeTable");
    // Gather values for the entire row.
    QVariantList value;
    for(int column = 0; column < shaderAttributeTable->columnCount(); ++column)
    {
        QTableWidgetItem *item = shaderAttributeTable->item(row, column);
        if ((item->flags() & Qt::ItemIsEditable) != 0)
            value.push_back(item->data(Qt::DisplayRole));
    }

    if (type.contains("VP"))
        mat->SetVertexShaderParameter(techIndex, passIndex, attrName, value);
    else if (type.contains("FP"))
        mat->SetPixelShaderParameter(techIndex, passIndex, attrName, value);
    else
        LogError("SetShaderAttribute: Invalid type identifier: " + type);

}

void OgreMaterialEditor::SetTexAssetRef()
{
    QLineEdit *assetRefLineEdit = dynamic_cast<QLineEdit *>(sender());
    assert(assetRefLineEdit);
    if (!assetRefLineEdit)
        return;

    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 4)
    {
        LogError("Invalid object name " + assetRefLineEdit->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt(), tuIndex = indices[3].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    mat->SetTexture(techIndex, passIndex, tuIndex, assetRefLineEdit->text());
    LogDebug("SetTexAssetRef: " + mat->Texture(techIndex, passIndex, tuIndex));
}

void OgreMaterialEditor::SetTexCoordSet(int value)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");
    if (indices.size() != 4)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt(), tuIndex = indices[3].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    mat->SetTextureCoordSet(techIndex, passIndex, tuIndex, value);
    LogDebug("SetTexCoordSet: " + QString::number(mat->TextureCoordSet(techIndex, passIndex, tuIndex)));
}

void OgreMaterialEditor::SetAddrMode(int mode)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");

    if (indices.size() != 4)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt(), tuIndex = indices[3].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    mat->SetTextureAddressingMode(techIndex, passIndex, tuIndex, mode);
    LogDebug("SetAddrMode u " + QString::number(mat->TextureAddressingModeU(techIndex, passIndex, tuIndex)) + 
        " v " + QString::number(mat->TextureAddressingModeV(techIndex, passIndex, tuIndex)) +
        " w " + QString::number(mat->TextureAddressingModeW(techIndex, passIndex, tuIndex)));
}

void OgreMaterialEditor::SetScrollAnimU(double value)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");

    if (indices.size() != 4)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt(), tuIndex = indices[3].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    bool hasUV = mat->HasTextureEffect(techIndex, passIndex, tuIndex, Ogre::TextureUnitState::ET_UVSCROLL);
    bool hasV = mat->HasTextureEffect(techIndex, passIndex, tuIndex, Ogre::TextureUnitState::ET_VSCROLL);
    float uSpeed = (float)value, vSpeed = 0.f;
    if (hasUV)
        vSpeed = uSpeed;
    else if (hasV)
        vSpeed = mat->ScrollAnimationU(techIndex, passIndex, tuIndex);
    mat->SetScrollAnimation(techIndex, passIndex, tuIndex, uSpeed, vSpeed);
    LogDebug("SetScrollAnimU: u " + QString::number(mat->ScrollAnimationU(techIndex, passIndex, tuIndex)));
    LogDebug("SetScrollAnimU: v " + QString::number(mat->ScrollAnimationV(techIndex, passIndex, tuIndex)));
}

void OgreMaterialEditor::SetScrollAnimV(double value)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");

    if (indices.size() != 4)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt(), tuIndex = indices[3].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    bool hasUV = mat->HasTextureEffect(techIndex, passIndex, tuIndex, Ogre::TextureUnitState::ET_UVSCROLL);
    bool hasU = mat->HasTextureEffect(techIndex, passIndex, tuIndex, Ogre::TextureUnitState::ET_USCROLL);
    float uSpeed = 0.f, vSpeed = (float)value;
    if (hasUV)
        uSpeed = vSpeed;
    else if (hasU)
        uSpeed = mat->ScrollAnimationU(techIndex, passIndex, tuIndex);

    mat->SetScrollAnimation(techIndex, passIndex, tuIndex, uSpeed, vSpeed);
    LogDebug("SetScrollAnimV: u " + QString::number(mat->ScrollAnimationU(techIndex, passIndex, tuIndex)));
    LogDebug("SetScrollAnimV: v " + QString::number(mat->ScrollAnimationV(techIndex, passIndex, tuIndex)));
}

void OgreMaterialEditor::SetRotateAnim(double value)
{
    assert(sender());
    if (!sender())
        return;
    QStringList indices = sender()->objectName().split(";");

    if (indices.size() != 4)
    {
        LogError("Invalid object name " + sender()->objectName());
        return;
    }
    int techIndex = indices[1].toInt(), passIndex = indices[2].toInt(), tuIndex = indices[3].toInt();
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    mat->SetRotateAnimation(techIndex, passIndex, tuIndex, (float)value);
    LogDebug("SetRotateAnimation " + QString::number(mat->RotateAnimation(techIndex, passIndex, tuIndex)));
}

void OgreMaterialEditor::PopulateShaderAttributes(int techIndex, int passIndex)
{
    OgreMaterialAsset *mat = dynamic_cast<OgreMaterialAsset *>(asset.lock().get());
    if (!mat)
    {
        LogError("OgreMaterialEditor: Invalid asset.");
        return;
    }

    QLabel *label = techniqueTabWidget->widget(techIndex)->findChild<QLabel *>("shaderAttributesLabel");
    if (label)
        label->hide();

    OgreRenderer::ShaderParameterMap propMap = OgreRenderer::GatherShaderParameters(mat->ogreMaterial);
    OgreRenderer::ShaderParameterMapIter it(propMap);
    if (propMap.size())
    {
        if (label)
            label->show();

        QWidget *parentPassTab = static_cast<QTabWidget *>(techniqueTabWidget->widget(techIndex))->widget(passIndex);
        // 16 values is the biggest possibile for attribute (float4x4)
        PropertyTableWidget *shaderAttributeTable = new PropertyTableWidget(propMap.size(), 2+16, parentPassTab);
        shaderAttributeTable->setObjectName("shaderAttributeTable");
        QTabWidget *passTabWidget = static_cast<QTabWidget *>(techniqueTabWidget->widget(techIndex));
        static_cast<QGridLayout *>(passTabWidget->widget(passIndex)->layout())->addWidget(shaderAttributeTable);

        size_t numColumns = 3;
        int row = 0;
        while(it.hasNext())
        {
            it.next();
            QMap<QString, QVariant> typeValuePair = it.value().toMap();
            // Property name, set non-editable.
            QTableWidgetItem *nameItem = new QTableWidgetItem(it.key());
            nameItem->setFlags(Qt::ItemIsEnabled);
            // Property type, set non-editable.
            Ogre::GpuConstantType type = (Ogre::GpuConstantType)typeValuePair.begin().key().toInt();
            QTableWidgetItem *typeItem = new QTableWidgetItem(OgreRenderer::GpuConstantTypeToString(type));
            typeItem->setFlags(Qt::ItemIsEnabled);
            // Property value
            QTableWidgetItem *valueItem = new QTableWidgetItem;
            // Disable drop support for non-texture properties.
            if (nameItem->text().indexOf(" TU") == -1)
            {
                Qt::ItemFlags flags = valueItem->flags();
                flags &= ~Qt::ItemIsDropEnabled;
                valueItem->setFlags(flags);
            }

            //valueItem->setData(Qt::DisplayRole, typeValuePair.begin().value());

            shaderAttributeTable->setItem(row, 0, nameItem);
            shaderAttributeTable->setItem(row, 1, typeItem);

            // The number of elements we really want to show
            size_t numElems = Ogre::GpuConstantDefinition::getElementSize(type, false);
            QStringList values = typeValuePair.begin().value().toString().split(" ");
            for(int i = 0; i  < values.size(); ++i)
            {
                QTableWidgetItem *valueItem = new QTableWidgetItem;
                if (i < numElems)
                    valueItem->setData(Qt::DisplayRole, values[i]);
                else
                    valueItem->setFlags(Qt::ItemIsEnabled); // Disable editing for surplus cells.
                shaderAttributeTable->setItem(row, i+2, valueItem);

                // Set custom editor delegate
                SpinBoxDelegate *spinBoxDelegate = new SpinBoxDelegate(Ogre::GpuConstantDefinition::isFloat(type), shaderAttributeTable);
                // Set special identifying object name: 'name;rowIndex;techIndex;passIndex'
                spinBoxDelegate->setObjectName(it.key() + QString(";%1;%2;%3)").arg(row).arg(techIndex).arg(passIndex));
                shaderAttributeTable->setItemDelegateForRow(row, spinBoxDelegate);
                connect(spinBoxDelegate, SIGNAL(closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)), SLOT(SetShaderAttributeValue()), Qt::UniqueConnection);
            }

            numColumns = Max(numColumns, numElems+2);
            ++row;
        }

        // Resize to fit the value with most elements.
        shaderAttributeTable->setColumnCount((int)numColumns);

        QStringList labels;
        labels << tr("Name") << tr("Type") << tr("Value") + "[0]";
        for(int i = 2;  i < shaderAttributeTable->columnCount(); ++i)
            labels << "Value[" + QString::number(i-1) + "]";
        shaderAttributeTable->setHorizontalHeaderLabels(labels);

        shaderAttributeTable->show();
    }
}

void OgreMaterialEditor::PopulateTextureUnits(int techIndex, int passIndex)
{
    QTabWidget *tuTabWidget = 0;

    OgreMaterialAsset *mat = static_cast<OgreMaterialAsset *>(asset.lock().get());
    const int numTus = mat->GetNumTextureUnits(techIndex, passIndex);
    if (numTus > 0)
    {
        QTabWidget *parentPassTab = static_cast<QTabWidget *>(techniqueTabWidget->widget(techIndex));
        QWidget *parentPassWidget = static_cast<QWidget *>(parentPassTab->widget(passIndex));
        tuTabWidget = new QTabWidget(parentPassWidget);
        tuTabWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        static_cast<QVBoxLayout *>(parentPassWidget->layout())->addWidget(tuTabWidget);
    }
    for(int tuIndex = 0; tuIndex < numTus; ++tuIndex)
    {
        Ogre::TextureUnitState* tu = mat->GetTextureUnit(techIndex, passIndex, tuIndex);
        assert(tu);
        if (!tu)
            continue;

        QFile tuFile(Application::InstallationDirectory() + "data/ui/MaterialEditorTuTab.ui");
        QUiLoader loader;
        loader.setLanguageChangeEnabled(true);
        QWidget *tuWidget = loader.load(&tuFile, tuTabWidget);
        tuFile.close();
        assert(tuWidget);
        //tuWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        const QString techniquePassTuId = QString(";%1;%2;%3").arg(techIndex).arg(passIndex).arg(tuIndex);

        QString assetRef = AssetAPI::DesanitateAssetRef(tu->getTextureName()).c_str();
        uint texCoordSet = mat->TextureCoordSet(techIndex, passIndex, tuIndex);
        unsigned addrModeU = mat->TextureAddressingModeU(techIndex, passIndex, tuIndex);
        unsigned addrModeV = mat->TextureAddressingModeV(techIndex, passIndex, tuIndex);
        unsigned addrModeW = mat->TextureAddressingModeW(techIndex, passIndex, tuIndex);
        if (!(addrModeU == addrModeV && addrModeU == addrModeW))
            LogWarning("OgreMaterialEditor: Non-uniform texture addressing modes for material " + mat->Name() + ".");

        // Ogre weirdness: if texture has scroll effect of which u and v values are the same, it has Ogre ET_UVSCROLL.
        // If it has scroll effect with different u and v values it has both ET_USCROLL and ET_VSCROLL.
        bool uvScrollAnimEnabled = mat->HasTextureEffect(techIndex, passIndex, tuIndex, Ogre::TextureUnitState::ET_UVSCROLL);
        bool uScrollAnimEnabled = mat->HasTextureEffect(techIndex, passIndex, tuIndex, Ogre::TextureUnitState::ET_USCROLL);
        bool vScrollAnimEnabled = mat->HasTextureEffect(techIndex, passIndex, tuIndex, Ogre::TextureUnitState::ET_VSCROLL);
        bool rotateAnimEnabled = mat->HasTextureEffect(techIndex, passIndex, tuIndex, Ogre::TextureUnitState::ET_ROTATE);
        bool scrollAnimEnabled = (uvScrollAnimEnabled || (uScrollAnimEnabled && vScrollAnimEnabled));
        float scrollU = 0.f, scrollV = 0.f, rotate = 0.f;
        if (uvScrollAnimEnabled)
        {
            scrollU = mat->ScrollAnimationU(techIndex, passIndex, tuIndex);
            scrollV = scrollU;
        }
        else if (uScrollAnimEnabled && vScrollAnimEnabled)
        {
            scrollU = mat->ScrollAnimationU(techIndex, passIndex, tuIndex);
            scrollV = mat->ScrollAnimationV(techIndex, passIndex, tuIndex);
        }

        if (rotateAnimEnabled)
            rotate = mat->RotateAnimation(techIndex, passIndex, tuIndex);

        // Asset ref
        QLineEdit *assetRefLineEdit = tuWidget->findChild<QLineEdit *>("assetRefLineEdit");
        assetRefLineEdit->setObjectName(assetRefLineEdit->objectName() + techniquePassTuId);
        assetRefLineEdit->setText(AssetAPI::DesanitateAssetRef(tu->getTextureName()).c_str());
        if (assetRefLineEdit->text().trimmed().isEmpty())
            assetRefLineEdit->setDisabled(true);

        // Tex coord set
        QSpinBox *texCoordSetSpinBox = tuWidget->findChild<QSpinBox *>("texCoordSetSpinBox");
        texCoordSetSpinBox->setObjectName(texCoordSetSpinBox->objectName() + techniquePassTuId);
        texCoordSetSpinBox->setValue(texCoordSet);

        // Addr mode
        QComboBox *addrModeComboBox = tuWidget->findChild<QComboBox *>("addrModeComboBox");
        addrModeComboBox->setObjectName(addrModeComboBox->objectName() + techniquePassTuId);

        QStringList addrModes(QStringList() << "wrap" << "mirror" << "clamp" /*<< "border"*/); // Note: border omitted intentionally.
        addrModeComboBox->addItems(addrModes);
        if (addrModeU > 3)
            LogWarning("Material uses tex addr mode 'border', which will not be shown in material editor combo box.");
        else
            addrModeComboBox->setCurrentIndex(addrModeU);

        // Scroll anim
        QDoubleSpinBox *scrollAnimUSpinBox = tuWidget->findChild<QDoubleSpinBox *>("scrollAnimUSpinBox");
        QDoubleSpinBox *scrollAnimVSpinBox = tuWidget->findChild<QDoubleSpinBox *>("scrollAnimVSpinBox");
        scrollAnimUSpinBox->setObjectName(scrollAnimUSpinBox->objectName() + techniquePassTuId);
        scrollAnimVSpinBox->setObjectName(scrollAnimVSpinBox->objectName() + techniquePassTuId);
        if (scrollAnimEnabled)
        {
            scrollAnimUSpinBox->setValue(scrollU);
            scrollAnimVSpinBox->setValue(scrollV);
        }
        else
        {
            scrollAnimUSpinBox->setDisabled(true);
            scrollAnimVSpinBox->setDisabled(true);
        }

        // Rotate anim
        QDoubleSpinBox *rotateAnimSpinBox = tuWidget->findChild<QDoubleSpinBox *>("rotateAnimSpinBox");
        rotateAnimSpinBox->setObjectName(rotateAnimSpinBox->objectName() + techniquePassTuId);
        if (rotateAnimEnabled)
            rotateAnimSpinBox->setValue(rotate);
        else
            rotateAnimSpinBox->setDisabled(true);

        connect(assetRefLineEdit, SIGNAL(editingFinished()), SLOT(SetTexAssetRef()), Qt::UniqueConnection);
        connect(texCoordSetSpinBox, SIGNAL(valueChanged(int)), SLOT(SetTexCoordSet(int)), Qt::UniqueConnection);
        connect(addrModeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(SetAddrMode(int)), Qt::UniqueConnection);
        connect(scrollAnimUSpinBox, SIGNAL(valueChanged(double)), SLOT(SetScrollAnimU(double)), Qt::UniqueConnection);
        connect(scrollAnimVSpinBox, SIGNAL(valueChanged(double)), SLOT(SetScrollAnimV(double)), Qt::UniqueConnection);
        connect(rotateAnimSpinBox, SIGNAL(valueChanged(double)), SLOT(SetRotateAnim(double)), Qt::UniqueConnection);

//        QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
//        static_cast<QVBoxLayout *>(tuWidget->layout())->addSpacerItem(spacer);

        tuTabWidget->addTab(tuWidget, tu->getName().c_str());
    }
}

void OgreMaterialEditor::OnAssetTransferSucceeded(AssetPtr scriptAsset)
{
    SetMaterialAsset(scriptAsset);
}

void OgreMaterialEditor::OnAssetTransferFailed(IAssetTransfer *transfer, QString reason)
{
    LogError("OgreMaterialEditor::OnAssetTransferFailed: " + reason);
}

// SpinBoxDelegate

SpinBoxDelegate::SpinBoxDelegate(bool floatingPoint_, QObject *parent) :
    QItemDelegate(parent),
    floatingPoint(floatingPoint_)
{
}

QWidget *SpinBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
    QWidget *w = 0;
    if (floatingPoint)
    {
        QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
        editor->setMinimum(0.0);
        editor->setMaximum(10000.0);
        editor->setSingleStep(0.001);
        editor->setDecimals(6);
        w = editor;
    }
    else
    {
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMinimum(0);
        editor->setMaximum(10000);
        w = editor;
    }
    return w;
}

void SpinBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (floatingPoint)
    {
        double value = index.model()->data(index, Qt::EditRole).toDouble();
        QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox *>(editor);
        spinBox->setValue(value);
    }
    else
    {
        int value = index.model()->data(index, Qt::EditRole).toInt();
        QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
        spinBox->setValue(value);
    }
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (floatingPoint)
    {
        QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox *>(editor);
        spinBox->interpretText();
        double value = spinBox->value();
        model->setData(index, value, Qt::EditRole);
    }
    else
    {
        QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
        spinBox->interpretText();
        int value = spinBox->value();
        model->setData(index, value, Qt::EditRole);
    }
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect);
}
