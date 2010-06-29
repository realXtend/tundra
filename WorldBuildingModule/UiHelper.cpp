// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UiHelper.h"

namespace WorldBuilding
{
    namespace Helpers
    {
        UiHelper::UiHelper() : variant_manager(0), browser(0)
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
    }
}