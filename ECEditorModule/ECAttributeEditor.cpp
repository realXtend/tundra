// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECAttributeEditor.h"
#include "ECComponentEditor.h"
#include "AttributeInterface.h"

// QtPropertyBrowser headers.
#include "qtvariantproperty.h"
#include "qtpropertymanager.h"
#include "qtpropertybrowser.h"
#include "qteditorfactory.h"

// Own QtPropertyBrowser headers.
#include "MultiEditPropertyManager.h"
#include "MultiEditPropertyFactory.h"

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    // static
    ECAttributeEditorBase *ECAttributeEditorBase::GetAttributeEditor(QtAbstractPropertyBrowser *browser, 
                                                                     ECComponentEditor *editor, 
                                                                     const Foundation::AttributeInterface &attribute, 
                                                                     Foundation::ComponentInterfacePtr component)
    {
        ECAttributeEditorBase *attributeEditor = 0;
        if(dynamic_cast<const Foundation::Attribute<Real> *>(&attribute))
            attributeEditor = new ECAttributeEditor<Real>(attribute.GetName(), browser, component, editor);
        else if(dynamic_cast<const Foundation::Attribute<Color> *>(&attribute))
            attributeEditor = new ECAttributeEditor<Color>(attribute.GetName(), browser, component, editor);

        if(attributeEditor)
            QObject::connect(attributeEditor, SIGNAL(AttributeChanged()), editor, SIGNAL(AttributeChanged()));
        return attributeEditor;
    }

    ECAttributeEditorBase::ECAttributeEditorBase(const QString &attributeName,
            QtAbstractPropertyBrowser *owner,
            Foundation::ComponentPtr component,
            QObject *parent):
        QObject(parent),
        owner_(owner),
        attributeName_(attributeName),
        rootProperty_(0),
        parentProperty_(0),
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

    ECAttributeEditorBase::ECAttributeEditorBase(const QString &attributeName,
           QtAbstractPropertyBrowser *owner,
           std::vector<Foundation::ComponentPtr> components,
           QObject *parent):
        QObject(parent),
        owner_(owner),
        attributeName_(attributeName),
        rootProperty_(0),
        parentProperty_(0),
        factory_(0),
        propertyMgr_(0),
        listenEditorChangedSignal_(false),
        useMultiEditor_(false),
        componentIsSerializable_(false)
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

    ECAttributeEditorBase::~ECAttributeEditorBase()
    {
        UninitializeEditor();
    }

    void ECAttributeEditorBase::AddNewComponent(Foundation::ComponentPtr component)
    {
        assert(component.get());
        Foundation::AttributeInterface *attribute = FindAttribute(component->GetAttributes());
        componentIsSerializable_ = component->IsSerializable();
        if(attribute)
        {
            attributeMap_[component] = attribute;
            QObject::connect(component.get(), SIGNAL(OnChanged()), this, SLOT(AttributeValueChanged()));
            listenEditorChangedSignal_ = true;
        }
    }

    void ECAttributeEditorBase::AddNewComponents(std::vector<Foundation::ComponentPtr>  components)
    {
        for(uint i = 0; i < components.size(); i++)
            AddNewComponent(components[i]);
    }

    void ECAttributeEditorBase::UpdateEditorUI()
    {
        if(attributeMap_.size() >= 1) // We need to check if multiple components has the same value.
                                      // If not start using multiedit and recreate the attributes ui.
        {
            if(!AttributesValueCheck())
                useMultiEditor_ = true;
            UninitializeEditor();
            InitializeEditor();
        }
    }

    Foundation::AttributeInterface *ECAttributeEditorBase::FindAttribute(Foundation::AttributeVector attributes)
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

    bool ECAttributeEditorBase::AttributesValueCheck() const
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

    void ECAttributeEditorBase::PreInitializeEditor()
    {
        if(propertyMgr_ || factory_ || rootProperty_)
            UninitializeEditor();
    }

    void ECAttributeEditorBase::UninitializeEditor()
    {
        if(owner_)
            owner_->unsetFactoryForManager(propertyMgr_);
        if(propertyMgr_)
        {
            propertyMgr_->deleteLater();
            propertyMgr_ = 0;
        }
        if(factory_)
        {
            factory_->deleteLater();
            factory_ = 0;
        }
    }

    template<typename T> void ECAttributeEditor<T>::InitializeMultiEditor()
    {
        ECAttributeEditorBase::PreInitializeEditor();
        if(useMultiEditor_)
        {
            MultiEditPropertyManager *multiEditManager = new MultiEditPropertyManager(this);
            MultiEditPropertyFact *multiEditFactory = new MultiEditPropertyFact(this);
            propertyMgr_ = multiEditManager;
            factory_ = multiEditFactory;

            rootProperty_ = multiEditManager->addProperty(attributeName_);
            owner_->setFactoryForManager(multiEditManager, multiEditFactory);
            UpdateMultiEditorValue();
            QObject::connect(multiEditManager, SIGNAL(ValueChanged(const QtProperty *, const QString &)), this, SLOT(MultiSelectValueSelected(const QtProperty *, const QString &)));
            if(parentProperty_)
                parentProperty_->addSubProperty(rootProperty_);
        }
    }

    template<typename T> void ECAttributeEditor<T>::UpdateMultiEditorValue()
    {
        if(useMultiEditor_ && componentIsSerializable_)
        {
            ECAttributeMap::iterator iter = attributeMap_.begin();
            QStringList stringList;
            MultiEditPropertyManager *testPropertyManager = dynamic_cast<MultiEditPropertyManager *>(propertyMgr_);
            while(iter != attributeMap_.end())
            {
                if(rootProperty_ && iter->second)
                {
                    Foundation::Attribute<T> *attribute = dynamic_cast<Foundation::Attribute<T>*>(iter->second);
                    stringList << QString(attribute->ToString().c_str());
                }
                iter++;
            }
            testPropertyManager->SetAttributeValues(rootProperty_, stringList);
        }
    }

    template<typename T> void ECAttributeEditor<T>::ValueSelected(const QtProperty *property, const QString &value)
    {
        if(useMultiEditor_)
        {
            if(rootProperty_ == property)
            {
                T newValue = StringToValue(value);
                useMultiEditor_ = false;
                UpdateEditorUI();
                SetValue(newValue);
            }
        }
    }

    template<> void ECAttributeEditor<Real>::InitializeEditor()
    {
        ECAttributeEditorBase::PreInitializeEditor();
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *realPropertyManager = new QtVariantPropertyManager(this);
            QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
            propertyMgr_ = realPropertyManager;
            factory_ = variantFactory;

            rootProperty_ = realPropertyManager->addProperty(QVariant::Double, attributeName_);
            if(rootProperty_)
            {
                UpdateEditorValue();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
            }
            owner_->setFactoryForManager(realPropertyManager, variantFactory);
            //owner_->addProperty(property_);
            QtVariantProperty *property = dynamic_cast<QtVariantProperty *>(rootProperty_);
            property->setAttribute("decimals", 2);
            if(parentProperty_)
                parentProperty_->addSubProperty(rootProperty_);
        }
        else
        {
            InitializeMultiEditor();
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
                if(rootProperty_ && iter->second)
                {
                    Foundation::Attribute<Real> *attribute = dynamic_cast<Foundation::Attribute<Real>*>(iter->second);
                    realPropertyManager->setValue(rootProperty_, attribute->Get());
                }
                iter++;
            }
        }
    }

    template<> void ECAttributeEditor<Real>::ValueSelected(const QtProperty *property, const QString &value)
    {
        if(useMultiEditor_)
        {
            if(rootProperty_ == property)
            {
                Real newValue = StringToValue(value);
                UninitializeEditor();
                useMultiEditor_ = false;
                InitializeEditor();
                SetValue(newValue);
            }
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
    }*/


    template<> void ECAttributeEditor<Color>::UpdateEditorValue()
    {
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            if(rootProperty_)
            {
                QList<QtProperty *> children = rootProperty_->subProperties();
                if(children.size() >= 4)
                {
                    if(attributeMap_.size() <= 0)
                    return;

                    Foundation::Attribute<Color> *attribute = dynamic_cast<Foundation::Attribute<Color> *>(attributeMap_.begin()->second);
                    if(!attribute)
                        return;

                    Color colorValue = attribute->Get();
                    variantManager->setValue(children[0], colorValue.r * 255);
                    variantManager->setValue(children[1], colorValue.g * 255);
                    variantManager->setValue(children[2], colorValue.b * 255);
                    variantManager->setValue(children[3], colorValue.a * 255);
                }
            }
        }
    }

    /*template<> void ECAttributeEditor<std::string>::UpdateEditorValue()
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
    }*/

    template<> void ECAttributeEditor<Color>::InitializeEditor()
    {
        ECAttributeEditorBase::PreInitializeEditor();
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *variantManager = new QtVariantPropertyManager(this);
            QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
            propertyMgr_ = variantManager;
            factory_ = variantFactory;

            rootProperty_ = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), attributeName_);
            if(rootProperty_)
            {
                QtVariantProperty *childProperty = 0;
                childProperty = variantManager->addProperty(QVariant::Int, "Red");
                rootProperty_->addSubProperty(childProperty);

                childProperty = variantManager->addProperty(QVariant::Int, "Green");
                rootProperty_->addSubProperty(childProperty);

                childProperty = variantManager->addProperty(QVariant::Int, "Blue");
                rootProperty_->addSubProperty(childProperty);

                childProperty = variantManager->addProperty(QVariant::Int, "Apha");
                rootProperty_->addSubProperty(childProperty);
                UpdateEditorValue();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
            }
            owner_->setFactoryForManager(variantManager, variantFactory);
            //owner_->addProperty(rootProperty_);
            if(parentProperty_)
                parentProperty_->addSubProperty(rootProperty_);
        }
        else
        {
            InitializeMultiEditor();
        }
    }

    template<> void ECAttributeEditor<Color>::SendNewValueToAttribute(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            QList<QtProperty *> children = rootProperty_->subProperties();
            if(children.size() >= 4)
            {
                if(attributeMap_.size() <= 0)
                    return;

                Foundation::Attribute<Color> *attribute = dynamic_cast<Foundation::Attribute<Color> *>(attributeMap_.begin()->second);
                if(!attribute)
                    return;

                Color newValue = attribute->Get();
                QString propertyName = property->propertyName();
                if(propertyName == "Red")
                    newValue.r = ParseString<int>(property->valueText().toStdString()) / 255.0f;
                else if(propertyName == "Green")
                    newValue.g = ParseString<int>(property->valueText().toStdString()) / 255.0f;
                else if(propertyName == "Blue")
                    newValue.b = ParseString<int>(property->valueText().toStdString()) / 255.0f;
                else if(propertyName == "Apha")
                    newValue.a = ParseString<int>(property->valueText().toStdString()) / 255.0f;
                SetValue(newValue);
            }
        }
    }

    template<> void ECAttributeEditor<Color>::ValueSelected(const QtProperty *property, const QString &value)
    {
        if(useMultiEditor_)
        {
            if(rootProperty_ == property)
            {
                StringVector values = SplitString(value.toStdString(), ' ');
                if(values.size() != 4)
                    return;

                Real color[4];
                for(uint i = 0; i < 4; i++)
                    color[i] = ParseString<Real>(values[i]);
                Color newValue = Color(color[0], color[1], color[2], color[3]);

                useMultiEditor_ = false;
                UpdateEditorUI();
                SetValue(newValue);
            }
        }
    }

    /*template<> void ECAttributeEditor<std::string>::InitializeEditor()
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
