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
#include "QtModule.h"
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

#include <OgreMaterial.h>
#include <OgreMaterialSerializer.h>

#include "propertyeditor.h"

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
    propertyEditor_(0),
    textEdit_(0),
    assetType_(asset_type)
{
    InitEditorWindow();

    if (assetType_ == RexTypes::RexAT_ParticleScript)
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

    else if (assetType_ == RexTypes::RexAT_MaterialScript)
    {
        // Create property editor for material scripts.
        propertyEditor_ = new PropertyEditor::PropertyEditor(framework_->GetQApplication());
        propertyEditor_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        propertyEditor_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        propertyEditor_->setParent(editorWidget_);
        propertyEditor_->resize(editorWidget_->size());
        propertyEditor_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//        propertyEditor_->header()->setResizeMode(QHeaderView::ResizeToContents);
        propertyEditor_->show();

//        QVBoxLayout *layout  = editorWidget_->findChild<QVBoxLayout *>("verticalLayout");
//        layout->addWidget(propertyEditor_);


    }

    lineEditName_->setText(name);
}

// virtual
OgreScriptEditor::~OgreScriptEditor()
{
    SAFE_DELETE(textEdit_);
    SAFE_DELETE(propertyEditor_);
    SAFE_DELETE(materialProperties_);
}

void OgreScriptEditor::HandleAssetReady(Foundation::AssetPtr asset)
{
    if (assetType_ == RexTypes::RexAT_ParticleScript)
    {
        QString script((const char *)asset->GetData());
        if (script.isEmpty() && script.isNull())
        {
            OgreAssetEditorModule::LogError("Invalid data for generating particle script.");
            return;
        }

        // Replace tabs (ASCII code decimal 9) with 4 spaces because tabs might appear incorrectly.
        script.trimmed();
        script.replace(QChar(9), "    ");
        textEdit_->setText(script);
    }
    else if (assetType_ == RexTypes::RexAT_MaterialScript)
    {
        OgreRenderer::OgreMaterialResource material(asset->GetId(), asset);
        materialProperties_ = new OgreMaterialProperties(&material);

        QList<QByteArray> properties = materialProperties_->dynamicPropertyNames();
        QListIterator<QByteArray> it(properties);
        while(it.hasNext())
        {
            QString propertyName = it.next();
            QString propertyValue = materialProperties_->property(propertyName.toStdString().c_str()).toString();
            std::cout << propertyName.toStdString() << " " << propertyValue.toStdString() << std::endl;
        }

        if (materialProperties_)
            propertyEditor_->setObject(materialProperties_);
    }
}

void OgreScriptEditor::SaveAs()
{
/*
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
    else if (assetType_ == RexTypes::RexAT_MaterialScript)
    {
        Ogre::MaterialPtr matPtr = materialProperties_->ToOgreMaterial();
        Ogre::MaterialSerializer serializer;
        serializer.queueForExport(matPtr, true, false);
        script = serializer.getQueuedAsString().c_str();
    }

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

    event_data.filenames.push_back(filename);
    event_data.buffers.push_back(data_buffer);

    event_mgr->SendEvent(event_cat, Inventory::Events::EVENT_INVENTORY_UPLOAD_BUFFER, &event_data);
*/
}

void OgreScriptEditor::Cancel()
{
     boost::shared_ptr<QtUI::QtModule> qtModule =
        framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();

    qtModule->DeleteCanvas(canvas_->GetID());
}

void OgreScriptEditor::InitEditorWindow()
{
    // Get QtModule and create canvas
     boost::shared_ptr<QtUI::QtModule> qtModule =
        framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
    if (!qtModule.get())
        return;

    canvas_ = qtModule->CreateCanvas(QtUI::UICanvas::Internal).lock();

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
    canvas_->SetSize(size.width() + 1, size.height() + 1);
    canvas_->SetSize(size.width() + 1, size.height() + 1);
    canvas_->SetWindowTitle(QString("OGRE Script Editor"));
    canvas_->SetStationary(false);
    canvas_->SetPosition(40, 40);
    canvas_->AddWidget(mainWidget_);
    canvas_->Show();

    // Get controls
    lineEditName_ = mainWidget_->findChild<QLineEdit *>("lineEditName");
    buttonSaveAs_ = mainWidget_->findChild<QPushButton *>("buttonSaveAs");
    buttonCancel_ = mainWidget_->findChild<QPushButton *>("buttonCancel");
    editorWidget_= mainWidget_->findChild<QWidget *>("widgetEditor");
//    editorWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Connect button signals
    QObject::connect(buttonSaveAs_, SIGNAL(clicked()), this, SLOT(SaveAs()));
    QObject::connect(buttonCancel_, SIGNAL(clicked(bool)), this, SLOT(Cancel()));

    // Connect line edit signals.
}

} // namespace RexLogic
