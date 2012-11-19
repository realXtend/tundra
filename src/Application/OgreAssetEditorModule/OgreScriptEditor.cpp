/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   OgreScriptEditor.cpp
    @brief  Text editing tool for OGRE material and particle scripts. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "OgreScriptEditor.h"
#include "OgreAssetEditorModule.h"
#include "PropertyTableWidget.h"
#include "OgreScriptHighlighter.h"

#include "LoggingFunctions.h"
#include "Application.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "IAssetTransfer.h"

#include <QUiLoader>
#include <QFile>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QColor>
#include <QVBoxLayout>

#include "MemoryLeakCheck.h"

OgreScriptEditor::OgreScriptEditor(const AssetPtr &scriptAsset, Framework *fw, QWidget *parent) :
    QWidget(parent),
    framework(fw),
    asset(scriptAsset),
    lineEditName(0),
    buttonSaveAs(0),
    textEdit(0)
//    propertyTable(0),
//    materialProperties(0)
{
    QUiLoader loader;
    QFile file(Application::InstallationDirectory() + "data/ui/ogrescripteditor.ui");
    if (!file.exists())
    {
        LogError("Cannot find OGRE Script Editor .ui file.");
        return;
    }

    QWidget *mainWidget = loader.load(&file, this);
    file.close();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mainWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    lineEditName = mainWidget->findChild<QLineEdit *>("lineEditName");
    buttonSaveAs = mainWidget->findChild<QPushButton *>("buttonSaveAs");
    QPushButton *buttonSave = mainWidget->findChild<QPushButton *>("buttonSave");
    QPushButton *buttonCancel = mainWidget->findChild<QPushButton *>("buttonCancel");

    ///\todo Save as -functionality disabled for now.
    lineEditName->setDisabled(true);
    buttonSaveAs->setDisabled(true);

    connect(buttonSave, SIGNAL(clicked()), SLOT(Save()));
    connect(buttonSaveAs, SIGNAL(clicked()), SLOT(SaveAs()));
    connect(buttonCancel, SIGNAL(clicked(bool)), SLOT(close()));
    connect(lineEditName, SIGNAL(textChanged(const QString &)), SLOT(ValidateScriptName(const QString &)));

    buttonSaveAs->setEnabled(false);

    SetScriptAsset(scriptAsset);
}

OgreScriptEditor::~OgreScriptEditor()
{
//    SAFE_DELETE(propertyTable);
//    SAFE_DELETE(materialProperties);
}

void OgreScriptEditor::SetScriptAsset(const AssetPtr &scriptAsset)
{
    asset = scriptAsset;
    assert(asset.lock());
    AssetPtr assetPtr = asset.lock();
    if (!assetPtr)
        LogError("OgreScriptEditor: null asset given.");
    if (assetPtr->Type() != "OgreMaterial" && assetPtr->Type() != "OgreParticle")
        LogWarning("Created OgreScriptEditor for non-supported asset type " + assetPtr->Type() + ".");

    lineEditName->setText((assetPtr?assetPtr->Name():QString()));
    setWindowTitle(tr("OGRE Script Editor:") + (assetPtr?assetPtr->Name():QString()));

    if (assetPtr && !assetPtr->IsLoaded())
    {
        AssetTransferPtr transfer = framework->Asset()->RequestAsset(assetPtr->Name(), assetPtr->Type(), true);
        connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), this, SLOT(OnAssetTransferSucceeded(AssetPtr)));
        connect(transfer.get(), SIGNAL(Failed(IAssetTransfer *, QString)), SLOT(OnAssetTransferFailed(IAssetTransfer *, QString)));
    }
    else
        Open();
}

void OgreScriptEditor::Open()
{
    AssetPtr assetPtr = asset.lock();
    if (assetPtr && !assetPtr->IsLoaded())
    {
        LogInfo("OgreScriptEditor::Open: asset not loaded.");
        return;
    }
    ///\todo
/*
    bool edit_raw = false;

    if (asset->Type() == "OgreParticle")
        edit_raw = true;

    if (asset->Type() == "OgreMaterial")
    {
        materialProperties = new OgreMaterialProperties(asset->Name(), asset);
        if (materialProperties && materialProperties->HasProperties())
            CreatePropertyEditor();
        else
            edit_raw = true;
    }

    if (edit_raw)
    {
        QString script(asset->RawData());
        if (script.isEmpty() && script.isNull())
        {
            LogError("Invalid data for generating an OGRE script.");
            return;
        }

        // Replace tabs (ASCII code decimal 9) with 4 spaces because tabs might appear incorrectly.
        script = script.trimmed();
        script.replace(QChar(9), "    ");

        CreateTextEdit();
        textEdit->setText(script);
    }
*/
    if (assetPtr->Type() == "OgreMaterial" || assetPtr->Type() == "OgreParticle")
    {
        std::vector<u8> data;
        if (assetPtr->SerializeTo(data))
        {
            data.push_back('\0');
            QString script((const char *)&data[0]);
            script = script.trimmed();
            script.replace(QChar(9), "    ");

            CreateTextEdit();
            textEdit->setText(script);
            OgreScriptHighlighter *hl= new OgreScriptHighlighter(assetPtr->Type(), textEdit);
            hl->setDocument(textEdit->document());
        }
    }
}

void OgreScriptEditor::Save()
{
    AssetPtr assetPtr = asset.lock();
    if (assetPtr && (assetPtr->Type() == "OgreMaterial" || assetPtr->Type() == "OgreParticle"))
    {
        QByteArray bytes = textEdit->toPlainText().toAscii().data();
        const char *data = bytes.data();
        assetPtr->LoadFromFileInMemory((u8 *)data, (size_t)bytes.size());
    }
}

void OgreScriptEditor::SaveAs()
{
///\todo Enable save as feature
/*
    // Get the script.
    QString script;
    if (asset->Type() == "OgreParticle")
    {
        script = textEdit->toPlainText();
        script = script.trimmed();
        if (script.isEmpty() || script.isNull())
        {
            LogError("Empty script cannot be saved.");
            return;
        }
    }

    if (asset->Type() == "OgreMaterial" && materialProperties)
        script = materialProperties->ToString();

    // Get the name.
    QString filename = lineEditName->text();
    if (filename.isEmpty() || filename.isNull())
    {
        LogError("Empty name for the script, cannot save.");
        return;
    }
*/
}

void OgreScriptEditor::ValidateScriptName(const QString &name)
{
    bool valid = !(name.isEmpty() || name.isNull() || framework->Asset()->GetAsset(name));
    buttonSaveAs->setEnabled(valid);
}

/*
void OgreScriptEditor::PropertyChanged(int row, int column)
{
    QTableWidgetItem *nameItem = propertyTable->item(row, column - 2);
    QTableWidgetItem *typeItem = propertyTable->item(row, column - 1);
    QTableWidgetItem *valueItem = propertyTable->item(row, column);
    if (!nameItem || !typeItem || !valueItem)
        return;

    QString newValueString(valueItem->text());
    newValueString = newValueString.trimmed();
    bool valid = true;

    QString type = typeItem->text();
    if (type == "TEX_1D" || type == "TEX_2D" || type == "TEX_3D" && type == "TEX_CUBEMAP")
    {
        // If the texture name is not valid UUID or URL it can't be used.
        if (IsValidUuid(newValueString))
            valid = true;
        else if(newValueString.indexOf("http://") != -1)
            valid = true;
        else
            valid = false;
    }
    else
    {
        int i = 0, j = 0;
        while(j != -1 && valid)
        {
            j = newValueString.indexOf(' ', i);
            QString newValue = newValueString.mid(i, j == -1 ? j : j - i);
            if (!newValue.isEmpty())
                newValue.toFloat(&valid);
            i = j + 1;
        }
    }

    if (valid)
    {
        valueItem->setBackgroundColor(QColor(QColor(81, 255, 81)));
        QMap<QString, QVariant> typeValuePair;
        typeValuePair[typeItem->text()] = newValueString;
        materialProperties->setProperty(nameItem->text().toLatin1(), QVariant(typeValuePair));
        ValidateScriptName(lineEditName->text());
    }
    else
    {
        valueItem->setBackgroundColor(QColor(255, 73, 73));
        buttonSaveAs->setEnabled(false);
    }

    propertyTable->setCurrentItem(valueItem, QItemSelectionModel::Deselect);
}
*/

void OgreScriptEditor::CreateTextEdit()
{
    // Raw text edit for particle scripts or material scripts without properties.
    textEdit = new QTextEdit(this);
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    textEdit->setLineWrapMode(QTextEdit::NoWrap);

    QVBoxLayout *layout  = findChild<QWidget *>("OgreScriptEditor")->findChild<QVBoxLayout *>("verticalLayoutEditor");
    layout->addWidget(textEdit);
    textEdit->show();
}

/*
void OgreScriptEditor::CreatePropertyEditor()
{
    ShaderParameterMap propMap = materialProperties->GetPropertyMap();
    ShaderParameterMapIter it(propMap);

    propertyTable = new PropertyTableWidget(propMap.size(), 3);
    QVBoxLayout *layout = findChild<QWidget *>("OgreScriptEditor")->findChild<QVBoxLayout *>("verticalLayoutEditor");
    layout->addWidget(propertyTable);

    int row = 0;
    while(it.hasNext())
    {
        it.next();
        QMap<QString, QVariant> typeValuePair = it.value().toMap();

        // Property name, set non-editable.
        QTableWidgetItem *nameItem = new QTableWidgetItem(it.key());
        nameItem->setFlags(Qt::ItemIsEnabled);

        // Property type, set non-editable.
        QTableWidgetItem *typeItem = new QTableWidgetItem(typeValuePair.begin().key());
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

        valueItem->setData(Qt::DisplayRole, typeValuePair.begin().value());
        valueItem->setBackgroundColor(QColor(81, 255, 81));

        propertyTable->setItem(row, 0, nameItem);
        propertyTable->setItem(row, 1, typeItem);
        propertyTable->setItem(row, 2, valueItem);
        ++row;
    }

    propertyTable->show();

    connect(propertyTable, SIGNAL(cellChanged(int, int)), this, SLOT(PropertyChanged(int, int)));
}
*/

void OgreScriptEditor::OnAssetTransferSucceeded(AssetPtr scriptAsset)
{
    SetScriptAsset(scriptAsset);
}

void OgreScriptEditor::OnAssetTransferFailed(IAssetTransfer *transfer, QString reason)
{
    LogError("OgreScriptEditor::OnAssetTransferFailed: " + reason);
    //setText("Could not load asset: " + reason);
}
