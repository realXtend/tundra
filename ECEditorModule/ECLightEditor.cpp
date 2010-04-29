#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ECLightEditor.h"
#include "Foundation.h"
#include "ComponentInterface.h"
#include "SceneEvents.h"
#include "EventManager.h"
#include "SceneManager.h"
#include "EC_Light.h"

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    AbstractECEditor::AbstractECEditor(QtVariantPropertyManager *propertyManager, Foundation::ComponentInterfacePtr component):
        variantManager_(propertyManager),
        rootProperty_(0),
        componentInterface_(component)
    {

    }
    
    AbstractECEditor::~AbstractECEditor()
    {

    }

    QtVariantProperty *AbstractECEditor::GetRootProperty() const
    {
        return rootProperty_;
    }

    bool AbstractECEditor::ContainProperty(QtProperty *property) const
    {
        QString proeprtyName = property->propertyName();
        VariantPropertyMap::const_iterator const_iter = propertyMap_.find(proeprtyName);
        if(const_iter != propertyMap_.end())
            return true;
        return false;
    }

    void AbstractECEditor::SetPropertyValue(QtProperty *property)
    {
        VariantPropertyMap::Iterator iter = propertyMap_.find(property->propertyName());
        if(iter != propertyMap_.end())
        {
            SetValue(property);
        }
    }

    ECLightEditor::ECLightEditor(QtVariantPropertyManager *propertyManager, Foundation::ComponentInterfacePtr component):
        AbstractECEditor(propertyManager, component)
    {
        Initialize();
    }

    ECLightEditor::~ECLightEditor()
    {

    }

    void ECLightEditor::UpdateProperties()
    {
        if(componentInterface_.expired() && variantManager_ && propertyMap_.empty())
            return;

        Foundation::ComponentInterfacePtr componentPtr = componentInterface_.lock();
        EC_Light *lightComponent = dynamic_cast<EC_Light*>(componentPtr.get());
        if(lightComponent)
        {
            EC_Light::Type lightType;
            switch(lightComponent->typeAttr_.Get())
            {
            case EC_Light::LT_Point:
                lightType = EC_Light::LT_Point;
                break;
            case EC_Light::LT_Spot:
                lightType = EC_Light::LT_Spot;
                break;
            case EC_Light::LT_Directional:
                lightType = EC_Light::LT_Directional;
                break;
            }
            propertyMap_["Light type"]->setValue(lightType);

            QList<QtProperty *> subProperties = propertyMap_["Direction"]->subProperties();
            for(uint i = 0; i < subProperties.size(); i++)
            {
                QtVariantProperty *variant = dynamic_cast<QtVariantProperty *>(subProperties[i]);
                if(variant->propertyName() == "x")
                    variant->setValue(lightComponent->directionAttr_.Get().x);
                else if(variant->propertyName() == "y")
                    variant->setValue(lightComponent->directionAttr_.Get().y);
                else if(variant->propertyName() == "z")
                    variant->setValue(lightComponent->directionAttr_.Get().z);
            }
            
            Color color = lightComponent->diffColorAttr_.Get();
            propertyMap_["diffuse color"]->setValue(QColor(color.r * 255, color.g * 255, color.b * 255, color.a * 255));

            color = lightComponent->specColorAttr_.Get();
            propertyMap_["specular color"]->setValue(QColor(color.r * 255, color.g * 255, color.b * 255, color.a * 255));

            if(lightComponent->castShadowsAttr_.Get())
                propertyMap_["Cast shadows"]->setValue(true);
            else
                propertyMap_["Cast shadows"]->setValue(false);

            propertyMap_["Range"]->setValue(lightComponent->rangeAttr_.Get());

            subProperties = propertyMap_["Attenuation"]->subProperties();
            for(uint i = 0; i < subProperties.size(); i++)
            {
                QtVariantProperty *variant = dynamic_cast<QtVariantProperty *>(subProperties[i]);
                if(variant->propertyName() == "Const atten")
                    variant->setValue(lightComponent->constAttenAttr_.Get());
                else if(variant->propertyName() == "Linear atten")
                    variant->setValue(lightComponent->linearAttenAttr_.Get());
                else if(variant->propertyName() == "Quad atten")
                    variant->setValue(lightComponent->quadraAttenAttr_.Get());
            }

            subProperties = propertyMap_["Angle"]->subProperties();
            for(uint i = 0; i < subProperties.size(); i++)
            {
                QtVariantProperty *variant = dynamic_cast<QtVariantProperty *>(subProperties[i]);
                if(variant->propertyName() == "Inner angle")
                    variant->setValue(lightComponent->innerAngleAttr_.Get());
                else if(variant->propertyName() == "Outer angle")
                    variant->setValue(lightComponent->outerAngleAttr_.Get());
            }
        }
    }

    void ECLightEditor::Initialize()
    {
        if(!componentInterface_.expired())
        {
            Foundation::ComponentInterfacePtr componentPtr = componentInterface_.lock();
            QObject::connect(componentPtr.get(), SIGNAL(OnChanged()), this, SLOT(UpdateProperties()));
        }

        rootProperty_ = variantManager_->addProperty(QtVariantPropertyManager::groupTypeId(), QString("ECLight"));
        //Create light type property option.
        QtVariantProperty *lightTypeItem = variantManager_->addProperty(QtVariantPropertyManager::enumTypeId(), QString("Light type"));
        if(lightTypeItem)
        {
            QStringList enumNames;
            enumNames << "Point" << "Spot" << "Directional";
            lightTypeItem->setAttribute(QString("enumNames"), enumNames);
            propertyMap_["Light type"] = lightTypeItem;
        }

        //Create light direction option.
        QtVariantProperty *directionItem = variantManager_->addProperty(QtVariantPropertyManager::groupTypeId(), QString("Direction"));
        if(directionItem)
        {
            QtVariantProperty *x_item = variantManager_->addProperty(QVariant::Double, QString("x"));
            directionItem->addSubProperty(x_item);
            
            QtVariantProperty *y_item = variantManager_->addProperty(QVariant::Double, QString("y"));
            directionItem->addSubProperty(y_item);
            
            QtVariantProperty *z_item = variantManager_->addProperty(QVariant::Double, QString("z"));
            directionItem->addSubProperty(z_item);

            propertyMap_["Direction"] = directionItem;
        }

        QtVariantProperty *colorItem = variantManager_->addProperty(QVariant::Color, "diffuse color");
        if(colorItem)
        {
            propertyMap_["diffuse color"] = colorItem;
            colorItem = 0;
        }
        
        colorItem = variantManager_->addProperty(QVariant::Color, "specular color");
        if(colorItem)
        {
            propertyMap_["specular color"] = colorItem;
        }

        QtVariantProperty *castShadowsItem = variantManager_->addProperty(QVariant::Bool, "Cast shadows");
        if(castShadowsItem)
        {
            propertyMap_["Cast shadows"] = castShadowsItem;
        }

        QtVariantProperty *rangeItem = variantManager_->addProperty(QVariant::Double, "Range");
        if(rangeItem)
        {
            propertyMap_["Range"] = rangeItem;
        }

        QtVariantProperty *attenuationItem = variantManager_->addProperty(QtVariantPropertyManager::groupTypeId(), QString("Attenuation"));
        if(attenuationItem)
        {
            QtVariantProperty *c_item = variantManager_->addProperty(QVariant::Double, QString("Const atten"));
            attenuationItem->addSubProperty(c_item);
            
            QtVariantProperty *l_item = variantManager_->addProperty(QVariant::Double, QString("Linear atten"));
            attenuationItem->addSubProperty(l_item);
            
            QtVariantProperty *q_item = variantManager_->addProperty(QVariant::Double, QString("Quad atten"));
            attenuationItem->addSubProperty(q_item);

            propertyMap_["Attenuation"] = attenuationItem;
        }

        QtVariantProperty *AngleItem = variantManager_->addProperty(QtVariantPropertyManager::groupTypeId(), QString("Angle"));
        if(AngleItem)
        {
            QtVariantProperty *i_item = variantManager_->addProperty(QVariant::Double, QString("Inner angle"));
            AngleItem->addSubProperty(i_item);
            
            QtVariantProperty *o_item = variantManager_->addProperty(QVariant::Double, QString("Outer angle"));
            AngleItem->addSubProperty(o_item);

            propertyMap_["Angle"] = AngleItem;
        }

        VariantPropertyMap::iterator iter = propertyMap_.begin();
        while(iter != propertyMap_.end())
        {
            rootProperty_->addSubProperty((*iter));
            iter++;
        }
        UpdateProperties();
    }

    void ECLightEditor::SetValue(QtProperty *property)
    {
        if(componentInterface_.expired() && variantManager_ && propertyMap_.empty())
            return;

        Foundation::ComponentInterfacePtr componentPtr = componentInterface_.lock();
        EC_Light *lightComponent = dynamic_cast<EC_Light*>(componentPtr.get());
        if(lightComponent)
        {
            QVariant variant = propertyMap_[property->propertyName()]->value();//variantManager_->property(property->propertyName().toStdString().c_str());
            QString propertyName = property->propertyName();
            if(propertyName == "Light type")
            {
                switch(variant.toInt())
                {
                case EC_Light::LT_Point:
                    lightComponent->typeAttr_.Set(EC_Light::LT_Point, Foundation::Local);
                    break;
                case EC_Light::LT_Spot:
                    lightComponent->typeAttr_.Set(EC_Light::LT_Spot, Foundation::Local);
                    break;
                case EC_Light::LT_Directional:
                    lightComponent->typeAttr_.Set(EC_Light::LT_Directional, Foundation::Local);
                    break;
                }
                
            }
            else if(property->propertyName() == "x")
                lightComponent->directionAttr_.Set(Vector3df(variant.toFloat(), lightComponent->directionAttr_.Get().y, lightComponent->directionAttr_.Get().z), Foundation::Local);
            else if(property->propertyName() == "y")
                lightComponent->directionAttr_.Set(Vector3df(lightComponent->directionAttr_.Get().x, variant.toFloat(), lightComponent->directionAttr_.Get().z), Foundation::Local);
            else if(property->propertyName() == "z")
                lightComponent->directionAttr_.Set(Vector3df(lightComponent->directionAttr_.Get().x, lightComponent->directionAttr_.Get().y, variant.toFloat()), Foundation::Local);
            else if(property->propertyName() == "diffuse color")
            {
                QColor color = variant.value<QColor>();
                lightComponent->diffColorAttr_.Set(Color(color.red(), color.green(), color.blue()), Foundation::Local);
            }
            else if(property->propertyName() == "specular color")
            {
                QColor color = variant.value<QColor>();
                lightComponent->specColorAttr_.Set(Color(color.red(), color.green(), color.blue()), Foundation::Local);
            }
            else if(property->propertyName() == "Cast shadows")
                lightComponent->castShadowsAttr_.Set(variant.toBool(), Foundation::Local);
            else if(property->propertyName() == "Range")
                lightComponent->castShadowsAttr_.Set(variant.toFloat(), Foundation::Local);
            else if(property->propertyName() == "Const atten")
                lightComponent->constAttenAttr_.Set(variant.toFloat(), Foundation::Local);
            else if(property->propertyName() == "Linear atten")
                lightComponent->linearAttenAttr_.Set(variant.toFloat(), Foundation::Local);
            else if(property->propertyName() == "Quad atten")
                lightComponent->quadraAttenAttr_.Set(variant.toFloat(), Foundation::Local);
            else if(property->propertyName() == "Inner angle")
                lightComponent->innerAngleAttr_.Set(variant.toFloat(), Foundation::Local);
            else if(property->propertyName() == "Outer angle")
                lightComponent->outerAngleAttr_.Set(variant.toFloat(), Foundation::Local);

            if(!componentInterface_.expired())
            {
                Foundation::ComponentPtr component = componentInterface_.lock();
                if(component.get())
                {
                    Scene::Entity *entity = component.get()->GetParentEntity();
                    if(entity)
                    {
                        Scene::Events::SceneEventData event_data(entity->GetId());
                        Foundation::EventManagerPtr event_manager = entity->GetFramework()->GetEventManager();
                        event_manager->SendEvent(event_manager->QueryEventCategory("Scene"), Scene::Events::EVENT_ENTITY_ECS_MODIFIED, &event_data);
                    }
                }
            }
        }
    }
}