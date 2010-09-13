// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EventManager.h"

#include "InventoryModule.h"
#include "InventoryService.h"
#include "AbstractInventoryDataModel.h"
#include "InventoryFolder.h"
#include "InventoryAsset.h"

#include "UiHelper.h"

namespace WorldBuilding
{
    namespace Helpers
    {
        UiHelper::UiHelper(QObject *parent, Foundation::Framework *framework) :
            QObject(parent),
            framework_(framework),
            variant_manager(0),
            browser(0),
            manip_ui_(0),
            ignore_manip_changes_(false)
        {
            information_items << "Name" << "Description";

            rex_prim_data_items << "DrawType" << "IsVisible" << "CastShadows" << "LightCreatesShadows"
                                << "DescriptionTexture" << "ScaleToPrim" << "DrawDistance" << "LOD" << "MeshID" << "CollisionMeshID"
                                << "ParticleScriptID" << "AnimationPackageID" << "AnimationName" << "AnimationRate" << "ServerScriptClass"
                                << "SoundID" << "SoundVolume" << "SoundRadius" << "SelectPriority";

            object_shape_items << "PathCurve" << "ProfileCurve" << "PathBegin" << "PathEnd" << "PathScaleX"
                               << "PathScaleY" << "PathShearX" << "PathShearY" << "PathTwist" << "PathTwistBegin" << "PathRadiusOffset"
                               << "PathTaperX" << "PathTaperY" /*<< "PathRevolutions"*/ << "PathSkew" << "ProfileBegin" << "ProfileEnd" << "ProfileHollow";

            shape_limiter_zero_to_one << "PathBegin" << "PathEnd" << "PathScaleX" << "PathScaleY"
                                      << "ProfileBegin" << "ProfileEnd" << "ProfileHollow";

            shape_limiter_minusone_to_plusone << "PathTwist" << "PathTwistBegin" << "PathRadiusOffset"
                                              << "PathTaperX" << "PathTaperY" << "PathSkew";

            shape_limiter_minushalf_to_plushalf << "PathShearX" << "PathShearY";

            shape_limiter_zero_to_three << "PathRevolutions";

            mapper_ = new QSignalMapper(this);
            connect(mapper_, SIGNAL(mapped(QWidget*)), SLOT(BrowseClicked(QWidget*)));
        }

        // Public helper slots

        QString UiHelper::CheckUiValue(QString value)
        {
            if (value.isEmpty())
                value = "-";
            return value;
        }

        QString UiHelper::CheckUiValue(unsigned int value)
        {
            return QString::number(value);
        }

        QtTreePropertyBrowser *UiHelper::CreatePropertyBrowser(QWidget *parent, QObject *controller, EC_OpenSimPrim *prim)
        {
            if (browser)
                return 0;

            // Managers and Factories
            QtVariantEditorFactory *variant_factory = new QtVariantEditorFactory(controller);
            variant_manager = new QtVariantPropertyManager(controller);

            CustomLineEditFactory *line_edit_factory = new CustomLineEditFactory(controller);
            string_manager = new QtStringPropertyManager(controller);

            // Init browser
            browser = new QtTreePropertyBrowser(parent);
            browser->setFactoryForManager(variant_manager, variant_factory);
            browser->setFactoryForManager(string_manager, line_edit_factory);

            CollapseSubGroups(browser->addProperty(CreateInformationGroup(variant_manager, prim)));
            CollapseSubGroups(browser->addProperty(CreateRexPrimDataGroup(variant_manager, prim)));
            CollapseSubGroups(browser->addProperty(CreateShapeGroup(variant_manager, prim)));

            browser->setPropertiesWithoutValueMarked(true);
            browser->setRootIsDecorated(true);
            browser->setHeaderVisible(false);
            browser->setResizeMode(QtTreePropertyBrowser::Fixed);
            browser->setSplitterPosition(190);
            browser->setMinimumHeight(300);

            return browser;
        }

        // Private slots

        void UiHelper::CollapseSubGroups(QtBrowserItem *main_group)
        {
            foreach (QtBrowserItem *sub_group, main_group->children())
                if (sub_group->property()->subProperties().count() > 0)
                    browser->setExpanded(sub_group, false);
        }

        QtProperty *UiHelper::CreateInformationGroup(QtVariantPropertyManager *variant_manager, EC_OpenSimPrim *prim)
        {
            // General Information
            QtProperty *title_group = variant_manager->addProperty(QtVariantPropertyManager::groupTypeId(), "Information");
            foreach(QString item_name, information_items)
            {
                QtProperty *item = string_manager->addProperty(item_name);
                string_manager->setValue(item, prim->property(item_name.toStdString().c_str()).toString());
                title_group->addSubProperty(item);
                editor_items[item_name] = item;
            }
            return title_group;
        }

        QtProperty *UiHelper::CreateRexPrimDataGroup(QtVariantPropertyManager *variant_manager, EC_OpenSimPrim *prim)
        {
            // Rex Prim Data
            QtProperty *title_group = variant_manager->addProperty(QtVariantPropertyManager::groupTypeId(), "realXtend Properties");
            QtProperty *title_flags_group = string_manager->addProperty("Flags");
            QtProperty *title_script_group = string_manager->addProperty("Scripting");
            QtProperty *title_animation_group = string_manager->addProperty("Animation");
            QtProperty *title_sound_group = string_manager->addProperty("Sound");
            QtProperty *title_appearance_group = string_manager->addProperty("Appearance");

            foreach(QString item_name, rex_prim_data_items)
            {
                QVariant current_property = prim->property(item_name.toStdString().c_str());
                QVariant::Type item_type = current_property.type();
                if (current_property.isNull() || !current_property.isValid() || item_type == QVariant::Invalid)
                    continue;

                // Our manager doesent like uint that much, lets convert and hope for the best ;)
                if (item_type == QVariant::LongLong || item_type == QVariant::UInt || item_type == QVariant::ULongLong)
                {
                    current_property = QVariant(current_property.toInt());
                    if (current_property.toInt() < 0)
                        continue; // omg!
                    item_type = current_property.type();
                }

                // Appropriate manager creates the item
                QtProperty *item = 0;
                if (item_type == QVariant::String)
                {
                    item = string_manager->addProperty(item_name);
                    string_manager->setValue(item, current_property.toString());
                }
                else if (variant_manager->isPropertyTypeSupported(item_type))
                {
                    item = variant_manager->addProperty(item_type, item_name);
                    variant_manager->setValue(item, current_property);

                    // Limiters for rex prim data values and steps here if needed!
                }
                if (!item)
                    continue;

                // Check group to add
                QtProperty *group = title_group;
                if (item_name == "ServerScriptClass")
                    group = title_script_group;
                else if (item_type == QVariant::Bool)
                    group = title_flags_group;
                else if (item_name.startsWith("Sound"))
                    group = title_sound_group;
                else if (item_name.startsWith("Animation"))
                    group = title_animation_group;
                else if (item_name.contains("ID"))
                    group = title_appearance_group;
                else
                    int i = 0;
        
                // Add to tree and to internals
                group->addSubProperty(item);
                editor_items[item_name] = item;
            }

            string_manager->setValue(title_flags_group, QString("(%1 items)").arg(title_flags_group->subProperties().count()));
            title_flags_group->setWhatsThis(title_flags_group->valueText());
            title_group->addSubProperty(title_flags_group);

            string_manager->setValue(title_script_group, QString("(%1 items)").arg(title_script_group->subProperties().count()));
            title_script_group->setWhatsThis(title_script_group->valueText());
            title_group->addSubProperty(title_script_group);

            string_manager->setValue(title_animation_group, QString("(%1 items)").arg(title_animation_group->subProperties().count()));
            title_animation_group->setWhatsThis(title_animation_group->valueText());
            title_group->addSubProperty(title_animation_group);

            string_manager->setValue(title_sound_group, QString("(%1 items)").arg(title_sound_group->subProperties().count()));
            title_sound_group->setWhatsThis(title_sound_group->valueText());
            title_group->addSubProperty(title_sound_group);

            string_manager->setValue(title_appearance_group, QString("(%1 items)").arg(title_appearance_group->subProperties().count()));
            title_appearance_group->setWhatsThis(title_appearance_group->valueText());
            title_group->addSubProperty(title_appearance_group);

            return title_group;
        }

        QtProperty *UiHelper::CreateShapeGroup(QtVariantPropertyManager *variant_manager, EC_OpenSimPrim *prim)
        {
            // Rex Prim Data
            QtProperty *title_group = variant_manager->addProperty(QtVariantPropertyManager::groupTypeId(), "Shape Properties");
            QtProperty *title_group_profile = string_manager->addProperty("Profile");
            QtProperty *title_group_path = string_manager->addProperty("Path");

            foreach(QString item_name, object_shape_items)
            {
                QVariant current_property = prim->property(item_name.toStdString().c_str());
                QVariant::Type item_type = current_property.type();
                if (current_property.isNull() || !current_property.isValid() || item_type == QVariant::Invalid)
                    continue;

                // Our manager doesent like uint that much, lets convert and hope for the best ;)
                if (item_type == QVariant::LongLong || item_type == QVariant::UInt || item_type == QVariant::ULongLong)
                {
                    current_property = QVariant(current_property.toInt());
                    if (current_property.toInt() < 0)
                        continue; // omg!
                    item_type = current_property.type();
                }

                // Appropriate manager creates the item
                QtProperty *item = 0;
                if (item_type == QVariant::String)
                {
                    item = string_manager->addProperty(item_name);
                    string_manager->setValue(item, current_property.toString());
                }
                else if (variant_manager->isPropertyTypeSupported(item_type))
                {
                    item = variant_manager->addProperty(item_type, item_name);
                    variant_manager->setValue(item, current_property);

                    if (item_type == QVariant::Double)
                    {
                        QtVariantProperty *variant_item = dynamic_cast<QtVariantProperty*>(item);
                        if (variant_item)
                        {
                            if (shape_limiter_zero_to_one.contains(item_name))
                            {
                                variant_item->setAttribute("minimum", 0.0);
                                variant_item->setAttribute("maximum", 1.0);
                                variant_item->setAttribute("decimals", 2);
                                variant_item->setAttribute("singleStep", 0.1);
                            }
                            else if (shape_limiter_minushalf_to_plushalf.contains(item_name))
                            {
                                variant_item->setAttribute("minimum", -0.5);
                                variant_item->setAttribute("maximum", 0.5);
                                variant_item->setAttribute("decimals", 2);
                                variant_item->setAttribute("singleStep", 0.1);
                            }
                            else if (shape_limiter_minusone_to_plusone.contains(item_name))
                            {
                                variant_item->setAttribute("minimum", -1.0);
                                variant_item->setAttribute("maximum", 1.0);
                                variant_item->setAttribute("decimals", 2);
                                variant_item->setAttribute("singleStep", 0.1);
                            }
                            else if (shape_limiter_zero_to_three.contains(item_name))
                            {
                                variant_item->setAttribute("minimum", 0.0);
                                variant_item->setAttribute("maximum", 3.0);
                                variant_item->setAttribute("decimals", 3);
                                variant_item->setAttribute("singleStep", 0.015);
                            }
                        }
                    }
                }
                if (!item)
                    continue;

                // Add to tree and internals
                if (item_name.startsWith("Profile"))
                    title_group_profile->addSubProperty(item);
                else if(item_name.startsWith("Path"))
                    title_group_path->addSubProperty(item);
                else
                    continue;
                editor_items[item_name] = item;
            }

            string_manager->setValue(title_group_profile, QString("(%1 items)").arg(title_group_profile->subProperties().count()));
            title_group_profile->setWhatsThis(title_group_profile->valueText());
            title_group->addSubProperty(title_group_profile);

            string_manager->setValue(title_group_path, QString("(%1 items)").arg(title_group_path->subProperties().count()));
            title_group_path->setWhatsThis(title_group_path->valueText());
            title_group->addSubProperty(title_group_path);

            return title_group;
        }

        void UiHelper::SetupManipControls(Ui_ObjectManipulationsWidget *manip_ui, QObject *python_handler)
        {
            manip_ui_ = manip_ui;
            
            connect(python_handler, SIGNAL(RotateValuesToUi(int, int, int)), SLOT(SetRotateValues(int, int, int)));
            connect(this, SIGNAL(RotationChanged(int, int, int)), python_handler, SLOT(EmitRotateChange(int, int, int)));
            connect(manip_ui_->slider_rotate_x, SIGNAL(valueChanged(int)), SLOT(RotateXChanged(int)));
            connect(manip_ui_->slider_rotate_y, SIGNAL(valueChanged(int)), SLOT(RotateYChanged(int)));
            connect(manip_ui_->slider_rotate_z, SIGNAL(valueChanged(int)), SLOT(RotateZChanged(int)));

            connect(python_handler, SIGNAL(ScaleValuesToUi(double, double, double)), SLOT(SetScaleValues(double, double, double)));
            connect(this, SIGNAL(ScaleChanged(double, double, double)), python_handler, SLOT(EmitScaleChange(double, double, double)));
            connect(manip_ui_->doubleSpinBox_scale_x, SIGNAL(valueChanged(double)), SLOT(OnScaleChanged(double)));
            connect(manip_ui_->doubleSpinBox_scale_y, SIGNAL(valueChanged(double)), SLOT(OnScaleChanged(double)));
            connect(manip_ui_->doubleSpinBox_scale_z, SIGNAL(valueChanged(double)), SLOT(OnScaleChanged(double)));

            connect(python_handler, SIGNAL(PosValuesToUi(double, double, double)), SLOT(SetPosValues(double, double, double)));
            connect(this, SIGNAL(PosChanged(double, double, double)), python_handler, SLOT(EmitPosChange(double, double, double)));
            connect(manip_ui_->doubleSpinBox_pos_x, SIGNAL(valueChanged(double)), SLOT(OnPosChanged(double)));
            connect(manip_ui_->doubleSpinBox_pos_y, SIGNAL(valueChanged(double)), SLOT(OnPosChanged(double)));
            connect(manip_ui_->doubleSpinBox_pos_z, SIGNAL(valueChanged(double)), SLOT(OnPosChanged(double)));
        }

        void UiHelper::SetRotateValues(int x, int y, int z)
        {
            if (x < 0)
                x += 360;
            if (y < 0)
                y += 360;
            if (z < 0)
                z += 360;

            ignore_manip_changes_ = true;
            manip_ui_->slider_rotate_z->setValue(z);
            manip_ui_->slider_rotate_x->setValue(x);
            manip_ui_->slider_rotate_y->setValue(y);
            ignore_manip_changes_ = false;
        }

        void UiHelper::RotateXChanged(int value)
        {
            manip_ui_->label_rotate_x_value->setText(QString("%1 °").arg(QString::number(value)));
            if (!ignore_manip_changes_)
                emit RotationChanged(value, manip_ui_->slider_rotate_y->value(), manip_ui_->slider_rotate_z->value());
        }

        void UiHelper::RotateYChanged(int value)
        {
            manip_ui_->label_rotate_y_value->setText(QString("%1 °").arg(QString::number(value)));
            if (!ignore_manip_changes_)
                emit RotationChanged(manip_ui_->slider_rotate_x->value(), value, manip_ui_->slider_rotate_z->value());
        }

        void UiHelper::RotateZChanged(int value)
        {
            manip_ui_->label_rotate_z_value->setText(QString("%1 °").arg(QString::number(value)));
            if (!ignore_manip_changes_)           
                emit RotationChanged(manip_ui_->slider_rotate_x->value(), manip_ui_->slider_rotate_y->value(), value);
        }

        void UiHelper::SetScaleValues(double x, double y, double z)
        {
            ignore_manip_changes_ = true;
            manip_ui_->doubleSpinBox_scale_x->setValue(x);
            manip_ui_->doubleSpinBox_scale_y->setValue(y);
            manip_ui_->doubleSpinBox_scale_z->setValue(z);
            ignore_manip_changes_ = false;
        }

        void UiHelper::OnScaleChanged(double value)
        {
            if (ignore_manip_changes_)
                return;
            emit ScaleChanged(manip_ui_->doubleSpinBox_scale_x->value(), manip_ui_->doubleSpinBox_scale_y->value(), manip_ui_->doubleSpinBox_scale_z->value());
        }

        void UiHelper::SetPosValues(double x, double y, double z)
        {
            ignore_manip_changes_ = true;
            manip_ui_->doubleSpinBox_pos_x->setValue(x);
            manip_ui_->doubleSpinBox_pos_y->setValue(y);
            manip_ui_->doubleSpinBox_pos_z->setValue(z);
            ignore_manip_changes_ = false;
        }

        void UiHelper::OnPosChanged(double value)
        {
            if (ignore_manip_changes_)
                return;
            emit PosChanged(manip_ui_->doubleSpinBox_pos_x->value(), manip_ui_->doubleSpinBox_pos_y->value(), manip_ui_->doubleSpinBox_pos_z->value());
        }

        void UiHelper::SetupVisibilityButtons(AnchorLayout *layout, Ui::BuildingWidget *manip_ui, Ui::BuildingWidget *info_ui)
        {
            if (!manip_ui->scene())
                return;

            // Manip hide
            QPushButton *vibility_button = new QPushButton();
            vibility_button->setFlat(true);
            vibility_button->setFixedSize(32,32);
            vibility_button->setStyleSheet("QPushButton { background-color: transparent; background-image: url('./data/ui/images/worldbuilding/draw-arrow-back.png'); } QPushButton::hover { background-color: transparent; background-image: url('./data/ui/images/worldbuilding/draw-arrow-back_bright.png'); } QPushButton::pressed { background-color: transparent; border: 0px; }");
            
            manip_ui->SetVisibilityButton(vibility_button);
            connect(vibility_button, SIGNAL(clicked()), manip_ui, SLOT(ToggleVisibility()));

            QGraphicsProxyWidget *proxy = manip_ui->scene()->addWidget(vibility_button);
            layout->AnchorWidgetsHorizontally(proxy, manip_ui);
            layout->AnchorItemToLayout(proxy, Qt::AnchorBottom, Qt::AnchorBottom);

            // Info hide
            vibility_button = new QPushButton();
            vibility_button->setFlat(true);
            vibility_button->setFixedSize(32,32);
            vibility_button->setStyleSheet("QPushButton { background-color: transparent; background-image: url('./data/ui/images/worldbuilding/draw-arrow-forward.png'); } QPushButton::hover { background-color: transparent; background-image: url('./data/ui/images/worldbuilding/draw-arrow-forward_bright.png'); } QPushButton::pressed { background-color: transparent; border: 0px; }");
            
            info_ui->SetVisibilityButton(vibility_button);
            connect(vibility_button, SIGNAL(clicked()), info_ui, SLOT(ToggleVisibility()));

            proxy = manip_ui->scene()->addWidget(vibility_button);
            layout->AnchorWidgetsHorizontally(info_ui, proxy);
            layout->AnchorItemToLayout(proxy, Qt::AnchorBottom, Qt::AnchorBottom);
        }

        void UiHelper::AddBrowsePair(QString name, QPushButton *button, QWidget *tool_widget)
        {
            if (browser_pairs_.contains(name))
                return;
            browser_pairs_[name] = QPair<QPushButton*,QWidget*>(button,tool_widget);
            mapper_->setMapping(button, button);
            connect(button, SIGNAL(clicked()), mapper_, SLOT(map()));
        }

        void UiHelper::BrowseClicked(QWidget *widget_ptr)
        {
            foreach (QString key, browser_pairs_.keys())
            {
                QPair<QPushButton*,QWidget*> pair = browser_pairs_[key];
                if (pair.first == widget_ptr)
                {
                    if (key == "mesh")
                        BrowserAndUpload(key, "mesh", "3D Models", pair.second);
                    else if (key == "animation")
                        BrowserAndUpload(key, "skeleton", "Animations", pair.second);
                    else if (key == "sound")
                        BrowserAndUpload(key, "ogg", "Sounds", pair.second);
                }
            }
        }

        void UiHelper::BrowserAndUpload(QString category, QString filter, QString upload_to, QWidget *tool_widget)
        {
            using namespace Inventory;

            // Get inventory module
            boost::shared_ptr<InventoryModule> inv_module = framework_->GetModuleManager()->GetModule<InventoryModule>().lock();
            if (!inv_module)
                return;

            // Get data model and service
            InventoryPtr inv_data = inv_module->GetInventoryPtr();
            InventoryService *inv_serv = inv_module->GetInventoryService();
            InventoryModule::InventoryDataModelType inv_type = inv_module->GetInventoryType();
            if (!inv_data || !inv_serv)
                return;

            disconnect(inv_data.get(), SIGNAL(UploadCompleted(const QString&, const QString&)),
                       this, SLOT(AssetUploadCompleted(const QString&, const QString&)));
            connect(inv_data.get(), SIGNAL(UploadCompleted(const QString&, const QString&)),
                    this, SLOT(AssetUploadCompleted(const QString&, const QString&)));

            QString filename = QFileDialog::getOpenFileName(0, "Setting " + category + " to object", QString(), "*." + filter);
            if (filename.isEmpty())
                return;
            QString only_name = filename.right(filename.length() - (filename.lastIndexOf("/")+1));

            // Check for parent folder
            AbstractInventoryItem *upload_folder = inv_data->GetFirstChildFolderByName(upload_to);
            if (!upload_folder)
            {
                upload_folder = inv_data->GetOrCreateNewFolder(RexUUID::CreateRandom().ToQString(), *inv_data->GetFirstChildFolderByName("My Inventory"), upload_to);
                if (!upload_folder)
                    return; // i'll get me coat sir
            }
            filename = filename.replace("/", "\\"); // webdav lib requires this
            
            pending_uploads_[only_name] = QPair<QWidget*, QString>(tool_widget, category);
            inv_data->UploadFile(filename, upload_folder); // upload
        }

        void UiHelper::AssetUploadCompleted(const QString &filename, const QString &asset_ref)
        {
            if (!pending_uploads_.contains(filename))
                return;

            QPair<QWidget*, QString> data_pair = pending_uploads_[filename];
            QWidget *tool_widget = data_pair.first;
            QString category = data_pair.second;

            if (tool_widget)
            {
                QLineEdit *line_edit = tool_widget->findChild<QLineEdit*>(category + "LineEdit");
                QPushButton *apply_button = tool_widget->findChild<QPushButton*>("Apply");
                if (line_edit && apply_button)
                {
                    line_edit->setText(asset_ref);
                    if (category == "sound")
                    {
                        QDoubleSpinBox *dsp = tool_widget->findChild<QDoubleSpinBox*>("soundVolume");
                        if (dsp)
                            dsp->setValue(1.0);
                    }
                    apply_button->setEnabled(true);
                    apply_button->click();
                }
            }
            pending_uploads_.remove(filename);
        }
    }
}