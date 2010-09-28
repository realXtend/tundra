// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarEditing/AvatarSceneManager.h"
#include "AvatarEditing/AnchorLayout.h"

#include "Avatar/AvatarHandler.h"
#include "Avatar/AvatarAppearance.h"

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

        // Scene
        avatar_scene_ = new QGraphicsScene(this);
        QGraphicsProxyWidget *editor_proxy = avatar_scene_->addWidget(avatar_editor_);

        // Layout
        scene_layout_ = new AnchorLayout(this, avatar_scene_);
        scene_layout_->AddCornerAnchor(editor_proxy, Qt::TopLeftCorner, Qt::TopLeftCorner);
        scene_layout_->AddCornerAnchor(editor_proxy, Qt::BottomLeftCorner, Qt::BottomLeftCorner);

        // Toolbar
        QGraphicsProxyWidget *toolbar_proxy = ui_helper_->CreateToolbar();
        scene_layout_->AddCornerAnchor(toolbar_proxy, Qt::TopRightCorner, Qt::TopRightCorner);

        // Info
        QGraphicsProxyWidget *info_proxy = ui_helper_->CreateInfoWidget();
        scene_layout_->AddCornerAnchor(editor_proxy, Qt::TopRightCorner, info_proxy, Qt::TopLeftCorner);
        scene_layout_->AddCornerAnchor(info_proxy, Qt::TopRightCorner, toolbar_proxy, Qt::TopLeftCorner);
        info_proxy->hide();

        // Hook info signals to ui
        connect(avatar_editor_, SIGNAL(EditorStatus(const QString&, int)), ui_helper_, SLOT(ShowStatus(const QString&, int)));
        connect(avatar_editor_, SIGNAL(EditorError(const QString&, int)), ui_helper_, SLOT(ShowError(const QString&, int)));
        connect(avatar_editor_, SIGNAL(EditorHideMessages()), ui_helper_, SLOT(HideInfo()));
        AvatarAppearance *appearance_handler = &avatar_module_->GetAvatarHandler()->GetAppearanceHandler();
        if (appearance_handler)
        {
            connect(appearance_handler, SIGNAL(AppearanceStatus(const QString&, int)), ui_helper_, SLOT(ShowStatus(const QString&, int)));
            connect(appearance_handler, SIGNAL(AppearanceError(const QString&, int)), ui_helper_, SLOT(ShowError(const QString&, int)));
        }

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