// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_UiHelper_h
#define incl_WorldBuildingModule_UiHelper_h

#include <EC_OpenSimPrim.h>

#include <QObject>
#include <QString>
#include <QMap>

#include <QtVariantPropertyManager>
#include <QtVariantEditorFactory>
#include <QtTreePropertyBrowser>
#include <QtVariantProperty>

#include <QDebug>

namespace WorldBuilding
{
    namespace Helpers
    {
        class UiHelper
        {

        public:
            // Constructor
            UiHelper() : variant_manager(0), browser(0)
            {
                information_items << "Name" << "Description";

                rex_prim_data_items << "DrawType" << "IsVisible" << "CastShadows" << "LightCreatesShadows"
                                    << "DescriptionTexture" << "ScaleToPrim" << "DrawDistance" << "LOD" << "MeshID" << "CollisionMeshID"
                                    << "ParticleScriptID" << "AnimationPackageID" << "AnimationName" << "AnimationRate" << "ServerScriptClass"
                                    << "SoundID" << "SoundVolume" << "SoundRadius" << "SelectPriority";

                object_shape_items << "PathCurve" << "ProfileCurve" << "PathBegin" << "PathEnd" << "PathScaleX"
                                   << "PathScaleY" << "PathShearX" << "PathShearY" << "PathTwist" << "PathTwistBegin" << "PathRadiusOffset"
                                   << "PathTaperX" << "PathTaperY" /*<< "PathRevolutions"*/ << "PathSkew" << "ProfileBegin" << "ProfileEnd" << "ProfileHollow";
            }

            // Public containers
            QMap<QString, QtVariantProperty*> editor_items;
            QStringList information_items, rex_prim_data_items, object_shape_items;

            // Browser pointers
            QtVariantPropertyManager *variant_manager;
            QtTreePropertyBrowser *browser;

            // Public helper functions

            QString CheckUiValue(QString value)
            {
                if (value.isEmpty())
                    value = "-";
                return value;
            }

            QString CheckUiValue(unsigned int value)
            {
                return QString::number(value);
            }

            QtTreePropertyBrowser *CreatePropertyBrowser(QWidget *parent, QObject *controller, EC_OpenSimPrim *prim)
            {
                if (browser)
                    return 0;

                // Managers and Factories
                QtVariantEditorFactory *variant_factory = new QtVariantEditorFactory();
                variant_manager = new QtVariantPropertyManager();

                // Init browser
                browser = new QtTreePropertyBrowser(parent);
                browser->setFactoryForManager(variant_manager, variant_factory);

                CollapseSubGroups(browser->addProperty(CreateInformationGroup(variant_manager, prim)));
                CollapseSubGroups(browser->addProperty(CreateRexPrimDataGroup(variant_manager, prim)));
                CollapseSubGroups(browser->addProperty(CreateShapeGroup(variant_manager, prim)));

                QObject::connect(variant_manager, SIGNAL(valueChanged(QtProperty*, const QVariant&)), 
                        controller, SLOT(PrimValueChanged(QtProperty*, const QVariant&)));

                browser->setPropertiesWithoutValueMarked(true);
                browser->setRootIsDecorated(true);
                browser->setHeaderVisible(false);
                browser->setResizeMode(QtTreePropertyBrowser::Fixed);
                browser->setSplitterPosition(190);

                return browser;
            }

            void CollapseSubGroups(QtBrowserItem *main_group)
            {
                foreach (QtBrowserItem *sub_group, main_group->children())
                    if (sub_group->property()->subProperties().count() > 0)
                        browser->setExpanded(sub_group, false);
            }

            QtProperty *CreateInformationGroup(QtVariantPropertyManager *variant_manager, EC_OpenSimPrim *prim)
            {
                // General Information
                QtProperty *title_group = variant_manager->addProperty(QtVariantPropertyManager::groupTypeId(), "Information");
                foreach(QString item_name, information_items)
                {
                    QtVariantProperty *item = variant_manager->addProperty(QVariant::String, item_name);
                    item->setValue(prim->property(item_name.toStdString().c_str()));
                    title_group->addSubProperty(item);
                    editor_items[item_name] = item;
                }
                return title_group;
            }

            QtProperty *CreateRexPrimDataGroup(QtVariantPropertyManager *variant_manager, EC_OpenSimPrim *prim)
            {
                // Rex Prim Data
                QtProperty *title_group = variant_manager->addProperty(QtVariantPropertyManager::groupTypeId(), "realXtend Properties");
                QtVariantProperty *title_flags_group = variant_manager->addProperty(QVariant::String, "Flags");
                QtVariantProperty *title_script_group = variant_manager->addProperty(QVariant::String, "Scripting");
                QtVariantProperty *title_animation_group = variant_manager->addProperty(QVariant::String, "Animation");
                QtVariantProperty *title_sound_group = variant_manager->addProperty(QVariant::String, "Sound");
                QtVariantProperty *title_appearance_group = variant_manager->addProperty(QVariant::String, "Appearance");

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
                    QtVariantProperty *item = 0;
                    if (variant_manager->isPropertyTypeSupported(item_type))
                        item = variant_manager->addProperty(item_type, item_name);
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
            
                    // Set value and add to tree
                    item->setValue(current_property);
                    group->addSubProperty(item);
                    editor_items[item_name] = item;
                }

                title_flags_group->setValue(QString("(%1 items)").arg(title_flags_group->subProperties().count()));
                title_flags_group->setWhatsThis(title_flags_group->value().toString());
                title_group->addSubProperty(title_flags_group);

                title_script_group->setValue(QString("(%1 items)").arg(title_script_group->subProperties().count()));
                title_script_group->setWhatsThis(title_script_group->value().toString());
                title_group->addSubProperty(title_script_group);

                title_animation_group->setValue(QString("(%1 items)").arg(title_animation_group->subProperties().count()));
                title_animation_group->setWhatsThis(title_animation_group->value().toString());
                title_group->addSubProperty(title_animation_group);

                title_sound_group->setValue(QString("(%1 items)").arg(title_sound_group->subProperties().count()));
                title_sound_group->setWhatsThis(title_sound_group->value().toString());
                title_group->addSubProperty(title_sound_group);

                title_appearance_group->setValue(QString("(%1 items)").arg(title_appearance_group->subProperties().count()));
                title_appearance_group->setWhatsThis(title_appearance_group->value().toString());
                title_group->addSubProperty(title_appearance_group);

                return title_group;
            }

            QtProperty *CreateShapeGroup(QtVariantPropertyManager *variant_manager, EC_OpenSimPrim *prim)
            {
                // Rex Prim Data
                QtProperty *title_group = variant_manager->addProperty(QtVariantPropertyManager::groupTypeId(), "Shape Properties");
                QtVariantProperty *title_group_profile = variant_manager->addProperty(QVariant::String, "Profile");
                QtVariantProperty *title_group_path = variant_manager->addProperty(QVariant::String, "Path");

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
                    QtVariantProperty *item = 0;
                    if (variant_manager->isPropertyTypeSupported(item_type))
                        item = variant_manager->addProperty(item_type, item_name);
                    if (!item)
                        continue;

                    // Set value and add to tree
                    item->setValue(current_property);

                    if (item_name.startsWith("Profile"))
                        title_group_profile->addSubProperty(item);
                    else if(item_name.startsWith("Path"))
                        title_group_path->addSubProperty(item);
                    else
                        continue;

                    editor_items[item_name] = item;
                }

                title_group_profile->setValue(QString("(%1 items)").arg(title_group_profile->subProperties().count()));
                title_group->addSubProperty(title_group_profile);
                title_group_path->setValue(QString("(%1 items)").arg(title_group_path->subProperties().count()));
                title_group->addSubProperty(title_group_path);
                return title_group;
            }
        };
    }
}

#endif