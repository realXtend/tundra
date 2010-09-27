// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Avatar_AvatarSceneManager_h
#define incl_Avatar_AvatarSceneManager_h

#include "Foundation.h"
#include "AvatarModule.h"
#include "UiHelper.h"

#include <QObject>
#include <QGraphicsScene>

namespace Avatar
{
    class AnchorLayout;

    class AvatarSceneManager : public QObject
    {

    Q_OBJECT

    public:
        AvatarSceneManager(AvatarModule *avatar_module, QWidget *avatar_editor);
        virtual ~AvatarSceneManager();

    public slots:
        void InitScene();
        void ToggleScene();
        void ShowScene();
        void ExitScene();

    private slots:
        void SceneChanged(const QString &old_name, const QString &new_name);
        void HandleTransferRequest(const QString &widget_name, QGraphicsProxyWidget *widget);

    private:
        Foundation::Framework *framework_;
        AvatarModule *avatar_module_;

        QString scene_name_;
        QGraphicsScene *avatar_scene_;
        AnchorLayout *scene_layout_;

        QWidget *avatar_editor_;

        Helpers::UiHelper *ui_helper_;
    };
}

#endif