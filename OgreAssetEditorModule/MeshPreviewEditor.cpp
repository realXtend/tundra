#include "StableHeaders.h"
#include "Framework.h"
#include "DebugOperatorNew.h"
#include "MeshPreviewEditor.h"
#include "OgreAssetEditorModule.h"

#include <UiModule.h>
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/View/UiWidgetProperties.h"
#include "Inworld/InworldSceneController.h"
#include "ModuleManager.h"
#include "AssetInterface.h"

#include <QUiLoader>
#include <QFile>
#include <QLayout>
#include <QPushButton>

#include "MemoryLeakCheck.h"

namespace Naali
{
    MeshPreviewLabel::MeshPreviewLabel(QWidget *parent, Qt::WindowFlags flags):
        QLabel(parent, flags)
    {

    }

    MeshPreviewLabel::~MeshPreviewLabel()
    {

    }

    MeshPreviewEditor::MeshPreviewEditor(Foundation::Framework *framework,
                                           const QString &inventory_id,
                                           const asset_type_t &asset_type,
                                           const QString &name,
                                           QWidget *parent):
        QWidget(parent),
        framework_(framework),
        assetType_(asset_type),
        inventoryId_(inventory_id),
        okButton_(0)
    {
        setObjectName(name);
        InitializeEditorWidget();
    }

    MeshPreviewEditor::~MeshPreviewEditor()
    {

    }

    void MeshPreviewEditor::HandleAssetReady(Foundation::AssetPtr asset)
    {
        //TODO! Add asset ready code in here.
    }

    void MeshPreviewEditor::Closed()
    {
        boost::shared_ptr<UiServices::UiModule> ui_module =
            framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (!ui_module.get())
            return;

        ui_module->GetInworldSceneController()->RemoveProxyWidgetFromScene(this);

        emit Closed(inventoryId_, assetType_);
    }

    void MeshPreviewEditor::InitializeEditorWidget()
    {
        // Get QtModule and create canvas
        boost::shared_ptr<UiServices::UiModule> ui_module = 
            framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (!ui_module.get())
            return;

        // Create widget from ui file
        QUiLoader loader;
        QFile file("./data/ui/mesh_preview.ui");
        if (!file.exists())
        {
            OgreAssetEditorModule::LogError("Cannot find OGRE Script Editor .ui file.");
            return;
        }
        mainWidget_ = loader.load(&file);
        file.close();

        QVBoxLayout *layout = new QVBoxLayout;
        setLayout(layout);
        layout->addWidget(mainWidget_);
        layout->setContentsMargins(0, 0, 0, 0);

        okButton_ = mainWidget_->findChild<QPushButton *>("okButton");
        QObject::connect(okButton_, SIGNAL(clicked()), this, SLOT(Closed()));

        // Add widget to UI via ui services module
        UiServices::UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(this, UiServices::UiWidgetProperties(tr("Mesh: ") + objectName(), UiServices::SceneWidget));
        QObject::connect(proxy, SIGNAL(Closed()), this, SLOT(Closed()));
        proxy->show();
        ui_module->GetInworldSceneController()->BringProxyToFront(proxy);
    }
}