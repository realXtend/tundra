// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarEditing/AvatarSceneManager.h"
#include "AvatarEditing/AnchorLayout.h"

#include "UiServiceInterface.h"

namespace Avatar
{
    AvatarSceneManager::AvatarSceneManager(AvatarModule *avatar_module, QWidget *avatar_editor) :
        QObject(0),
        avatar_module_(avatar_module),
        framework_(avatar_module->GetFramework()),
        avatar_editor_(avatar_editor),
        scene_name_("Avatar"),
        avatar_scene_(0),
        ui_helper_(new Helpers::UiHelper(this))
    {
    }

    AvatarSceneManager::~AvatarSceneManager()
    {
    }

    void AvatarSceneManager::InitScene()
    {
        if (avatar_scene_)
            return;

        Foundation::UiServiceInterface *ui_service = avatar_module_->GetFramework()->GetService<Foundation::UiServiceInterface>();
        if (!ui_service) 
            return;

        avatar_scene_ = new QGraphicsScene(this);
        QGraphicsProxyWidget *editor_proxy = avatar_scene_->addWidget(avatar_editor_);

        scene_layout_ = new AnchorLayout(this, avatar_scene_);
        scene_layout_->AddCornerAnchor(editor_proxy, Qt::TopLeftCorner, Qt::TopLeftCorner);
        scene_layout_->AddCornerAnchor(editor_proxy, Qt::BottomLeftCorner, Qt::BottomLeftCorner);

        QGraphicsProxyWidget *toolbar_proxy = ui_helper_->CreateToolbar();
        scene_layout_->AddCornerAnchor(toolbar_proxy, Qt::TopRightCorner, Qt::TopRightCorner);

        ui_service->RegisterScene(scene_name_, avatar_scene_);
        connect(ui_service, SIGNAL(SceneChanged(const QString&, const QString&)), SLOT(SceneChanged(const QString&, const QString&)));
        connect(ui_service, SIGNAL(TransferRequest(const QString&, QGraphicsProxyWidget*)), SLOT(HandleTransferRequest(const QString&, QGraphicsProxyWidget*)));

        connect(ui_helper_, SIGNAL(ExitRequest()), SLOT(ExitScene()));
    }

    void AvatarSceneManager::ToggleScene()
    {
        if (avatar_scene_->isActive())
            ExitScene();
        else
            ShowScene();
    }

    void AvatarSceneManager::ShowScene()
    {
        Foundation::UiServiceInterface *ui_service = avatar_module_->GetFramework()->GetService<Foundation::UiServiceInterface>();
        if (ui_service)
            ui_service->SwitchToScene(scene_name_);
    }

    void AvatarSceneManager::ExitScene()
    {
        Foundation::UiServiceInterface *ui_service = avatar_module_->GetFramework()->GetService<Foundation::UiServiceInterface>();
        if (ui_service)
            ui_service->SwitchToScene("Inworld");
    }

    void AvatarSceneManager::SceneChanged(const QString &old_name, const QString &new_name)
    {
        if (new_name == scene_name_)
        {
            // focus camera to avatar etc.
        }
    }

    void AvatarSceneManager::HandleTransferRequest(const QString &widget_name, QGraphicsProxyWidget *widget)
    {
        if (!avatar_scene_->isActive())
            return;
        if (widget_name != "Console")
            return;
        avatar_scene_->addItem(widget);
    }
}