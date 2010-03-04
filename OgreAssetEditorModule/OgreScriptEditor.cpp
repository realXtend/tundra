// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreScriptEditor.cpp
 *  @brief  Editing tool for OGRE material and particle scripts.
 *          Provides raw text edit for particles and QProperty editing for materials.
 */

#include "StableHeaders.h"
#include "OgreScriptEditor.h"
#include "OgreAssetEditorModule.h"
#include "OgreMaterialResource.h"
#include "OgreMaterialProperties.h"
#include "PropertyTableWidget.h"
#include "ModuleManager.h"
#include "EventManager.h"
#include "Inworld/InworldSceneController.h"

#include "Framework.h"
#include "Inventory/InventoryEvents.h"
#include "AssetEvents.h"

#include <UiModule.h>
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/View/UiWidgetProperties.h"

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

namespace OgreAssetEditor
{

OgreScriptEditor::OgreScriptEditor(
    Foundation::Framework *framework,
    const QString &inventory_id,
    const asset_type_t &asset_type,
    const QString &name,
    QWidget *parent) :
    framework_(framework),
    QWidget(parent),
    proxyWidget_(0),
    mainWidget_(0),
    lineEditName_(0),
    buttonSaveAs_(0),
    buttonCancel_(0),
    textEdit_(0),
    propertyTable_(0),
    inventoryId_(inventory_id),
    assetType_(asset_type),
    name_(name),
    materialProperties_(0)
{
    InitEditorWindow();
    lineEditName_->setText(name_);
    buttonSaveAs_->setEnabled(false);
}

// virtual
OgreScriptEditor::~OgreScriptEditor()
{
    if (proxyWidget_->isVisible())
        proxyWidget_->hide();

    SAFE_DELETE(textEdit_);
    SAFE_DELETE(propertyTable_);
    SAFE_DELETE(materialProperties_);
    SAFE_DELETE(layout_);
    SAFE_DELETE(mainWidget_);
}

void OgreScriptEditor::HandleAssetReady(Foundation::AssetPtr asset)
{
    bool edit_raw = false;

    if (assetType_ == RexTypes::RexAT_ParticleScript)
        edit_raw = true;

    if (assetType_ == RexTypes::RexAT_MaterialScript)
    {
        materialProperties_ = new OgreMaterialProperties(name_, asset);
        if (materialProperties_ && materialProperties_->HasProperties())
            CreatePropertyEditor();
        else
            edit_raw = true;
    }

    if (edit_raw)
    {
        QString script(QByteArray((const char*)asset->GetData(), asset->GetSize()));
        if (script.isEmpty() && script.isNull())
        {
            OgreAssetEditorModule::LogError("Invalid data for generating an OGRE script.");
            return;
        }

        // Replace tabs (ASCII code decimal 9) with 4 spaces because tabs might appear incorrectly.
        script.trimmed();
        script.replace(QChar(9), "    ");

        CreateTextEdit();
        textEdit_->setText(script);
    }
}

void OgreScriptEditor::Close()
{
    proxyWidget_->hide();
    boost::shared_ptr<UiServices::UiModule> ui_module =
        framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
        return;

    ui_module->GetInworldSceneController()->RemoveProxyWidgetFromScene(proxyWidget_);

    emit Closed(inventoryId_, assetType_);
}

void OgreScriptEditor::SaveAs()
{
    Foundation::EventManagerPtr event_mgr = framework_->GetEventManager();
    event_category_id_t event_cat = event_mgr->QueryEventCategory("Inventory");
    if (event_cat == 0)
    {
        OgreAssetEditorModule::LogError("Could not query event category \"Inventory\".");
        return;
    }

    // Get the script.
    QString script;
    if (assetType_ == RexTypes::RexAT_ParticleScript)
    {
        script = textEdit_->toPlainText();
        script.trimmed();
        if (script.isEmpty() || script.isNull())
        {
            OgreAssetEditorModule::LogError("Empty script cannot be saved.");
            return;
        }
    }

    if (assetType_ == RexTypes::RexAT_MaterialScript)
        script = materialProperties_->ToString();

    // Get the name.
    QString filename = lineEditName_->text();
    if (filename.isEmpty() || filename.isNull())
    {
        OgreAssetEditorModule::LogError("Empty name for the script, cannot upload.");
        return;
    }

    // Create event data.
    Inventory::InventoryUploadBufferEventData event_data;

    QVector<u8> data_buffer;
    data_buffer.resize(script.size());
    memcpy(&data_buffer[0], script.toStdString().c_str(), script.size());

    // Add file extension.
    filename.append(RexTypes::GetFileExtensionFromAssetType(assetType_).c_str());
    event_data.filenames.push_back(filename);
    event_data.buffers.push_back(data_buffer);

    event_mgr->SendEvent(event_cat, Inventory::Events::EVENT_INVENTORY_UPLOAD_BUFFER, &event_data);
}

void OgreScriptEditor::ValidateScriptName(const QString &name)
{
    if (name == name_ || name.isEmpty() || name.isNull())
        buttonSaveAs_->setEnabled(false);
    else
        buttonSaveAs_->setEnabled(true);
}

void OgreScriptEditor::PropertyChanged(int row, int column)
{
    QTableWidgetItem *nameItem = propertyTable_->item(row, column - 2);
    QTableWidgetItem *typeItem = propertyTable_->item(row, column - 1);
    QTableWidgetItem *valueItem = propertyTable_->item(row, column);
    if (!nameItem || !typeItem || !valueItem)
        return;

    QString newValueString(valueItem->text());
    newValueString.trimmed();
    bool valid = true;

    ///\todo Validity check for texture names.
    QString type = typeItem->text();
    if (type != "TEX_1D" && type != "TEX_2D" && type != "TEX_3D" && type != "TEX_CUBEMAP")
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
        materialProperties_->setProperty(nameItem->text().toLatin1(), QVariant(typeValuePair));
        ValidateScriptName(lineEditName_->text());
    }
    else
    {
        valueItem->setBackgroundColor(QColor(255, 73, 73));
        buttonSaveAs_->setEnabled(false);
    }

    propertyTable_->setCurrentItem(valueItem, QItemSelectionModel::Deselect);
}

void OgreScriptEditor::InitEditorWindow()
{
    // Get QtModule and create canvas
    boost::shared_ptr<UiServices::UiModule> ui_module = 
        framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
        return;

    // Create widget from ui file
    QUiLoader loader;
    QFile file("./data/ui/ogrescripteditor.ui");
    if (!file.exists())
    {
        OgreAssetEditorModule::LogError("Cannot find OGRE Script Editor .ui file.");
        return;
    }

    mainWidget_ = loader.load(&file);
    file.close();

    layout_ = new QVBoxLayout;
    layout_->addWidget(mainWidget_);
    layout_->setContentsMargins(0, 0, 0, 0);
    setLayout(layout_);

    // Get controls
    lineEditName_ = mainWidget_->findChild<QLineEdit *>("lineEditName");
    buttonSaveAs_ = mainWidget_->findChild<QPushButton *>("buttonSaveAs");
    buttonCancel_ = mainWidget_->findChild<QPushButton *>("buttonCancel");

    // Connect signals
    QObject::connect(buttonSaveAs_, SIGNAL(clicked()), this, SLOT(SaveAs()));
    QObject::connect(buttonCancel_, SIGNAL(clicked(bool)), this, SLOT(Close()));
    QObject::connect(lineEditName_, SIGNAL(textChanged(const QString &)), this, SLOT(ValidateScriptName(const QString &)));

    // Add widget to UI via ui services module
    proxyWidget_ = ui_module->GetInworldSceneController()->AddWidgetToScene(
        this, UiServices::UiWidgetProperties("OGRE Script Editor: " + name_, UiServices::SceneWidget));

    QObject::connect(proxyWidget_, SIGNAL(Closed()), this, SLOT(Close()));

    proxyWidget_->show();
    ui_module->GetInworldSceneController()->BringProxyToFront(proxyWidget_);
}

void OgreScriptEditor::CreateTextEdit()
{
    // Raw text edit for particle scripts or material scripts without properties.
    textEdit_ = new QTextEdit();
    textEdit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textEdit_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    textEdit_->setLineWrapMode(QTextEdit::NoWrap);

    QVBoxLayout *layout  = mainWidget_->findChild<QVBoxLayout *>("verticalLayoutEditor");
    layout->addWidget(textEdit_);
    textEdit_->show();
}

void OgreScriptEditor::CreatePropertyEditor()
{
    PropertyMap propMap = materialProperties_->GetPropertyMap();
    PropertyMapIter it(propMap);

    propertyTable_ = new PropertyTableWidget(propMap.size(), 3, 0);
    QVBoxLayout *layout = mainWidget_->findChild<QVBoxLayout *>("verticalLayoutEditor");
    layout->addWidget(propertyTable_);

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

        propertyTable_->setItem(row, 0, nameItem);
        propertyTable_->setItem(row, 1, typeItem);
        propertyTable_->setItem(row, 2, valueItem);
        ++row;
    }

    propertyTable_->show();

    QObject::connect(propertyTable_, SIGNAL(cellChanged(int, int)), this, SLOT(PropertyChanged(int, int)));
}

}
