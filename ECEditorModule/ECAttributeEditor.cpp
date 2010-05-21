// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECAttributeEditor.h"
#include "AttributeInterface.h"

#include <QtDoublePropertyManager>
#include <QtDoubleSpinBoxFactory>
#include <QtVariantPropertyManager>
#include <QtBoolPropertyManager>
#include <QtCheckBoxFactory>
#include <QtProperty>

#include "MultiEditPropertyManager.h"
#include "MultiEditPropertyFactory.h"

#include "MemoryLeakCheck.h"

namespace ECEditor
{
ECAttributeEditorInterface::ECAttributeEditorInterface(const QString &attributeName,
                       QtAbstractPropertyBrowser *owner,
                       Foundation::ComponentPtr component,
                       QObject *parent):
        QObject(parent),
        owner_(owner),
        attributeName_(attributeName),
        property_(0),
        factory_(0),
        propertyMgr_(0),
        listenEditorChangedSignal_(false),
        useMultiEditor_(false)
    {
        assert(component.get());
        Foundation::AttributeInterface *attribute = FindAttribute(component->GetAttributes());
        if(attribute)
        {
            attributeMap_[component] = attribute;
            QObject::connect(component.get(), SIGNAL(OnChanged()), this, SLOT(AttributeValueChanged()));
        }
    }

    ECAttributeEditorInterface::ECAttributeEditorInterface(const QString &attributeName,
                       QtAbstractPropertyBrowser *owner,
                       std::vector<Foundation::ComponentPtr> components,
                       QObject *parent):
        QObject(parent),
        owner_(owner),
        attributeName_(attributeName),
        property_(0),
        factory_(0),
        propertyMgr_(0),
        listenEditorChangedSignal_(false),
        useMultiEditor_(false)
    {
        for(uint i = 0; i < components.size(); i++)
        {
            Foundation::ComponentPtr component = components[i];
            assert(component.get());
            Foundation::AttributeInterface *attribute = FindAttribute(component->GetAttributes());
            if(attribute)
            {
                attributeMap_[component] = attribute;
                QObject::connect(component.get(), SIGNAL(OnChanged()), this, SLOT(AttributeValueChanged()));
            }
        }
    }

    ECAttributeEditorInterface::~ECAttributeEditorInterface()
    {
        UninitializeEditor();
    }

    void ECAttributeEditorInterface::AddNewComponent(Foundation::ComponentPtr component)
    {
        assert(component.get());
        Foundation::AttributeInterface *attribute = FindAttribute(component->GetAttributes());
        if(attribute)
        {
            attributeMap_[component] = attribute;
            QObject::connect(component.get(), SIGNAL(OnChanged()), this, SLOT(AttributeValueChanged()));
            listenEditorChangedSignal_ = true;
        }
        if(attributeMap_.size() > 1 && !useMultiEditor_) //We need to check if multiple attributes has a same value and 
                                                         //if not recreate new editor windows that contain drop list for each attribute values.
        {
            if(!AttributesValueCheck())
            {
                useMultiEditor_ = true;
                UninitializeEditor();
                InitializeEditor();
            }
        }
    }

    Foundation::AttributeInterface *ECAttributeEditorInterface::FindAttribute(Foundation::AttributeVector attributes)
    {
        Foundation::AttributeInterface *attribute = 0;
        for(uint i = 0; i < attributes.size(); i++)
        {
            if(attributes[i]->GetName() == attributeName_)
            {
                attribute = attributes[i];
                break;
            }
        }
        return attribute;
    }

    bool ECAttributeEditorInterface::AttributesValueCheck() const
    {
        if(attributeMap_.size() > 1)
        {
            ECAttributeMap::const_iterator iter = attributeMap_.begin();
            std::string value = iter->second->ToString();
            while(iter != attributeMap_.end())
            {
                if(value != iter->second->ToString())
                    return false;
                iter++;
            }
        }
        return true;
    }

    void ECAttributeEditorInterface::UninitializeEditor()
    {
        if(owner_)
            owner_->unsetFactoryForManager(propertyMgr_);

        SAFE_DELETE(property_)
        SAFE_DELETE(propertyMgr_)
        SAFE_DELETE(factory_)
    }

    template<> void ECAttributeEditor<Real>::InitializeEditor()
    {
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *realPropertyManager = new QtVariantPropertyManager();
            QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory();
            propertyMgr_ = realPropertyManager;
            factory_ = variantFactory;

            property_ = realPropertyManager->addProperty(QVariant::Double, attributeName_);
            if(property_)
            {
                UpdateEditorValue();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
            }
            owner_->setFactoryForManager(realPropertyManager, variantFactory);
            owner_->addProperty(property_);
            QtVariantProperty *property = dynamic_cast<QtVariantProperty *>(property_);
            property->setAttribute("decimals", 2);
        }
        else
        {
            MultiEditPropertyManager *testManager = new MultiEditPropertyManager();
            MultiEditPropertyFact *testFactory = new MultiEditPropertyFact();
            propertyMgr_ = testManager;
            factory_ = testFactory;

            property_ = testManager->addProperty(attributeName_);
            owner_->setFactoryForManager(testManager, testFactory);
            owner_->addProperty(property_);
            UpdateEditorValue();
        }
    }


    template<> void ECAttributeEditor<Real>::UpdateEditorValue()
    {
        if(!useMultiEditor_)
        {
            ECAttributeMap::iterator iter = attributeMap_.begin();
            while(iter != attributeMap_.end())
            {
                QtVariantPropertyManager *realPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
                if(property_ && iter->second)
                {
                    Foundation::Attribute<Real> *attribute = dynamic_cast<Foundation::Attribute<Real>*>(iter->second);
                    realPropertyManager->setValue(property_, attribute->Get());
                }
                iter++;
            }
        }
        else
        {
            ECAttributeMap::iterator iter = attributeMap_.begin();
            QStringList stringList;
            MultiEditPropertyManager *testPropertyManager = dynamic_cast<MultiEditPropertyManager *>(propertyMgr_);
            while(iter != attributeMap_.end())
            {
                if(property_ && iter->second)
                {
                    Foundation::Attribute<Real> *attribute = dynamic_cast<Foundation::Attribute<Real>*>(iter->second);
                    stringList << QString(attribute->ToString().c_str());
                }
                iter++;
            }
            testPropertyManager->SetAttributeValues(property_, stringList);
        }
    }

    /*template<> void ECAttributeEditor<bool>::UpdateEditorValue()
    {
        QtVariantPropertyManager *boolPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(property_ && attribute_)
        {
            boolPropertyManager->setValue(property_, attribute_->Get());
        }
    }

    template<> void ECAttributeEditor<int>::UpdateEditorValue()
    {
        QtVariantPropertyManager *intPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(property_ && attribute_)
        {
            intPropertyManager->setValue(property_, attribute_->Get());
        }
    }

    template<> void ECAttributeEditor<Vector3df>::UpdateEditorValue()
    {
        QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(property_ && attribute_)
        {
            QList<QtProperty *> children = property_->subProperties();
            if(children.size() >= 3)
            {
                Vector3df vectorValue = GetValue();
                variantManager->setValue(children[0], vectorValue.x);
                variantManager->setValue(children[1], vectorValue.y);
                variantManager->setValue(children[2], vectorValue.z);
            }
        }
    }

    template<> void ECAttributeEditor<Color>::UpdateEditorValue()
    {
        QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(property_ && attribute_)
        {
            QList<QtProperty *> children = property_->subProperties();
            if(children.size() >= 4)
            {
                Color colorValue = GetValue();
                variantManager->setValue(children[0], colorValue.r * 255);
                variantManager->setValue(children[1], colorValue.g * 255);
                variantManager->setValue(children[2], colorValue.b * 255);
                variantManager->setValue(children[3], colorValue.a * 255);
            }
        }
    }

    template<> void ECAttributeEditor<std::string>::UpdateEditorValue()
    {
        QtVariantPropertyManager *qStringPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if (property_ && attribute_)
            qStringPropertyManager->setValue(property_, attribute_->Get().c_str());
    }*/

    template<> void ECAttributeEditor<Real>::SendNewValueToAttribute(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            Real newValue = ParseString<Real>(property->valueText().toStdString());
            SetValue(newValue);
        }
    }

    /*template<> void ECAttributeEditor<bool>::InitializeEditor()
    {
        QtVariantPropertyManager *boolPropertyManager = new QtVariantPropertyManager();
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory();
        propertyMgr_ = boolPropertyManager;
        factory_ = variantFactory;
        property_ = boolPropertyManager->addProperty(QVariant::Bool, attributeName_);
        if(property_)
        {
            UpdateEditorValue();
            QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
        }
        owner_->setFactoryForManager(boolPropertyManager, variantFactory);
        owner_->addProperty(property_);
    }

    template<> void ECAttributeEditor<bool>::SendNewValueToAttribute(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            if(property->valueText().toStdString() == "True")
                SetValue(true);
            else
                SetValue(false);
        }
    }

    template<> void ECAttributeEditor<int>::InitializeEditor()
    {
        QtVariantPropertyManager *intPropertyManager = new QtVariantPropertyManager();//dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory();
        propertyMgr_ = intPropertyManager;
        factory_ = variantFactory;
        property_ = intPropertyManager->addProperty(QVariant::Int, attributeName_);
        if(property_)
        {
            UpdateEditorValue();
            QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
        }
        owner_->setFactoryForManager(intPropertyManager, variantFactory);
        owner_->addProperty(property_);
    }


    template<> void ECAttributeEditor<int>::SendNewValueToAttribute(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            Real newValue = ParseString<int>(property->valueText().toStdString());
            SetValue(newValue);
        }
    }

    template<> void ECAttributeEditor<Vector3df>::InitializeEditor()
    {
        QtVariantPropertyManager *variantManager = new QtVariantPropertyManager();//dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory();
        propertyMgr_ = variantManager;
        factory_ = variantFactory;
        property_ = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), attributeName_);
        if(property_)
        {
            QtProperty *childProperty = 0;
            childProperty = variantManager->addProperty(QVariant::Double, "x");
            property_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "y");
            property_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "z");
            property_->addSubProperty(childProperty);
            UpdateEditorValue();
            QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
        }
        owner_->setFactoryForManager(variantManager, variantFactory);
        owner_->addProperty(property_);
    }

    template<> void ECAttributeEditor<Vector3df>::SendNewValueToAttribute(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            QList<QtProperty *> children = property_->subProperties();
            if(children.size() >= 3 && attribute_)
            {
                Vector3df newValue = attribute_->Get();
                QString propertyName = property->propertyName();
                if(propertyName == "x")
                    newValue.x = ParseString<Real>(property->valueText().toStdString());
                else if(propertyName == "y")
                    newValue.y = ParseString<Real>(property->valueText().toStdString());
                else if(propertyName == "z")
                    newValue.z = ParseString<Real>(property->valueText().toStdString());
                SetValue(newValue);
            }
        }
    }

    template<> void ECAttributeEditor<Color>::InitializeEditor()
    {
        QtVariantPropertyManager *variantManager = new QtVariantPropertyManager();
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory();
        propertyMgr_ = variantManager;
        factory_ = variantFactory;
        property_ = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), attributeName_);
        if(property_)
        {
            QtVariantProperty *childProperty = 0;
            childProperty = variantManager->addProperty(QVariant::Int, "Red");
            property_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Int, "Green");
            property_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Int, "Blue");
            property_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Int, "Alfa");
            property_->addSubProperty(childProperty);
            UpdateEditorValue();
            QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
        }
        owner_->setFactoryForManager(variantManager, variantFactory);
        owner_->addProperty(property_);
    }

    template<> void ECAttributeEditor<Color>::SendNewValueToAttribute(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            QList<QtProperty *> children = property_->subProperties();
            if(children.size() >= 4 && attribute_)
            {
                Color newValue = attribute_->Get();
                QString propertyName = property->propertyName();
                if(propertyName == "Red")
                    newValue.r = ParseString<int>(property->valueText().toStdString()) / 255.0f;
                else if(propertyName == "Green")
                    newValue.g = ParseString<int>(property->valueText().toStdString()) / 255.0f;
                else if(propertyName == "Blue")
                    newValue.b = ParseString<int>(property->valueText().toStdString()) / 255.0f;
                else if(propertyName == "Alfa")
                    newValue.a = ParseString<int>(property->valueText().toStdString()) / 255.0f;
                SetValue(newValue);
            }
        }
    }

    template<> void ECAttributeEditor<std::string>::InitializeEditor()
    {
        QtVariantPropertyManager *qStringPropertyManager = new QtVariantPropertyManager();
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory();
        propertyMgr_ = qStringPropertyManager;
        factory_ = variantFactory;
        property_ = qStringPropertyManager->addProperty(QVariant::String, attributeName_);
        if(property_)
        {
            UpdateEditorValue();
            QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
        }
        owner_->setFactoryForManager(qStringPropertyManager, variantFactory);
        owner_->addProperty(property_);
    }

    template<> void ECAttributeEditor<std::string>::SendNewValueToAttribute(QtProperty *property)
    {
        if (listenEditorChangedSignal_)
            SetValue(property->valueText().toStdString());
    }*/
}
