// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECEditorModule.h"
#include "ECAttributeEditor.h"

#include "AttributeInterface.h"
#include "ComponentInterface.h"

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
    ECAttributeEditorBase::ECAttributeEditorBase(const QString &attributeName,
            QtAbstractPropertyBrowser *owner,
            Foundation::ComponentPtr component,
            QObject *parent):
        QObject(parent),
        owner_(owner),
        attributeName_(attributeName),
        rootProperty_(0),
        factory_(0),
        propertyMgr_(0),
        listenEditorChangedSignal_(false),
        useMultiEditor_(false),
        isInitialized_(false)
    {
        assert(component.get());
        Foundation::AttributeInterface *attribute = FindAttribute(component->GetAttributes());
        if(attribute)
        {
            attributeMap_[Foundation::ComponentWeakPtr(component)] = attribute;
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
        factory_(0),
        propertyMgr_(0),
        listenEditorChangedSignal_(false),
        useMultiEditor_(false),
        componentIsSerializable_(false),
        isInitialized_(false)
    {
        for(uint i = 0; i < components.size(); i++)
        {
            Foundation::ComponentPtr component = components[i];
            assert(component.get());
            Foundation::AttributeInterface *attribute = FindAttribute(component->GetAttributes());
            if(attribute)
            {
                attributeMap_[Foundation::ComponentWeakPtr(component)] = attribute;
                QObject::connect(component.get(), SIGNAL(OnChanged()), this, SLOT(AttributeValueChanged()));
            }
        }
    }

    ECAttributeEditorBase::~ECAttributeEditorBase()
    {
        UninitializeEditor();
    }

    bool ECAttributeEditorBase::ContainProperty(QtProperty *property) const
    {
        QSet<QtProperty *> properties = propertyMgr_->properties();
        QSet<QtProperty *>::const_iterator iter = properties.find(property);//const_cast<QtProperty*>(property));
        if(iter != properties.end())
            return true;
        return false;
    }

    void ECAttributeEditorBase::AddNewComponents(std::vector<Foundation::ComponentPtr>  components)
    {
        for(uint i = 0; i < components.size(); i++)
            AddNewComponent(components[i]);
    }

    void ECAttributeEditorBase::UpdateEditorUI()
    {
        if(attributeMap_.size() == 1)
        {
            useMultiEditor_ = false;
            InitializeEditor();
            emit AttributeChanged(attributeName_.toStdString());
        }
        else if(attributeMap_.size() > 1)
        {
            if(!AttributesValueCheck())
            {
                if(!useMultiEditor_)
                {
                    useMultiEditor_ = true;
                    UninitializeEditor();
                }
            }
            else
            {
                if(useMultiEditor_)
                {
                    useMultiEditor_ = false;
                    UninitializeEditor();
                }
            }

            if(!isInitialized_)
                InitializeEditor();
            else
                UpdateEditorValue();
            emit AttributeChanged(attributeName_.toStdString());
        }
    }

    void ECAttributeEditorBase::AddNewComponent(Foundation::ComponentPtr component)
    {
        assert(component.get());
        Foundation::AttributeInterface *attribute = FindAttribute(component->GetAttributes());
        componentIsSerializable_ = component->IsSerializable();
        if(attribute)
        {
            attributeMap_[Foundation::ComponentWeakPtr(component)] = attribute;
            listenEditorChangedSignal_ = true;
            QObject::connect(component.get(), SIGNAL(OnChanged()), this, SLOT(AttributeValueChanged()));
            //UpdateEditorUI();
        }
    }

    void ECAttributeEditorBase::RemoveComponentFromEditor(Foundation::ComponentInterface *component)
    {
        ECAttributeMap::iterator iter = attributeMap_.begin();
        while(iter != attributeMap_.end())
        {
            Foundation::ComponentWeakPtr compWeakPtr = iter->first;
            if(compWeakPtr.expired())
                continue;
            Foundation::ComponentPtr componentPtr = compWeakPtr.lock();
            assert(componentPtr.get());
            Foundation::ComponentInterface *compInterface = componentPtr.get();
            if(compInterface == component)
            {
                attributeMap_.erase(iter);
                UpdateEditorUI();
                break;
            }
            iter++;
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
        isInitialized_ = false;
    }

    //-------------------------REAL ATTRIBUTE TYPE-------------------------

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
        }
        else
        {
            InitializeMultiEditor();
        }
        isInitialized_ = true;
        emit AttributeEditorCreated(rootProperty_);
    }


    template<> void ECAttributeEditor<Real>::UpdateEditorValue()
    {
        if(!useMultiEditor_)
        {
            ECAttributeMap::iterator iter = attributeMap_.begin();
            QtVariantPropertyManager *realPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            assert(realPropertyManager);
            if(!realPropertyManager)
                return;

            if(iter != attributeMap_.end())
            {
                if(rootProperty_ && iter->second)
                {
                    Foundation::Attribute<Real> *attribute = dynamic_cast<Foundation::Attribute<Real>*>(iter->second);
                    realPropertyManager->setValue(rootProperty_, attribute->Get());
                }
            }
        }
        else
            UpdateMultiEditorValue();
    }

    template<> void ECAttributeEditor<Real>::SendNewValueToAttribute(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            Real newValue = ParseString<Real>(property->valueText().toStdString());
            SetValue(newValue);
        }
    }

    template<> void ECAttributeEditor<Real>::ValueSelected(const QtProperty *property, const QString &value)
    {
        if(useMultiEditor_)
        {
            if(rootProperty_ == property)
            {
                Real newValue;
                try
                {
                    newValue = ParseString<Real>(value.toStdString());//StringToValue(value);
                }
                catch (boost::bad_lexical_cast e)
                {
                    ECEditor::ECEditorModule::LogError(std::string(e.what()) + ". ECAttributeEditor cannot cast string value to real format.");
                    return;
                }
                UninitializeEditor();
                useMultiEditor_ = false;
                InitializeEditor();
                SetValue(newValue);
            }
        }
    }

    //-------------------------INT ATTRIBUTE TYPE-------------------------

    template<> void ECAttributeEditor<int>::UpdateEditorValue()
    {
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *intPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            ECAttributeMap::iterator iter = attributeMap_.begin();
            assert(intPropertyManager);
            if(!intPropertyManager)
                return;

            if(iter != attributeMap_.end())
            {
                if(rootProperty_ && iter->second)
                {
                    Foundation::Attribute<int> *attribute = dynamic_cast<Foundation::Attribute<int>*>(iter->second);
                    intPropertyManager->setValue(rootProperty_, attribute->Get());
                }
            }
        }
        else
            UpdateMultiEditorValue();
    }

    template<> void ECAttributeEditor<int>::InitializeEditor()
    {
        ECAttributeEditorBase::PreInitializeEditor();
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *intPropertyManager = new QtVariantPropertyManager(this);
            QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
            propertyMgr_ = intPropertyManager;
            factory_ = variantFactory;
            rootProperty_ = intPropertyManager->addProperty(QVariant::Int, attributeName_);
            if(rootProperty_)
            {
                UpdateEditorValue();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
            }
            owner_->setFactoryForManager(intPropertyManager, variantFactory);
        }
        else
        {
            InitializeMultiEditor();
        }
        isInitialized_ = true;
        emit AttributeEditorCreated(rootProperty_);
    }

    template<> void ECAttributeEditor<int>::ValueSelected(const QtProperty *property, const QString &value)
    {
        if(useMultiEditor_)
        {
            if(rootProperty_ == property)
            {
                int newValue;
                try
                {
                    newValue = ParseString<int>(value.toStdString());
                }
                catch (boost::bad_lexical_cast e)
                {
                    ECEditor::ECEditorModule::LogError(std::string(e.what()) + ". ECAttributeEditor cannot cast string value to real format.");
                    return;
                }
                UninitializeEditor();
                useMultiEditor_ = false;
                InitializeEditor();
                SetValue(newValue);
            }
        }
    }

    template<> void ECAttributeEditor<int>::SendNewValueToAttribute(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            Real newValue = ParseString<int>(property->valueText().toStdString());
            SetValue(newValue);
        }
    }

    //-------------------------BOOL ATTRIBUTE TYPE-------------------------

    template<> void ECAttributeEditor<bool>::InitializeEditor()
    {
        ECAttributeEditorBase::PreInitializeEditor();
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *boolPropertyManager = new QtVariantPropertyManager(this);
            QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
            propertyMgr_ = boolPropertyManager;
            factory_ = variantFactory;
            rootProperty_ = boolPropertyManager->addProperty(QVariant::Bool, attributeName_);
            if(rootProperty_)
            {
                UpdateEditorValue();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
            }
            owner_->setFactoryForManager(boolPropertyManager, variantFactory);
        }
        else
        {
            InitializeMultiEditor();
        }
        isInitialized_ = true;
        emit AttributeEditorCreated(rootProperty_);
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

    template<> void ECAttributeEditor<bool>::UpdateEditorValue()
    {
        if(!useMultiEditor_)
        {
            ECAttributeMap::iterator iter = attributeMap_.begin();
            QtVariantPropertyManager *boolPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            assert(boolPropertyManager);
            if(!boolPropertyManager)
                return;

            if(iter != attributeMap_.end())
            {
                if(rootProperty_ && iter->second)
                {
                    Foundation::Attribute<bool> *attribute = dynamic_cast<Foundation::Attribute<bool>*>(iter->second);
                    boolPropertyManager->setValue(rootProperty_, attribute->Get());
                }
            }
        }
        else
            UpdateMultiEditorValue();
    }

    template<> void ECAttributeEditor<bool>::ValueSelected(const QtProperty *property, const QString &value)
    {
        if(useMultiEditor_)
        {
            if(rootProperty_ == property)
            {
                if(value.size() < 4 || value.size() > 5)
                    return;

                bool newValue = false;
                if(value == "false" || value == "False" || value == "0")
                    newValue = false;
                else if(value == "true" || value == "True" || value == "1")
                    newValue = true;
                else
                    return;

                UninitializeEditor();
                useMultiEditor_ = false;
                InitializeEditor();
                SetValue(newValue);
            }
        }
    }

    //-------------------------VECTOR3DF ATTRIBUTE TYPE-------------------------

    template<> void ECAttributeEditor<Vector3df>::UpdateEditorValue()
    {
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            if(rootProperty_)
            {
                QList<QtProperty *> children = rootProperty_->subProperties();
                if(children.size() >= 3)
                {
                    if(attributeMap_.size() <= 0)
                        return;

                    Foundation::Attribute<Vector3df> *attribute = dynamic_cast<Foundation::Attribute<Vector3df> *>(attributeMap_.begin()->second);
                    if(!attribute)
                        return;

                    Vector3df vectorValue = attribute->Get();
                    variantManager->setValue(children[0], vectorValue.x);
                    variantManager->setValue(children[1], vectorValue.y);
                    variantManager->setValue(children[2], vectorValue.z);
                }
            }
        }
        else
            UpdateMultiEditorValue();
    }

    template<> void ECAttributeEditor<Vector3df>::InitializeEditor()
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
                QtProperty *childProperty = 0;
                childProperty = variantManager->addProperty(QVariant::Double, "x");
                rootProperty_->addSubProperty(childProperty);

                childProperty = variantManager->addProperty(QVariant::Double, "y");
                rootProperty_->addSubProperty(childProperty);

                childProperty = variantManager->addProperty(QVariant::Double, "z");
                rootProperty_->addSubProperty(childProperty);
                UpdateEditorValue();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
            }
            owner_->setFactoryForManager(variantManager, variantFactory);
        }
        else
        {
            InitializeMultiEditor();
        }
        isInitialized_ = true;
        emit AttributeEditorCreated(rootProperty_);
    }

    template<> void ECAttributeEditor<Vector3df>::SendNewValueToAttribute(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            QList<QtProperty *> children = rootProperty_->subProperties();
            if(children.size() >= 3)
            {
                if(attributeMap_.size() <= 0)
                return;

                Foundation::Attribute<Vector3df> *attribute = dynamic_cast<Foundation::Attribute<Vector3df> *>(attributeMap_.begin()->second);
                if(!attribute)
                    return;

                Vector3df newValue = attribute->Get();
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

    template<> void ECAttributeEditor<Vector3df>::ValueSelected(const QtProperty *property, const QString &value)
    {
        if(useMultiEditor_)
        {
            if(rootProperty_ == property)
            {
                StringVector values = SplitString(value.toStdString(), ' ');
                if(values.size() != 3)
                    return;

                Real vector[3];
                for(uint i = 0; i < 3; i++)
                {
                    try
                    {
                        vector[i] = ParseString<Real>(values[i]);
                    }
                    catch (boost::bad_lexical_cast e)
                    {
                        ECEditor::ECEditorModule::LogError(std::string(e.what()) + ". ECAttributeEditor cannot cast string value to real format.");
                        return;
                    }
                }
                Vector3df newValue = Vector3df(vector[0], vector[1], vector[2]);

                UninitializeEditor();
                useMultiEditor_ = false;
                InitializeEditor();
                SetValue(newValue);
            }
        }
    }

    //-------------------------COLOR ATTRIBUTE TYPE-------------------------

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
        else
            UpdateMultiEditorValue();
    }

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
                variantManager->setAttribute(childProperty, "minimum", QVariant(0));
                variantManager->setAttribute(childProperty, "maximum", QVariant(255));

                childProperty = variantManager->addProperty(QVariant::Int, "Green");
                rootProperty_->addSubProperty(childProperty);
                variantManager->setAttribute(childProperty, "minimum", QVariant(0));
                variantManager->setAttribute(childProperty, "maximum", QVariant(255));

                childProperty = variantManager->addProperty(QVariant::Int, "Blue");
                rootProperty_->addSubProperty(childProperty);
                variantManager->setAttribute(childProperty, "minimum", QVariant(0));
                variantManager->setAttribute(childProperty, "maximum", QVariant(255));

                childProperty = variantManager->addProperty(QVariant::Int, "Alpha");
                rootProperty_->addSubProperty(childProperty);
                variantManager->setAttribute(childProperty, "minimum", QVariant(0));
                variantManager->setAttribute(childProperty, "maximum", QVariant(255));

                UpdateEditorValue();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
            }
            owner_->setFactoryForManager(variantManager, variantFactory);
        }
        else
        {
            InitializeMultiEditor();
        }
        isInitialized_ = true;
        emit AttributeEditorCreated(rootProperty_);
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
                else if(propertyName == "Alpha")
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
                {
                    try
                    {
                        color[i] = ParseString<Real>(values[i]);
                    }
                    catch (boost::bad_lexical_cast e)
                    {
                        ECEditor::ECEditorModule::LogError(std::string(e.what()) + ". ECAttributeEditor cannot cast string value to real format.");
                        return;
                    }
                }
                Color newValue = Color(color[0], color[1], color[2], color[3]);

                UninitializeEditor();
                useMultiEditor_ = false;
                InitializeEditor();
                SetValue(newValue);
            }
        }
    }

    //-------------------------STD::STRING ATTRIBUTE TYPE-------------------------

    template<> void ECAttributeEditor<std::string>::InitializeEditor()
    {
        ECAttributeEditorBase::PreInitializeEditor();
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *qStringPropertyManager = new QtVariantPropertyManager(this);
            QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
            propertyMgr_ = qStringPropertyManager;
            factory_ = variantFactory;
            rootProperty_ = qStringPropertyManager->addProperty(QVariant::String, attributeName_);
            if(rootProperty_)
            {
                UpdateEditorValue();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
            }
            owner_->setFactoryForManager(qStringPropertyManager, variantFactory);
        }
        else
        {
            InitializeMultiEditor();
        }
        isInitialized_ = true;
        emit AttributeEditorCreated(rootProperty_);
    }

    template<> void ECAttributeEditor<std::string>::SendNewValueToAttribute(QtProperty *property)
    {
        if (listenEditorChangedSignal_)
            SetValue(property->valueText().toStdString());
    }

    template<> void ECAttributeEditor<std::string>::UpdateEditorValue()
    {
        if(!useMultiEditor_)
        {
            ECAttributeMap::iterator iter = attributeMap_.begin();
            QtVariantPropertyManager *qStringPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            assert(qStringPropertyManager);
            if(!qStringPropertyManager)
                return;
            
            if(iter != attributeMap_.end())
            {
                if (rootProperty_ && iter->second)
                {
                    Foundation::Attribute<std::string> *attribute = dynamic_cast<Foundation::Attribute<std::string>*>(iter->second);
                    qStringPropertyManager->setValue(rootProperty_, attribute->Get().c_str());
                }
                iter++;
            }
        }
        else
            UpdateMultiEditorValue();
    }

    template<> void ECAttributeEditor<std::string>::ValueSelected(const QtProperty *property, const QString &value)
    {
        if(useMultiEditor_)
        {
            if(rootProperty_ == property)
            {
                UninitializeEditor();
                useMultiEditor_ = false;
                InitializeEditor();
                SetValue(value.toStdString());
            }
        }
    }
}
