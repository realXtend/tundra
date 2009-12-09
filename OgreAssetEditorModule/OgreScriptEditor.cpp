// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreScriptEditor.cpp
 *  @brief  Editing tool for OGRE material and particle scripts.
 *          Provides raw text edit for particles and QProperty editing for materials.
 */

#include "StableHeaders.h"
#include "OgreScriptEditor.h"
#include "OgreAssetEditorModule.h"
#include "Framework.h"
#include "Inventory/InventoryEvents.h"
#include "AssetEvents.h"
#include "OgreMaterialResource.h"
#include "OgreMaterialProperties.h"

#include <QUiLoader>
#include <QFile>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidget>

#include <UiModule.h>
#include <UiProxyWidget.h>
#include <UiWidgetProperties.h>

namespace OgreAssetEditor
{

OgreScriptEditor::OgreScriptEditor(
    Foundation::Framework *framework,
    const RexTypes::asset_type_t &asset_type,
    const QString &name) :
    framework_(framework),
    mainWidget_(0),
    editorWidget_(0),
    lineEditName_(0),
    buttonSaveAs_(0),
    buttonCancel_(0),
    textEdit_(0),
    propertyTable_(0),
    assetType_(asset_type),
    name_(name),
    materialProperties_(0)
{
    InitEditorWindow();

    if (assetType_ == RexTypes::RexAT_ParticleScript)
        CreateTextEdit();

    else if (assetType_ == RexTypes::RexAT_MaterialScript)
    {
        // Create editing widgets for material scripts.
        //QVBoxLayout *layout  = editorWidget_->findChild<QVBoxLayout *>("verticalLayout");
        //layout->addWidget(propertyEditor_);
    }

    lineEditName_->setText(name_);

    buttonSaveAs_->setEnabled(false);
}

// virtual
OgreScriptEditor::~OgreScriptEditor()
{
    SAFE_DELETE(textEdit_);
    SAFE_DELETE(materialProperties_);
    SAFE_DELETE(propertyTable_);
}

void OgreScriptEditor::HandleAssetReady(Foundation::AssetPtr asset)
{
    bool edit_raw = false;

    if (assetType_ == RexTypes::RexAT_ParticleScript)
        edit_raw = true;

    if (assetType_ == RexTypes::RexAT_MaterialScript)
    {
        OgreRenderer::OgreMaterialResource material(asset->GetId(), asset);
        materialProperties_ = new OgreMaterialProperties(name_, &material);

        if (!materialProperties_->HasProperties())
        {
            edit_raw = true;
        }
        else
        {
            OgreMaterialProperties::PropertyMap propMap = materialProperties_->GetPropertyMap();
            OgreMaterialProperties::PropertyMapIter it(propMap);
            size_t mapSize = propMap.size();

            propertyTable_ = new QTableWidget(mapSize, 2);
            propertyTable_->setHorizontalHeaderLabels(QStringList() << tr("Property") << tr("Value"));
            propertyTable_->verticalHeader()->setVisible(false);
            propertyTable_->resize(150, 50);

            int index = 0;
            while(it.hasNext())
            {
                QTableWidgetItem *propertyItem = new QTableWidgetItem(it.peekNext().key());
                QTableWidgetItem *valueItem = new QTableWidgetItem;
                valueItem->setData(Qt::DisplayRole, it.peekNext().value());

                propertyTable_->setItem(index, 0, propertyItem);
                propertyTable_->setItem(index, 1, valueItem);
                ++index;
                it.next();
            }

            propertyTable_->resizeColumnToContents(0);
            propertyTable_->horizontalHeader()->setStretchLastSection(true);

            QVBoxLayout *layout  = mainWidget_->findChild<QVBoxLayout *>("verticalLayout");
            if (layout)
                layout->addWidget(propertyTable_);

//            QGridLayout *layout = new QGridLayout;
//            layout->addWidget(table, 0, 0);
//            setLayout(layout);
        }
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
    ///\todo This destroys only the canvas. Delete the editor instance also.

    // boost::shared_ptr<QtUI::QtModule> qtModule =
    //    framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();

    //qtModule->DeleteCanvas(canvas_->GetID());
}

void OgreScriptEditor::SaveAs()
{
    Foundation::EventManagerPtr event_mgr = framework_->GetEventManager();
    Core::event_category_id_t event_cat = event_mgr->QueryEventCategory("Inventory");
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

    QVector<Core::u8> data_buffer;
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
    script_editor_proxy_widget_->hide();
    // boost::shared_ptr<QtUI::QtModule> qtModule =
    //    framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();

    //qtModule->DeleteCanvas(canvas_->GetID());
}

void OgreScriptEditor::InitEditorWindow()
{
    // Get QtModule and create canvas
    boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
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

    mainWidget_ = loader.load(&file, 0);
    file.close();

    QSize size = mainWidget_->size();
    //canvas_->SetSize(size.width() + 1, size.height() + 1);
    //canvas_->SetSize(size.width() + 1, size.height() + 1);
    //canvas_->SetWindowTitle(QString("OGRE Script Editor"));
    //canvas_->SetStationary(false);
    //canvas_->SetPosition(40, 40);
    //canvas_->AddWidget(mainWidget_);
    //canvas_->Show();

    // Get controls
    lineEditName_ = mainWidget_->findChild<QLineEdit *>("lineEditName");
    buttonSaveAs_ = mainWidget_->findChild<QPushButton *>("buttonSaveAs");
    buttonCancel_ = mainWidget_->findChild<QPushButton *>("buttonCancel");
    editorWidget_= mainWidget_->findChild<QWidget *>("widgetEditor");

    // Connect signals
    QObject::connect(buttonSaveAs_, SIGNAL(clicked()), this, SLOT(SaveAs()));
    QObject::connect(buttonCancel_, SIGNAL(clicked(bool)), this, SLOT(Close()));
    QObject::connect(lineEditName_, SIGNAL(textChanged(const QString &)), this, SLOT(ValidateScriptName(const QString &)));

    // Add widget to UI via ui services module
    script_editor_proxy_widget_ = ui_module->GetSceneManager()->AddWidgetToCurrentScene(mainWidget_, UiServices::UiWidgetProperties(QPointF(10.0, 60.0), size, Qt::Dialog, "Script Editor"));
}

void OgreScriptEditor::CreateTextEdit()
{
    // Raw text edit for particle scripts.
    textEdit_ = new QTextEdit(editorWidget_);
    textEdit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textEdit_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textEdit_->resize(editorWidget_->size());
    textEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    textEdit_->setLineWrapMode(QTextEdit::NoWrap);

//        QVBoxLayout *layout  = mainWidget_->findChild<QVBoxLayout *>("verticalLayout");
//        layout->addWidget(editorWidget_);
    textEdit_->show();
}

} // namespace RexLogic
