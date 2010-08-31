// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ActionProxyWidget_h
#define incl_UiModule_ActionProxyWidget_h

#include "UiModuleFwd.h"

#include <QGraphicsProxyWidget>
#include <QPropertyAnimation>
#include <QWidget>
#include <QVBoxLayout>

#include "ui_EtherActionWidget.h"

namespace ProtocolUtilities
{
    class GridInfoHelper;
}

namespace Ether
{
    namespace View
    {
        class ActionProxyWidget : public QGraphicsProxyWidget
        {

        Q_OBJECT

        public:
            ActionProxyWidget(Data::DataManager *data_manager);

        public slots:
            void ShowWidget(QString type, View::InfoCard *card);
            void UpdateGeometry(const QRectF &rect);

        private slots:
            void ClearLayout();

            QWidget *CreateNewOpenSimAvatar();
            QWidget *OpenSimAvatarInfoWidget(Data::OpenSimAvatar *data);
            QWidget *OpenSimAvatarEditWidget(Data::OpenSimAvatar *data);

            QWidget *CreateNewOpenSimWorld();
            QWidget *OpenSimWorldInfoWidget(Data::OpenSimWorld *data);
            QWidget *OpenSimWorldEditWidget(Data::OpenSimWorld *data);

            QWidget *CreateNewRealXtendAvatar();
            QWidget *RealXtendAvatarInfoWidget(Data::RealXtendAvatar *data);
            QWidget *RealXtendAvatarEditWidget(Data::RealXtendAvatar *data);

            QPixmap CretatePicture(QSize image_size, QString pixmap_path);

            void StartShowAnimations(QWidget *shown_widget);
            void StartHideAnimations();
            void AnimationsComplete();

            void GridInfoRequested();
            void GridInfoRevieced(QMap<QString, QVariant>);
            void SetGridInfoToWorldWidget(QWidget *world_widget, QMap<QString, QVariant> grid_info_map);
        
            QWidget *AvatarSelectionWidget();
            void AvatarSelectedOpenSim();
            void AvatarSelectedRealXtend();

            void SaveInformation();

        private:
            Data::DataManager *data_manager_;
            Ui::EtherActionWidget ether_action_widget_ui_;

            QWidget *current_widget_;
            QLayout *layout_;

            QPropertyAnimation *visibility_animation_;

            // Current data and type
            Data::OpenSimWorld *current_os_world_data_;
            Data::OpenSimAvatar *current_os_avatar_data_;
            Data::RealXtendAvatar *current_rex_avatar_data_;
            QString current_type_;
            QMap<QString,QVariant> current_grid_info_map_;

            ProtocolUtilities::GridInfoHelper *grid_info_helper_;

        signals:
            void ActionInProgress(bool);
            void RemoveAvatar(Data::AvatarInfo *);
            void RemoveWorld(Data::WorldInfo *);

        };
    }
}

#endif // incl_UiModule_ActionProxyWidget_h