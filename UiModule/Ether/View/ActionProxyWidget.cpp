// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ActionProxyWidget.h"
#include "OpenSim/GridInfoHelper.h"
#include "CoreDefines.h"
#include "Ether/View/InfoCard.h"
#include "Ether/Data/DataManager.h"
#include "Ether/Data/AvatarInfo.h"
#include "Ether/Data/OpenSimAvatar.h"
#include "Ether/Data/RealXtendAvatar.h"
#include "Ether/Data/WorldInfo.h"
#include "Ether/Data/OpenSimWorld.h"

#include <QLayoutItem>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QGraphicsScene>

#include <QUiLoader>
#include <QFile>

#include <QDebug>

#include "MemoryLeakCheck.h"

namespace Ether
{
    namespace View
    {
        ActionProxyWidget::ActionProxyWidget(Data::DataManager *data_manager)
            : QGraphicsProxyWidget(0, Qt::Widget),
              data_manager_(data_manager),
              current_widget_(0),
              current_type_(""),
              current_os_world_data_(0),
              current_os_avatar_data_(0),
              current_rex_avatar_data_(0)

        {
            QWidget *container_widget = new QWidget(0);
            ether_action_widget_ui_.setupUi(container_widget);
            setWidget(container_widget);

            connect(ether_action_widget_ui_.pushButtonBack, SIGNAL( clicked() ), SLOT( StartHideAnimations() ));

            setZValue(100);
            setOpacity(0);
            hide();

            visibility_animation_ = new QPropertyAnimation(this, "opacity", this);
            visibility_animation_->setDuration(300);
            visibility_animation_->setEasingCurve(QEasingCurve::Linear);

            connect(visibility_animation_, SIGNAL( finished() ), SLOT( AnimationsComplete() ));
        }

        void ActionProxyWidget::ShowWidget(QString type, View::InfoCard *card)
        {
            ClearLayout();
            
            QWidget *action_widget = 0;
            QString title;

            // Controlling already made cards
            switch (card->dataType())
            {
                case InfoCard::Avatar:
                {
                    // Retrieve data
                    Data::AvatarInfo *avatar_info = data_manager_->GetAvatarInfo(card->id());
                    if (!avatar_info)
                        return;
                    
                    // Adding a new card
                    if (type == "add")    
                    {
                        action_widget = AvatarSelectionWidget();
                        title = "Creating new avatar";
                    }

                    // Removing the card
                    else if (type == "remove")
                    {
                        emit RemoveAvatar(avatar_info);
                        return;
                    }

                    // Modifying the card
                    else
                    {
                        // Generate QWidget with avatar and action widget type
                        if (avatar_info->avatarType() == AvatarTypes::OpenSim)
                        {
                            
                            Data::OpenSimAvatar *os_avatar = dynamic_cast<Data::OpenSimAvatar *>(avatar_info);
                            if (os_avatar && type == "info")
                            {
                                action_widget = OpenSimAvatarInfoWidget(os_avatar);
                                title = QString("Information of OpenSim avatar %1").arg(os_avatar->userName());
                            }
                            else if (os_avatar && type == "edit")
                            {
                                action_widget = OpenSimAvatarEditWidget(os_avatar);
                                title = QString("Editing OpenSim avatar %1").arg(os_avatar->userName());
                            }
                        }
                        else if (avatar_info->avatarType() == AvatarTypes::RealXtend)
                        {
                            Data::RealXtendAvatar *rex_avatar = dynamic_cast<Data::RealXtendAvatar *>(avatar_info);
                            if (rex_avatar && type == "info")
                            {
                                action_widget = RealXtendAvatarInfoWidget(rex_avatar);
                                title = QString("Information of RealXtend avatar %1").arg(rex_avatar->account());
                            }
                            else if (rex_avatar && type == "edit")
                            {
                                action_widget = RealXtendAvatarEditWidget(rex_avatar);
                                title = QString("Editing RealXtend avatar %1").arg(rex_avatar->account());
                            }
                        }
                    }
                    break;
                }

                case InfoCard::World:
                {
                    // Retrieve data
                    Data::WorldInfo *world_info = data_manager_->GetWorldInfo(card->id());
                    if (!world_info)
                        return;

                    // Adding a new card
                    if (type == "add")
                    {
                        action_widget = CreateNewOpenSimWorld();
                        title = "Creating new OpenSim world";
                    }

                    // Removing the card
                    else if (type == "remove")
                    {
                        emit RemoveWorld(world_info);
                        return;
                    }

                    // Modifying the card
                    else
                    {
                        // Generate QWidget with world and action widget type
                        if (world_info->worldType() == WorldTypes::OpenSim)
                        {
                            Data::OpenSimWorld *os_world = dynamic_cast<Data::OpenSimWorld *>(world_info);
                            if (os_world && type == "info")
                            {
                                action_widget = OpenSimWorldInfoWidget(os_world);
                                title = QString("Information of OpenSim world %1").arg(os_world->loginUrl().host());
                            }
                            else if (os_world && type == "edit")
                            {
                                action_widget = OpenSimWorldEditWidget(os_world);
                                title = QString("Editing OpenSim world %1").arg(os_world->loginUrl().host());
                            }
                        }
                    }
                    break;
                }

                default:
                {
                    action_widget = 0;
                }
            }

            if (action_widget)
            {
                current_widget_ = action_widget;
                ether_action_widget_ui_.titleLabel->setText(title);
                ether_action_widget_ui_.widgetLayout->addWidget(action_widget);
                StartShowAnimations(action_widget);
            }
        }

        // Private

        void ActionProxyWidget::ClearLayout()
        {
            if (!current_widget_)
                return;

            QLayoutItem *item = ether_action_widget_ui_.widgetLayout->itemAt(ether_action_widget_ui_.widgetLayout->indexOf(current_widget_));
            if (item)
            {
                ether_action_widget_ui_.widgetLayout->removeItem(item);
                current_widget_->setParent(0);
                SAFE_DELETE(current_widget_);
            }
    
            // In case there are many items in the layout
            // will happen when last action has been adding an avatar, 
            // that requires two widgets on the same show
            int count = ether_action_widget_ui_.widgetLayout->count();
            if (count > 0)
            {
                for (int i=0; i<count; i++)
                {
                    QLayoutItem *item = ether_action_widget_ui_.widgetLayout->itemAt(i);
                    ether_action_widget_ui_.widgetLayout->removeItem(item);
                    item->widget()->setParent(0);
                    SAFE_DELETE(item);
                }
            }
        }

        void ActionProxyWidget::StartShowAnimations(QWidget *shown_widget)
        {
            UpdateGeometry(scene()->sceneRect());
            show();

            visibility_animation_->setDirection(QAbstractAnimation::Forward);
            visibility_animation_->setStartValue(0);
            visibility_animation_->setEndValue(1);
            visibility_animation_->start();

            emit ActionInProgress(true);
        }

        void ActionProxyWidget::StartHideAnimations()
        {
            visibility_animation_->setDirection(QAbstractAnimation::Backward);
            visibility_animation_->start();
            emit ActionInProgress(false);
        }

        void ActionProxyWidget::AnimationsComplete()
        {
            if (visibility_animation_->state() == QAbstractAnimation::Backward)
                hide();
        }

        void ActionProxyWidget::UpdateGeometry(const QRectF &rect)
        {
            setGeometry(rect);
        }

        QWidget *ActionProxyWidget::CreateNewOpenSimAvatar()
        {
            QWidget *new_os_avatar;

            QUiLoader loader;
            QFile uiFile("./data/ui/ether/avatar-edit-opensim.ui");
            new_os_avatar = loader.load(&uiFile, 0);
            uiFile.close();

            QPushButton *button = new_os_avatar->findChild<QPushButton*>("pushButtonSave");
            connect(button, SIGNAL( clicked() ), SLOT( SaveInformation() ));

            QPixmap pic = CretatePicture(QSize(150,150), "./data/ui/images/ether/naali.png");
            QLabel *pic_label = new_os_avatar->findChild<QLabel*>("pictureLabel");
            pic_label->setPixmap(pic);

            // Store data pointer and define types for save function
            current_type_ = "new-avatar-opensim";
            current_os_avatar_data_ = 0;

            return new_os_avatar;
        }

        QWidget *ActionProxyWidget::OpenSimAvatarInfoWidget(Data::OpenSimAvatar *data)
        {
            return OpenSimAvatarEditWidget(data);
        }

        QWidget *ActionProxyWidget::OpenSimAvatarEditWidget(Data::OpenSimAvatar *data)
        {
            QWidget *os_edit_widget;

            QUiLoader loader;
            QFile uiFile("./data/ui/ether/avatar-edit-opensim.ui");
            os_edit_widget = loader.load(&uiFile, 0);
            uiFile.close();

            QLineEdit *line_edit;
            line_edit = os_edit_widget->findChild<QLineEdit*>("firstNameLineEdit");
            line_edit->setText(data->firstName());
            line_edit = os_edit_widget->findChild<QLineEdit*>("lastNameLineEdit");
            line_edit->setText(data->lastName());
            line_edit = os_edit_widget->findChild<QLineEdit*>("passwordLineEdit");
            line_edit->setText(data->password());

            QPixmap pic = CretatePicture(QSize(150,150), data->pixmapPath());
            QLabel *pic_label = os_edit_widget->findChild<QLabel*>("pictureLabel");
            pic_label->setPixmap(pic);

            QPushButton *button = os_edit_widget->findChild<QPushButton*>("pushButtonSave");
            connect(button, SIGNAL( clicked() ), SLOT( SaveInformation() ));

            // Store data pointer and define types for save function
            current_type_ = "avatar-opensim";
            current_os_avatar_data_ = data;

            return os_edit_widget;
        }

        QWidget *ActionProxyWidget::OpenSimWorldInfoWidget(Data::OpenSimWorld *data)
        {
            return OpenSimWorldEditWidget(data);
        }

        QWidget *ActionProxyWidget::OpenSimWorldEditWidget(Data::OpenSimWorld *data)
        {
            QWidget *os_world_edit_widget;
            current_grid_info_map_.clear();

            QUiLoader loader;
            QFile uiFile("./data/ui/ether/world-edit-opensim.ui");
            os_world_edit_widget = loader.load(&uiFile, 0);
            uiFile.close();

            QLineEdit *line_edit;
            line_edit = os_world_edit_widget->findChild<QLineEdit*>("loginURLLineEdit");
            line_edit->setText(data->loginUrl().toString());
            line_edit = os_world_edit_widget->findChild<QLineEdit*>("startLocationLineEdit");
            line_edit->setText(data->startLocation());

            QPushButton *button;
            button = os_world_edit_widget->findChild<QPushButton*>("pushButtonGetGridInfo");
            connect(button, SIGNAL( clicked() ), SLOT( GridInfoRequested() ));

            button = os_world_edit_widget->findChild<QPushButton*>("pushButtonSave");
            connect(button, SIGNAL( clicked() ), SLOT( SaveInformation() ));

            QPixmap pic = CretatePicture(QSize(150,150), data->pixmapPath());
            QLabel *pic_label = os_world_edit_widget->findChild<QLabel*>("pictureLabel");
            pic_label->setPixmap(pic);

            // Grid info from file if it is there
            if (data->gridInfo().count() > 0)
            {
                SetGridInfoToWorldWidget(os_world_edit_widget, data->gridInfo());
                QLabel *status = os_world_edit_widget->findChild<QLabel*>("gridInfoStatusLabel");
                if (status)
                    status->setText("Grid info from local storage");
            }

            // Store data pointer and define types for save function
            current_type_ = "world-opensim";
            current_os_world_data_ = data;
            current_grid_info_map_ = data->gridInfo();

            return os_world_edit_widget;
        }

        QWidget *ActionProxyWidget::CreateNewOpenSimWorld()
        {
            QWidget *new_os_world_widget;
            current_grid_info_map_.clear();

            QUiLoader loader;
            QFile uiFile("./data/ui/ether/world-edit-opensim.ui");
            new_os_world_widget = loader.load(&uiFile, 0);
            uiFile.close();

            QPushButton *button;
            button = new_os_world_widget->findChild<QPushButton*>("pushButtonGetGridInfo");
            connect(button, SIGNAL( clicked() ), SLOT( GridInfoRequested() ));

            button = new_os_world_widget->findChild<QPushButton*>("pushButtonSave");
            connect(button, SIGNAL( clicked() ), SLOT( SaveInformation() ));

            QPixmap pic = CretatePicture(QSize(150,150), "./data/ui/images/ether/world.png");
            QLabel *pic_label = new_os_world_widget->findChild<QLabel*>("pictureLabel");
            pic_label->setPixmap(pic);

            current_type_ = "new-world-opensim";
            current_os_world_data_ = 0;

            return new_os_world_widget;
        }

        void ActionProxyWidget::GridInfoRequested()
        {
            // Lets make a request to protocolutilities to fetch grid info
            QLineEdit *line_edit = current_widget_->findChild<QLineEdit*>("loginURLLineEdit");
            if (!line_edit)
                return;

            QString url_string = line_edit->text();
            if (!url_string.startsWith("http://"))
            {
                url_string = "http://" + url_string;
                line_edit->setText(url_string);
            }

            QUrl login_url(url_string);
            if (login_url.isValid())
            {
                ProtocolUtilities::GridInfoHelper *grid_info_helper = new ProtocolUtilities::GridInfoHelper(this, login_url);
                connect(grid_info_helper, SIGNAL( GridInfoDataRecieved(QMap<QString, QVariant>) ), SLOT( GridInfoRevieced(QMap<QString, QVariant>) ));
                
                QLabel *status = current_widget_->findChild<QLabel*>("gridInfoStatusLabel");
                if (status)
                    status->setText("Retrieving grid info from server...");
                grid_info_helper->GetGridInfo();
            }
            else
            {
                QLabel *status = current_widget_->findChild<QLabel*>("gridInfoStatusLabel");
                if (status)
                    status->setText("Invalid login url");
            }
        }

        void ActionProxyWidget::GridInfoRevieced(QMap<QString, QVariant> grid_info_map)
        {
            // If there is not gridInfoStatusLabel in the current widget, then we wont go further
            QLabel *status = current_widget_->findChild<QLabel*>("gridInfoStatusLabel");
            if (!status)
                return;

            // Test that this is actual grid info by looking if it had the first element <login>
            if (grid_info_map.contains("login"))
            {
                SetGridInfoToWorldWidget(current_widget_, grid_info_map);
                status->setText("Grid info retrieved from server");
                current_grid_info_map_ = grid_info_map;
            }
            else
                status->setText("Could not retrieve grid info from server");
        }

        void ActionProxyWidget::SetGridInfoToWorldWidget(QWidget *world_widget, QMap<QString, QVariant> grid_info_map)
        {
            QLineEdit *line_edit;

            // Grid Nick
            line_edit = world_widget->findChild<QLineEdit*>("nicknameLineEdit");
            if (line_edit && grid_info_map.contains("gridnick"))
                line_edit->setText(grid_info_map["gridnick"].toString());

            // Grid mode
            line_edit = world_widget->findChild<QLineEdit*>("modeLineEdit");
            if (line_edit && grid_info_map.contains("mode"))
                line_edit->setText(grid_info_map["mode"].toString());

            // Help url
            line_edit = world_widget->findChild<QLineEdit*>("helpURLLineEdit");
            if (line_edit && grid_info_map.contains("help"))
                line_edit->setText(grid_info_map["help"].toString());

            // Register url
            line_edit = world_widget->findChild<QLineEdit*>("registerURLLineEdit");
            if (line_edit && grid_info_map.contains("register"))
                line_edit->setText(grid_info_map["register"].toString());

            // Platform
            line_edit = world_widget->findChild<QLineEdit*>("platformLineEdit");
            if (line_edit && grid_info_map.contains("platform"))
                line_edit->setText(grid_info_map["platform"].toString());

            // About url
            line_edit = world_widget->findChild<QLineEdit*>("aboutURLLineEdit");
            if (line_edit && grid_info_map.contains("about"))
                line_edit->setText(grid_info_map["about"].toString());

            // Welcome url
            line_edit = world_widget->findChild<QLineEdit*>("welcomeURLLineEdit");
            if (line_edit && grid_info_map.contains("welcome"))
                line_edit->setText(grid_info_map["welcome"].toString());

            // Password url
            line_edit = world_widget->findChild<QLineEdit*>("passwordURLLineEdit");
            if (line_edit && grid_info_map.contains("password"))
                line_edit->setText(grid_info_map["password"].toString());
        }

        QWidget *ActionProxyWidget::CreateNewRealXtendAvatar()
        {
            QWidget *new_rex_avatar;

            QUiLoader loader;
            QFile uiFile("./data/ui/ether/avatar-edit-realxtend.ui");
            new_rex_avatar = loader.load(&uiFile, 0);
            uiFile.close();

            QPushButton *button = new_rex_avatar->findChild<QPushButton*>("pushButtonSave");
            connect(button, SIGNAL( clicked() ), SLOT( SaveInformation() ));

            QPixmap pic = CretatePicture(QSize(150,150), "./data/ui/images/ether/naali.png");
            QLabel *pic_label = new_rex_avatar->findChild<QLabel*>("pictureLabel");
            pic_label->setPixmap(pic);

            // Store data pointer and define types for save function
            current_type_ = "new-avatar-realxtend";
            current_rex_avatar_data_ = 0;

            return new_rex_avatar;
        }

        QWidget *ActionProxyWidget::RealXtendAvatarInfoWidget(Data::RealXtendAvatar *data)
        {
            return RealXtendAvatarEditWidget(data);
        }

        QWidget *ActionProxyWidget::RealXtendAvatarEditWidget(Data::RealXtendAvatar *data)
        {
            QWidget *rex_edit_widget;

            QUiLoader loader;
            QFile uiFile("./data/ui/ether/avatar-edit-realxtend.ui");
            rex_edit_widget = loader.load(&uiFile, 0);
            uiFile.close();

            QLineEdit *line_edit;
            QLabel *full_account_label = rex_edit_widget->findChild<QLabel*>("fullAccountLabel");
            full_account_label->setText(QString("%1@%2:%3").arg(data->account(), data->authUrl().host(), QString::number(data->authUrl().port())));
            line_edit = rex_edit_widget->findChild<QLineEdit*>("accountLineEdit");
            line_edit->setText(data->account());
            line_edit = rex_edit_widget->findChild<QLineEdit*>("authenticationServerLineEdit");
            line_edit->setText(data->authUrl().toString());
            line_edit = rex_edit_widget->findChild<QLineEdit*>("passwordLineEdit");
            line_edit->setText(data->password());

            QPixmap pic = CretatePicture(QSize(150,150), data->pixmapPath());
            QLabel *pic_label = rex_edit_widget->findChild<QLabel*>("pictureLabel");
            pic_label->setPixmap(pic);

            QPushButton *button = rex_edit_widget->findChild<QPushButton*>("pushButtonSave");
            connect(button, SIGNAL( clicked() ), SLOT( SaveInformation() ));

            // Store data pointer and define types for save function
            current_type_ = "avatar-realxtend";
            current_rex_avatar_data_ = data;

            return rex_edit_widget;
        }

        QPixmap ActionProxyWidget::CretatePicture(QSize image_size, QString pixmap_path)
        {
            QPixmap pixmap;
            pixmap.load(pixmap_path);
            if (pixmap.rect().width() < image_size.width() && pixmap.rect().height() < image_size.height())
                pixmap = pixmap.scaled(image_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            else
            {
                pixmap = pixmap.scaledToHeight(image_size.height(), Qt::SmoothTransformation);
                pixmap = pixmap.copy(QRect(QPoint(pixmap.width()/2-image_size.width()/2,0), QPoint(pixmap.width()/2+image_size.width()/2, pixmap.height())));
            }
            return pixmap;
        }

        QWidget *ActionProxyWidget::AvatarSelectionWidget()
        {
            QWidget *avatar_selection_widget;

            QUiLoader loader;
            QFile uiFile("./data/ui/ether/avatar-selection-widget.ui");
            avatar_selection_widget = loader.load(&uiFile, 0);
            uiFile.close();

            QPushButton *button;
            button = avatar_selection_widget->findChild<QPushButton*>("pushButtonOpenSim");
            connect(button, SIGNAL( clicked() ), SLOT( AvatarSelectedOpenSim() ));
            button = avatar_selection_widget->findChild<QPushButton*>("pushButtonRealXtend");
            connect(button, SIGNAL( clicked() ), SLOT( AvatarSelectedRealXtend() ));

            return avatar_selection_widget;
        }

        void ActionProxyWidget::AvatarSelectedOpenSim()
        {
            current_widget_->hide();
            QWidget *widget = CreateNewOpenSimAvatar();
            ether_action_widget_ui_.widgetLayout->addWidget(widget);
            ether_action_widget_ui_.titleLabel->setText("Creating new OpenSim avatar");
            current_widget_ = widget;
        }

        void ActionProxyWidget::AvatarSelectedRealXtend()
        {
            current_widget_->hide();
            QWidget *widget = CreateNewRealXtendAvatar();
            ether_action_widget_ui_.widgetLayout->addWidget(widget);
            ether_action_widget_ui_.titleLabel->setText("Creating new RealXtend avatar");
            current_widget_ = widget;
        }

        void ActionProxyWidget::SaveInformation()
        {
            if (!current_widget_)
                return;
            
            QLineEdit *line_edit;
            QLabel *status = current_widget_->findChild<QLabel*>("statusLabel");

            // Updating cards
            if (current_type_ == "world-opensim" && current_os_world_data_)
            {
                QUrl login_url;
                QString start_location;
                QString url_string;

                line_edit = current_widget_->findChild<QLineEdit*>("loginURLLineEdit");
                if (line_edit)
                {
                    // Some url validation
                    url_string = line_edit->text();
                    if (!url_string.startsWith("http://") && !url_string.startsWith("https://"))
                        url_string = "http://" + url_string;
                    login_url = QUrl(url_string);
                    if (login_url.port() == -1)
                        login_url.setPort(80);
                    line_edit->setText(login_url.toString());

                    if (login_url.isValid())
                        current_os_world_data_->setLoginUrl(login_url);
                    else if (status)
                    {
                        status->setText("Entered authentication url is invalid");
                        return;
                    }
                }

                line_edit = current_widget_->findChild<QLineEdit*>("startLocationLineEdit");
                if (line_edit)
                {
                    start_location = line_edit->text();
                    current_os_world_data_->setStartLocation(start_location);
                }

                current_os_world_data_->setGridInfo(current_grid_info_map_);
                data_manager_->StoreOrUpdateWorld(current_os_world_data_);
            }
            else if (current_type_ == "avatar-opensim" && current_os_avatar_data_)
            {
                line_edit = current_widget_->findChild<QLineEdit*>("firstNameLineEdit");
                if (line_edit)
                {
                    QString first = line_edit->text();
                    if ((first.isEmpty() || first.contains(" ")) && status)
                    {
                        status->setText("First name can't be empty or have spaces");
                        return;
                    }
                    current_os_avatar_data_->setFirstName(first);
                }

                line_edit = current_widget_->findChild<QLineEdit*>("lastNameLineEdit");
                if (line_edit)
                {
                    QString last = line_edit->text();
                    if ((last.isEmpty() || last.contains(" ")) && status)
                    {
                        status->setText("Last name can't be empty or have spaces");
                        return;
                    }
                    current_os_avatar_data_->setLastName(last);
                }

                line_edit = current_widget_->findChild<QLineEdit*>("passwordLineEdit");
                if (line_edit)
                    current_os_avatar_data_->setPassword(line_edit->text());

                data_manager_->StoreOrUpdateAvatar(current_os_avatar_data_);
            }
            else if (current_type_ == "avatar-realxtend" && current_rex_avatar_data_)
            {
                QString url_string;
                QUrl auth_url;

                line_edit = current_widget_->findChild<QLineEdit*>("accountLineEdit");
                if (line_edit)
                    current_rex_avatar_data_->setAccount(line_edit->text());

                line_edit = current_widget_->findChild<QLineEdit*>("authenticationServerLineEdit");
                if (line_edit)
                {
                    // Some url validation
                    url_string = line_edit->text();
                    if (!url_string.startsWith("http://") && !url_string.startsWith("https://"))
                        url_string = "http://" + url_string;
                    auth_url = QUrl(url_string);
                    if (auth_url.port() == -1)
                        auth_url.setPort(10001);
                    line_edit->setText(auth_url.toString());
                    if (auth_url.isValid())
                        current_rex_avatar_data_->setAuthUrl(auth_url);
                    else if (status)
                    {
                        status->setText("Entered authentication url is invalid");
                        return;
                    }
                }

                line_edit = current_widget_->findChild<QLineEdit*>("passwordLineEdit");
                if (line_edit)
                    current_rex_avatar_data_->setPassword(line_edit->text());

                data_manager_->StoreOrUpdateAvatar(current_rex_avatar_data_);
            }

            // Creating new cards
            else if (current_type_ == "new-avatar-opensim")
            {
                QString first, last, password;
                line_edit = current_widget_->findChild<QLineEdit*>("firstNameLineEdit");
                if (line_edit)
                    first = line_edit->text();

                line_edit = current_widget_->findChild<QLineEdit*>("lastNameLineEdit");
                if (line_edit)
                    last = line_edit->text();

                line_edit = current_widget_->findChild<QLineEdit*>("passwordLineEdit");
                if (line_edit)
                    password = line_edit->text();

                bool spaces_on_names = first.contains(" ");
                if (!spaces_on_names)
                    spaces_on_names = last.contains(" ");
                
                if (!first.isEmpty() && !last.isEmpty() && !spaces_on_names)
                {
                    Data::OpenSimAvatar *new_opensim_avatar = new Data::OpenSimAvatar(first, last, password);
                    data_manager_->StoreOrUpdateAvatar(new_opensim_avatar);
                }
                else if (status)
                {
                    if (spaces_on_names)
                        status->setText("You can't have spaces in first or last name");
                    else
                        status->setText("Both first and last name are required");
                    return;
                }
            }
            else if (current_type_ == "new-avatar-realxtend")
            {
                QString account, password, url_string;
                QUrl auth_url;

                line_edit = current_widget_->findChild<QLineEdit*>("accountLineEdit");
                if (line_edit)
                    account =line_edit->text();

                line_edit = current_widget_->findChild<QLineEdit*>("authenticationServerLineEdit");
                if (line_edit)
                {
                    url_string = line_edit->text();
                    if (!url_string.startsWith("http://") && !url_string.startsWith("https://"))
                        url_string = "http://" + url_string;
                    auth_url = QUrl(url_string);
                    if (auth_url.port() == -1)
                        auth_url.setPort(10001);
                    line_edit->setText(auth_url.toString());
                }

                line_edit = current_widget_->findChild<QLineEdit*>("passwordLineEdit");
                if (line_edit)
                    password = line_edit->text();


                if (!account.isEmpty() && !password.isEmpty() && auth_url.isValid())
                {
                    Data::RealXtendAvatar *new_realxtend_avatar = new Data::RealXtendAvatar(account, auth_url, password);
                    data_manager_->StoreOrUpdateAvatar(new_realxtend_avatar);
                }
                else if (status)
                {
                    if (!auth_url.isValid())
                        status->setText("Entered authentication url is invalid");
                    else
                        status->setText("Fill all the fields before saving");
                    return;
                }
            }
            else if (current_type_ == "new-world-opensim")
            {
                QUrl login_url;
                QString url_string;
                QString start_location;

                line_edit = current_widget_->findChild<QLineEdit*>("loginURLLineEdit");
                if (line_edit)
                {
                    url_string = line_edit->text();
                    if (!url_string.startsWith("http://") && !url_string.startsWith("https://"))
                        url_string = "http://" + url_string;
                    login_url = QUrl(url_string);
                    if (login_url.port() == -1)
                        login_url.setPort(80);
                    line_edit->setText(login_url.toString());
                }

                line_edit = current_widget_->findChild<QLineEdit*>("startLocationLineEdit");
                if (line_edit)
                {
                    start_location = line_edit->text();
                }

                if (login_url.isValid())
                {
					Data::OpenSimWorld *new_opensim_world = new Data::OpenSimWorld(login_url, start_location, current_grid_info_map_);
                    data_manager_->StoreOrUpdateWorld(new_opensim_world);
                }
                else if (status)
                {
                    status->setText("Entered login url is invalid");
                    return;
                }
            }

            // Start hiding widget due save was clicked
            StartHideAnimations();
        }
    }
}