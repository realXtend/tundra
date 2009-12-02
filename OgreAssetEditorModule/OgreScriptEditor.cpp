// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreScriptEditor.cpp
 *  @brief  Editing tool for OGRE material and particle scripts.
 */

#include "StableHeaders.h"
#include "OgreScriptEditor.h"
#include "OgreAssetEditorModule.h"
#include "Framework.h"
#include "QtModule.h"
#include "Inventory/InventoryEvents.h"
#include "AssetEvents.h"

#include <QUiLoader>
#include <QFile>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>

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
    textEdit_(0)
{
    InitEditorWindow();

//    if (asset_type == RexTypes::RexAT_ParticleScript)
//    {
        // Raw text edit for particle scripts.
        textEdit_ = new QTextEdit(editorWidget_);
        textEdit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        textEdit_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        textEdit_->resize(editorWidget_->size());
        //textEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        textEdit_->setLineWrapMode(QTextEdit::NoWrap);
        textEdit_->show();
//    }

/*
    if (asset_type == RexTypes::RexAT_MaterialScript)
    {
        // Create property editor for material scripts.
        propertyEditor_ = new PropertyEditor::PropertyEditor(framework_->GetQApplication());
        propertyEditor_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        propertyEditor_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        propertyEditor_->setParent(editorWidget_);
        propertyEditor_->resize(editorWidget_->size());
        propertyEditor_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // doesn't seem to do the trick...
    }
*/

    lineEditName_->setText(name);
}

// virtual
OgreScriptEditor::~OgreScriptEditor()
{
    SAFE_DELETE(propertyEditor_);
}

void OgreScriptEditor::HandleAssetReady(Foundation::AssetPtr asset)
{
    QString script((const char *)asset->GetData());
    if (!script.isEmpty() && !script.isNull())
    {
        // Replaces tabs (ascii code decimal 9) with 4 spaces.
        script.replace(QChar(9), "    ");
        textEdit_->setText(script);
    }
}

void OgreScriptEditor::SaveAs()
{
/*
    propertyEditor_->setObject(editorWidget_);
    propertyEditor_->show();
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

    // Add to control bar
//    qtModule->AddCanvasToControlBar(canvas_, QString("Ogre Script Editor"));

    // Get controls
    lineEditName_ = mainWidget_->findChild<QLineEdit *>("lineEditName");
    buttonSaveAs_ = mainWidget_->findChild<QPushButton *>("buttonSaveAs");
    buttonCancel_ = mainWidget_->findChild<QPushButton *>("buttonCancel");
    editorWidget_= mainWidget_->findChild<QWidget *>("widgetEditor");
    editorWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Connect signals
    QObject::connect(buttonSaveAs_, SIGNAL(clicked()), this, SLOT(SaveAs()));
    QObject::connect(buttonCancel_, SIGNAL(clicked(bool)), this, SLOT(Cancel()));
    // Connect lineEditName_ signals
}

} // namespace RexLogic
