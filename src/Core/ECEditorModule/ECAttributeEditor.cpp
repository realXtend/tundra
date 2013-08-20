// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECAttributeEditor.h"
#include "MultiEditPropertyManager.h"
#include "MultiEditPropertyFactory.h"
#include "LineEditPropertyFactory.h"
#include "EditorButtonFactory.h"
#include "Profiler.h"

#include "Math/float2.h"
#include "Math/float4.h"
#include "Math/Quat.h"
#include "float.h"

#include "IComponent.h"
#include "IAttribute.h"
#include "Transform.h"
#include "AssetReference.h"
#include "AssetsWindow.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "LoggingFunctions.h"

// QtPropertyBrowser headers.
#include <qtvariantproperty.h>
#include <qtpropertymanager.h>
#include <qtpropertybrowser.h>
#include <qteditorfactory.h>

#include "MemoryLeakCheck.h"

ECAttributeEditorBase::ECAttributeEditorBase(QtAbstractPropertyBrowser *owner, IAttribute *attribute, QObject *parent) :
    QObject(parent),
    owner_(owner),
    name_(attribute->Name()),
    typeName_(attribute->TypeName()),
    rootProperty_(0),
    factory_(0),
    propertyMgr_(0),
    listenEditorChangedSignal_(false),
    useMultiEditor_(false),
    metaDataFlag_(0)
{
    AddComponent(attribute->Owner()->shared_from_this());
}

ECAttributeEditorBase::~ECAttributeEditorBase()
{
    UnInitialize();
}

bool ECAttributeEditorBase::ContainsProperty(QtProperty *property) const
{
    QSet<QtProperty *> properties = propertyMgr_->properties();
    QSet<QtProperty *>::const_iterator iter = properties.find(property);
    if(iter != properties.end())
        return true;
    return false;
}

void ECAttributeEditorBase::UpdateEditorUI(IAttribute *attr)
{
    PROFILE(ECAttributeEditor_UpdateEditorUI);
    
    // Check if the attributes metadata has changed designable to false
    if (!useMultiEditor_ && !components_.empty())
    {
        ComponentPtr comp = components_[0].lock();
        IAttribute *attribute = FindAttribute(comp);
        if (attribute && attribute->Metadata() && !attribute->Metadata()->designable)
        {
            // If not uninitialize that attributes ui and return
            UnInitialize();
            return;
        }
    }

    // If attribute editor is holding only single component, "HasIdenticalAttributes" method should automaticly return true.
    bool identical_attr = HasIdenticalAttributes();
    if (!useMultiEditor_ && !identical_attr)
    {
        useMultiEditor_ = true;
        UnInitialize();
        Initialize();
    }
    else if(useMultiEditor_ && identical_attr)
    {
        useMultiEditor_ = false;
        UnInitialize();
        Initialize();
    }
    else
    {
        if (!propertyMgr_)
            Initialize();
        else
        {
            listenEditorChangedSignal_ = false;
            Update(attr);
            listenEditorChangedSignal_ = true;
        }
    }
}

void ECAttributeEditorBase::AddComponent(const ComponentPtr &component)
{
    PROFILE(ECAttributeEditor_AddComponent);
    // Before we add new component we make sure that it's not already added
    // and it contains right attribute.
    if(!HasComponent(component) && component->AttributeByName(name_))
    {
        components_.push_back(ComponentWeakPtr(component));
        connect(component.get(), SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), 
                this, SLOT(AttributeChanged(IAttribute*)),
                Qt::UniqueConnection);
        emit OnComponentAdded(rootProperty_, component.get());
    }
}

void ECAttributeEditorBase::RemoveComponent(const ComponentPtr &component)
{
    ComponentWeakPtrList::iterator iter = FindComponent(component);
    if(iter != components_.end())
    {
        emit OnComponentRemoved(rootProperty_, component.get());
        components_.erase(iter);
        disconnect(component.get(), SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), 
                   this, SLOT(AttributeChanged(IAttribute*)));
    }

    if (components_.isEmpty())
        deleteLater();
}

bool ECAttributeEditorBase::HasComponent(const ComponentPtr &component)
{
    PROFILE(ECAttributeEditor_HasComponent);
    ComponentWeakPtrList::iterator iter = FindComponent(component);
    if(iter != components_.end())
        return true;
    return false;
}

void ECAttributeEditorBase::AttributeChanged(IAttribute* attribute)
{
    if (listenEditorChangedSignal_)
    {
        // Ensure that attribute's name matchs with the editor's name variable.
        // If they doesn't match, no need to update the ui.
        if (attribute->Name() == this->name_)
            UpdateEditorUI(attribute);
    }
}

void ECAttributeEditorBase::MultiEditValueSelected(const QString &value) 
{
    ComponentWeakPtr comp;
    foreach(comp, components_)
        if(!comp.expired())
        {
            IAttribute *attribute = FindAttribute(comp.lock());
            if(attribute)
                attribute->FromString(value.toStdString(), AttributeChange::Default);
        }
}

IAttribute *ECAttributeEditorBase::FindAttribute(const ComponentPtr &component) const
{
    PROFILE(ECAttributeEditor_FindAttribute);
    if(component)
        return component->AttributeByName(name_);
    LogError("Component has expired.");
    return 0;
}

QList<ComponentWeakPtr>::iterator ECAttributeEditorBase::FindComponent(ComponentPtr component)
{
    ComponentWeakPtrList::iterator iter = components_.begin();
    for(; iter != components_.end(); ++iter)
        if(!(*iter).expired() && component.get() == (*iter).lock().get())
            return iter;
    return components_.end();
}

void ECAttributeEditorBase::CleanExpiredComponents()
{
    ComponentWeakPtrList::iterator iter = components_.begin();
    while(iter != components_.end())
    {
        if ((*iter).expired())
            iter = components_.erase(iter);
        else
            ++iter;
    }

    if(!components_.size())
        deleteLater();
}

void ECAttributeEditorBase::PreInitialize()
{
    if (propertyMgr_ || factory_ || rootProperty_)
        UnInitialize();
}

void ECAttributeEditorBase::UnInitialize()
{
    PROFILE(ECAttributeEditor_Uninitialize)
    if(owner_)
    {
        owner_->unsetFactoryForManager(propertyMgr_);
        for(uint i = 0; i < optionalPropertyManagers_.size(); ++i)
            owner_->unsetFactoryForManager(optionalPropertyManagers_[i]);
    }
    if(propertyMgr_)
    {
        propertyMgr_->deleteLater();
        propertyMgr_ = 0;
    }
    while(!optionalPropertyManagers_.empty())
    {
        optionalPropertyManagers_.back()->deleteLater();
        optionalPropertyManagers_.pop_back();
    }
    if(factory_)
    {
        factory_->deleteLater();
        factory_ = 0;
    }
    while(!optionalPropertyFactories_.empty())
    {
        optionalPropertyFactories_.back()->deleteLater();
        optionalPropertyFactories_.pop_back();
    }
}

//-------------------------REAL ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<float>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        QtVariantPropertyManager *realPropertyManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        propertyMgr_ = realPropertyManager;
        factory_ = variantFactory;
        rootProperty_ = realPropertyManager->addProperty(QVariant::Double, name_);

        ComponentPtr comp = components_[0].lock();
        IAttribute *attribute = FindAttribute(comp);
        if (!attribute) 
        {
            LogError("Could not find attribute by " + name_);
            return;
        }

        AttributeMetadata *metaData = attribute->Metadata();
        if (metaData)
        {
            if(!metaData->minimum.isEmpty())
                metaDataFlag_ |= UsingMinValue;
            if(!metaData->maximum.isEmpty())
                metaDataFlag_ |= UsingMaxValue;
            if(!metaData->step.isEmpty())
                metaDataFlag_ |= UsingStepValue;
            if((metaDataFlag_ & UsingMinValue) != 0)
                realPropertyManager->setAttribute(rootProperty_, "minimum", metaData->minimum.toFloat());
            else
                realPropertyManager->setAttribute(rootProperty_, "minimum", -(FLT_MAX));
            if((metaDataFlag_ & UsingMaxValue) != 0)
                realPropertyManager->setAttribute(rootProperty_, "maximum", metaData->maximum.toFloat());
            else
                realPropertyManager->setAttribute(rootProperty_, "maximum", FLT_MAX);
            if((metaDataFlag_ & UsingStepValue) != 0)
                realPropertyManager->setAttribute(rootProperty_, "singleStep", metaData->step.toFloat());
        }
        else
        {
            realPropertyManager->setAttribute(rootProperty_, "minimum", -(FLT_MAX));
            realPropertyManager->setAttribute(rootProperty_, "maximum", FLT_MAX);
        }

        if(rootProperty_)
        {
            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(realPropertyManager, variantFactory);
    }

    emit EditorChanged(name_);
}


template<> void ECAttributeEditor<float>::Update(IAttribute *attr)
{
    Attribute<float> *attribute = 0;
    if (!attr)
        attribute = FindAttribute<float>(components_[0].lock());
    else
        attribute = dynamic_cast<Attribute<float>*>(attr);
    if (!attribute)
    {
        LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer into Attribute<float> format.");
        return;
    }

    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        QtVariantPropertyManager *realPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        assert(realPropertyManager);
        if (realPropertyManager && rootProperty_)
            realPropertyManager->setValue(rootProperty_, attribute->Get());
    }
}

template<> void ECAttributeEditor<float>::Set(QtProperty *property)
{
    if(listenEditorChangedSignal_)
    {
        bool success = false;
        float newValue = QLocale::system().toFloat(property->valueText(), &success);
        if (success)
            SetValue(newValue);
        else
            LogError("ECAttributeEditor<float>: Failed to convert the property value text to float.");
    }
}

//-------------------------INT ATTRIBUTE TYPE-------------------------
template<> void ECAttributeEditor<int>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        ComponentPtr comp = components_[0].lock();
        IAttribute *attribute = FindAttribute(comp);
        if (!attribute)
        {
            LogError("Could not find attribute by " + name_);
            return;
        }

        //Check if int need to have min and max value set and also enum types are presented as a int value.
        AttributeMetadata *metaData = attribute->Metadata();
        if(metaData)
        {
            if(!metaData->enums.empty())
                metaDataFlag_ |= UsingEnums;
            else
            {
                if(!metaData->minimum.isEmpty())
                    metaDataFlag_ |= UsingMinValue;
                if(!metaData->maximum.isEmpty())
                    metaDataFlag_ |= UsingMaxValue;
                if(!metaData->step.isEmpty())
                    metaDataFlag_ |= UsingStepValue;
            }
            if(!metaData->description.isEmpty())
                metaDataFlag_ |= UsingDescription;
        }

        QtVariantPropertyManager *intPropertyManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        // Check if attribute want to use enums.
        if((metaDataFlag_ & UsingEnums) != 0)
        {
            QtVariantProperty *prop = 0;
            prop = intPropertyManager->addProperty(QtVariantPropertyManager::enumTypeId(), name_);
            rootProperty_ = prop;
            QStringList enumNames;
            for(AttributeMetadata::EnumDescMap_t::iterator iter = metaData->enums.begin(); iter != metaData->enums.end(); ++iter)
                enumNames << iter->second;

            prop->setAttribute("enumNames", enumNames);
        }
        else
        {
            rootProperty_ = intPropertyManager->addProperty(QVariant::Int, name_);
            if((metaDataFlag_ & UsingMinValue) != 0)
                intPropertyManager->setAttribute(rootProperty_, "minimum", metaData->minimum.toInt());
            if((metaDataFlag_ & UsingMaxValue) != 0)
                intPropertyManager->setAttribute(rootProperty_, "maximum", metaData->maximum.toInt());
            if((metaDataFlag_ & UsingStepValue) != 0)
                intPropertyManager->setAttribute(rootProperty_, "singleStep", metaData->step.toInt());
        }
        propertyMgr_ = intPropertyManager;
        factory_ = variantFactory;
        if(rootProperty_)
        {
            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(intPropertyManager, variantFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<int>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<int> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<int>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<int>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer into Attribute<int> format.");
            return;
        }

        QtVariantPropertyManager *intPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        assert(intPropertyManager);
        if (intPropertyManager && rootProperty_)
            intPropertyManager->setValue(rootProperty_, attribute->Get());
    }
}

template<> void ECAttributeEditor<int>::Set(QtProperty *property)
{
    if (listenEditorChangedSignal_)
    {
        int newValue = 0;
        QString valueString = property->valueText();
        if ((metaDataFlag_ & UsingEnums) != 0)
        {
            ComponentPtr comp = components_[0].lock();
            IAttribute *attribute = FindAttribute(comp);
            if (!attribute)
            {
                LogError("Could not find attribute by " + name_);
                return;
            }

            AttributeMetadata *metaData = attribute->Metadata();
            AttributeMetadata::EnumDescMap_t::iterator iter = metaData->enums.begin();
            for(; iter != metaData->enums.end(); ++iter)
                if (valueString == iter->second)
                    newValue = iter->first;
        }
        else
            newValue = valueString.toInt();
        SetValue(newValue);
    }
}

//-------------------------UINT ATTRIBUTE TYPE-------------------------
template<> void ECAttributeEditor<unsigned int>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        ComponentPtr comp = components_[0].lock();
        IAttribute *attribute = FindAttribute(comp);
        if (!attribute)
        {
            LogError("Could not find attribute by " + name_);
            return;
        }

        //Check if int need to have min and max value set and also enum types are presented as a int value.
        AttributeMetadata *metaData = attribute->Metadata();
        if (metaData)
        {
            if (!metaData->enums.empty())
                metaDataFlag_ |= UsingEnums;
            else
            {
                if (!metaData->minimum.isEmpty())
                    metaDataFlag_ |= UsingMinValue;
                if (!metaData->maximum.isEmpty())
                    metaDataFlag_ |= UsingMaxValue;
                if (!metaData->step.isEmpty())
                    metaDataFlag_ |= UsingStepValue;
            }
            if (!metaData->description.isEmpty())
                metaDataFlag_ |= UsingDescription;
        }

        QtVariantPropertyManager *uintPropertyManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        // Check if attribute want to use enums.
        if ((metaDataFlag_ & UsingEnums) != 0)
        {
            QtVariantProperty *prop = 0;
            prop = uintPropertyManager->addProperty(QtVariantPropertyManager::enumTypeId(), name_);
            rootProperty_ = prop;
            QStringList enumNames;
            for(AttributeMetadata::EnumDescMap_t::iterator iter = metaData->enums.begin(); iter != metaData->enums.end(); ++iter)
                enumNames << iter->second;

            prop->setAttribute("enumNames", enumNames);
        }
        else
        {
            /// @todo Returns null if QVariant::UInt passed.
            /// Use QVariant::Int and enforce minimum value of 0 always.
            rootProperty_ = uintPropertyManager->addProperty(QVariant::Int, name_);
            uintPropertyManager->setAttribute(rootProperty_, "minimum", 0);

            if ((metaDataFlag_ & UsingMinValue) != 0)
                uintPropertyManager->setAttribute(rootProperty_, "minimum", metaData->minimum.toUInt());
            if ((metaDataFlag_ & UsingMaxValue) != 0)
                uintPropertyManager->setAttribute(rootProperty_, "maximum", metaData->maximum.toUInt());
            if ((metaDataFlag_ & UsingStepValue) != 0)
                uintPropertyManager->setAttribute(rootProperty_, "singleStep", metaData->step.toUInt());
        }

        propertyMgr_ = uintPropertyManager;
        factory_ = variantFactory;
        if (rootProperty_)
        {
            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }

        owner_->setFactoryForManager(uintPropertyManager, variantFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<unsigned int>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<unsigned int> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<unsigned int>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<unsigned int> *>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer into Attribute<unsigned int> format.");
            return;
        }

        QtVariantPropertyManager *intPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        assert(intPropertyManager);
        if (intPropertyManager && rootProperty_)
            intPropertyManager->setValue(rootProperty_, attribute->Get());
    }
}

template<> void ECAttributeEditor<unsigned int>::Set(QtProperty *property)
{
    if (listenEditorChangedSignal_)
    {
        unsigned int newValue = 0;
        QString valueString = property->valueText();
        if ((metaDataFlag_ & UsingEnums) != 0)
        {
            ComponentPtr comp = components_[0].lock();
            IAttribute *attribute = FindAttribute(comp);
            if (!attribute)
            {
                LogError("Could not find attribute by " + name_);
                return;
            }

            AttributeMetadata *metaData = attribute->Metadata();
            AttributeMetadata::EnumDescMap_t::iterator iter = metaData->enums.begin();
            for(; iter != metaData->enums.end(); ++iter)
                if (valueString == iter->second)
                    newValue = iter->first;
        }
        else
            newValue = valueString.toUInt();
        SetValue(newValue);
    }
}

//-------------------------BOOL ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<bool>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        QtVariantPropertyManager *boolPropertyManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        propertyMgr_ = boolPropertyManager;
        factory_ = variantFactory;
        rootProperty_ = boolPropertyManager->addProperty(QVariant::Bool, name_);
        if(rootProperty_)
        {
            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(boolPropertyManager, variantFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<bool>::Set(QtProperty *property)
{
    if (listenEditorChangedSignal_)
        SetValue(ParseBool(property->valueText()));
}

template<> void ECAttributeEditor<bool>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<bool> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<bool>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<bool>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<bool> format.");
            return;
        }

        QtVariantPropertyManager *boolPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if (boolPropertyManager && rootProperty_ && components_.size() > 0)
            boolPropertyManager->setValue(rootProperty_, attribute->Get());
    }
}

// ================================ float2 ==============================
template<> void ECAttributeEditor<float2>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<float2> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<float2>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<float2>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<float2> format.");
            return;
        }

        QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(rootProperty_)
        {
            QList<QtProperty *> children = rootProperty_->subProperties();
            if(children.size() >= 2)
            {
                const float2 &vectorValue = attribute->Get();
                variantManager->setValue(children[0], vectorValue.x);
                variantManager->setValue(children[1], vectorValue.y);
            }
        }
    }
}

template<> void ECAttributeEditor<float2>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        QtVariantPropertyManager *variantManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        propertyMgr_ = variantManager;
        factory_ = variantFactory;
        rootProperty_ = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), name_);

        if(rootProperty_)
        {
            QtVariantProperty *childProperty = 0;
            childProperty = variantManager->addProperty(QVariant::Double, "x");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rootProperty_->addSubProperty(childProperty);
            
            childProperty = variantManager->addProperty(QVariant::Double, "y");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rootProperty_->addSubProperty(childProperty);
            
            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(variantManager, variantFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<float2>::Set(QtProperty *property)
{
    if(listenEditorChangedSignal_)
    {
        QList<QtProperty *> children = rootProperty_->subProperties();
        if(children.size() >= 2)
        {
            Attribute<float2> *attribute = FindAttribute<float2>(components_[0].lock());
            if (!attribute)
                return;

            bool success = false;
            float value = QLocale::system().toFloat(property->valueText(), &success);
            if (success)
            {
                float2 newValue = attribute->Get();
                QString propertyName = property->propertyName();
                if (propertyName == "x")
                    newValue.x = value;
                else if(propertyName == "y")
                    newValue.y = value;
                SetValue(newValue);
            }
            else
                LogError("ECAttributeEditor<float2>: Failed to convert the property value text to float.");
        }
    }
}

// ================================ float3 ==============================
template<> void ECAttributeEditor<float3>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<float3> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<float3>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<float3>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<float3> format.");
            return;
        }

        QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(rootProperty_)
        {
            QList<QtProperty *> children = rootProperty_->subProperties();
            if(children.size() >= 3)
            {
                const float3 &vectorValue = attribute->Get();
                variantManager->setValue(children[0], vectorValue.x);
                variantManager->setValue(children[1], vectorValue.y);
                variantManager->setValue(children[2], vectorValue.z);
            }
        }
    }
}

template<> void ECAttributeEditor<float3>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        QtVariantPropertyManager *variantManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        propertyMgr_ = variantManager;
        factory_ = variantFactory;
        rootProperty_ = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), name_);

        if(rootProperty_)
        {
            QtVariantProperty *childProperty = 0;
            childProperty = variantManager->addProperty(QVariant::Double, "x");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rootProperty_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "y");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rootProperty_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "z");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rootProperty_->addSubProperty(childProperty);
            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(variantManager, variantFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<float3>::Set(QtProperty *property)
{
    if(listenEditorChangedSignal_)
    {
        QList<QtProperty *> children = rootProperty_->subProperties();
        if(children.size() >= 3)
        {
            Attribute<float3> *attribute = FindAttribute<float3>(components_[0].lock());
            if (!attribute)
                return;
                
            bool success = false;
            float value = QLocale::system().toFloat(property->valueText(), &success);
            if (success)
            {
                float3 newValue = attribute->Get();
                QString propertyName = property->propertyName();
                if(propertyName == "x")
                    newValue.x = value;
                else if(propertyName == "y")
                    newValue.y = value;
                else if(propertyName == "z")
                    newValue.z = value;
                SetValue(newValue);
            }
            else
                LogError("ECAttributeEditor<float3>: Failed to convert the property value text to float.");
        }
    }
}

// ================================ float4 ==============================
template<> void ECAttributeEditor<float4>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<float4> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<float4>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<float4>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<float4> format.");
            return;
        }

        QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(rootProperty_)
        {
            QList<QtProperty *> children = rootProperty_->subProperties();
            if(children.size() >= 4)
            {
                const float4 &vectorValue = attribute->Get();
                variantManager->setValue(children[0], vectorValue.x);
                variantManager->setValue(children[1], vectorValue.y);
                variantManager->setValue(children[2], vectorValue.z);
                variantManager->setValue(children[3], vectorValue.w);
            }
        }
    }
}

template<> void ECAttributeEditor<float4>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        QtVariantPropertyManager *variantManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        propertyMgr_ = variantManager;
        factory_ = variantFactory;
        rootProperty_ = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), name_);

        if(rootProperty_)
        {
            QtVariantProperty *childProperty = 0;
            childProperty = variantManager->addProperty(QVariant::Double, "x");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rootProperty_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "y");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rootProperty_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "z");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rootProperty_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "w");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rootProperty_->addSubProperty(childProperty);

            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(variantManager, variantFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<float4>::Set(QtProperty *property)
{
    if(listenEditorChangedSignal_)
    {
        QList<QtProperty *> children = rootProperty_->subProperties();
        if(children.size() >= 4)
        {
            Attribute<float4> *attribute = FindAttribute<float4>(components_[0].lock());
            if (!attribute)
                return;

            bool success = false;
            float value = QLocale::system().toFloat(property->valueText(), &success);
            if (success)
            {
                float4 newValue = attribute->Get();
                QString propertyName = property->propertyName();
                if(propertyName == "x")
                    newValue.x = value;
                else if(propertyName == "y")
                    newValue.y = value;
                else if(propertyName == "z")
                    newValue.z = value;
                else if(propertyName == "w")
                    newValue.w = value;
                SetValue(newValue);
            }
            else
                LogError("ECAttributeEditor<float4>: Failed to convert the property value text to float.");
        }
    }
}

// ================================ Quat ==============================
template<> void ECAttributeEditor<Quat>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<Quat> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<Quat>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<Quat>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<Quat> format.");
            return;
        }

        QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(rootProperty_)
        {
            QList<QtProperty *> children = rootProperty_->subProperties();
            if(children.size() >= 4)
            {
                const Quat &vectorValue = attribute->Get();
                variantManager->setValue(children[0], vectorValue.x);
                variantManager->setValue(children[1], vectorValue.y);
                variantManager->setValue(children[2], vectorValue.z);
                variantManager->setValue(children[2], vectorValue.w);
            }
        }
    }
}

template<> void ECAttributeEditor<Quat>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        QtVariantPropertyManager *variantManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        propertyMgr_ = variantManager;
        factory_ = variantFactory;
        rootProperty_ = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), name_);

        if(rootProperty_)
        {
            QtVariantProperty *childProperty = 0;
            childProperty = variantManager->addProperty(QVariant::Double, "x");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rootProperty_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "y");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rootProperty_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "z");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rootProperty_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "w");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rootProperty_->addSubProperty(childProperty);

            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(variantManager, variantFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<Quat>::Set(QtProperty *property)
{
    if(listenEditorChangedSignal_)
    {
        QList<QtProperty *> children = rootProperty_->subProperties();
        if(children.size() >= 4)
        {
            Attribute<Quat> *attribute = FindAttribute<Quat>(components_[0].lock());
            if (!attribute)
                return;

            bool success = false;
            float value = QLocale::system().toFloat(property->valueText(), &success);
            if (success)
            {
                Quat newValue = attribute->Get();
                QString propertyName = property->propertyName();
                if (propertyName == "x")
                    newValue.x = value;
                else if(propertyName == "y")
                    newValue.y = value;
                else if(propertyName == "z")
                    newValue.z = value;
                else if(propertyName == "w")
                    newValue.w = value;
                SetValue(newValue);
            }
            else
                LogError("ECAttributeEditor<Quat>: Failed to convert the property value text to float.");
        }
    }
}

//-------------------------COLOR ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<Color>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<Color> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<Color>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<Color>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<Color> format.");
            return;
        }

        QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if (rootProperty_)
            variantManager->setValue(rootProperty_, attribute->Get().ToQColor());
    }
}

template<> void ECAttributeEditor<Color>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        QtVariantPropertyManager *variantManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        rootProperty_ = variantManager->addProperty(QVariant::Color, name_);
        propertyMgr_ = variantManager;
        factory_ = variantFactory;
        Update();
        connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        owner_->setFactoryForManager(variantManager, variantFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<Color>::Set(QtProperty * /*property*/)
{
    if(listenEditorChangedSignal_)
    {
        QtVariantProperty *prop = dynamic_cast<QtVariantProperty*>(rootProperty_);
        SetValue(prop->value().value<QColor>());
    }
}

//-------------------------QPOINT ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<QPoint>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<QPoint> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<QPoint>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<QPoint>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<QPoint> format.");
            return;
        }

        QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(rootProperty_)
        {
            QList<QtProperty *> children = rootProperty_->subProperties();
            if(children.size() >= 2)
            {
                QPoint sizeValue = attribute->Get();
                variantManager->setValue(children[0], sizeValue.x());
                variantManager->setValue(children[1], sizeValue.y());
            }
        }
    }
}

template<> void ECAttributeEditor<QPoint>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        QtVariantPropertyManager *variantManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        propertyMgr_ = variantManager;
        factory_ = variantFactory;
        rootProperty_ = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), name_);

        if(rootProperty_)
        {
            QtProperty *childProperty = 0;
            childProperty = variantManager->addProperty(QVariant::Int, "x");
            rootProperty_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Int, "y");
            rootProperty_->addSubProperty(childProperty);

            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(variantManager, variantFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<QPoint>::Set(QtProperty *property)
{
    if(listenEditorChangedSignal_)
    {
        QList<QtProperty *> children = rootProperty_->subProperties();
        if(children.size() >= 2)
        {
            Attribute<QPoint> *attribute = FindAttribute<QPoint>(components_[0].lock());
            if (!attribute)
                return;

            QPoint newValue = attribute->Get();
            QString propertyName = property->propertyName();
            if (propertyName == "x")
                newValue.setX(property->valueText().toInt());
            else if(propertyName == "y")
                newValue.setY(property->valueText().toInt());
            SetValue(newValue);
        }
    }
}

//-------------------------QSTRING ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<QString>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        QtStringPropertyManager *qStringPropertyManager = new QtStringPropertyManager(this);
        LineEditPropertyFactory *lineEditFactory = new LineEditPropertyFactory(this);
        if (components_.size())
        {
            ComponentPtr comp = components_[0].lock();
            if (comp)
            {
                IAttribute *attr = comp->AttributeByName(name_);
                if (attr && attr->Metadata())
                {
                    AttributeMetadata *meta = attr->Metadata();
                    lineEditFactory->SetComponents(rootProperty_, components_);
                    lineEditFactory->AddButtons(meta->buttons);
                }
            }
        }

        connect(this, SIGNAL(OnComponentAdded(QtProperty*, IComponent*)), lineEditFactory, SLOT(ComponentAdded(QtProperty*, IComponent*)));
        connect(this, SIGNAL(OnComponentRemoved(QtProperty*, IComponent*)), lineEditFactory, SLOT(ComponentRemoved(QtProperty*, IComponent*)));

        propertyMgr_ = qStringPropertyManager;
        factory_ = lineEditFactory;
        rootProperty_ = qStringPropertyManager->addProperty(name_);
        if (rootProperty_)
        {
            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }

        owner_->setFactoryForManager(qStringPropertyManager, lineEditFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<QString>::Set(QtProperty *property)
{
    if (listenEditorChangedSignal_)
        SetValue(property->valueText());
}

template<> void ECAttributeEditor<QString>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<QString> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<QString>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<QString>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<QString> format.");
            return;
        }

        QtStringPropertyManager *qStringPropertyManager = dynamic_cast<QtStringPropertyManager *>(propertyMgr_);
        assert(qStringPropertyManager);
        if (!qStringPropertyManager)
            return;

        if (rootProperty_)
            qStringPropertyManager->setValue(rootProperty_, attribute->Get());
    }
}

//---------------------------TRANSFORM----------------------------

template<> void ECAttributeEditor<Transform>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<Transform> *attribute = 0;
        if (!attr)
            attribute = dynamic_cast<Attribute<Transform>*>(FindAttribute(components_[0].lock()));
        else
            attribute = dynamic_cast<Attribute<Transform>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<Transform> format.");
            return;
        }

        QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(rootProperty_)
        {
            QList<QtProperty *> children = rootProperty_->subProperties();
            if(children.size() >= 3)
            {                
                Transform transformValue = attribute->Get();
                QList<QtProperty *> positions = children[0]->subProperties();
                variantManager->setValue(positions[0], transformValue.pos.x);
                variantManager->setValue(positions[1], transformValue.pos.y);
                variantManager->setValue(positions[2], transformValue.pos.z);

                QList<QtProperty *> rotations = children[1]->subProperties();
                variantManager->setValue(rotations[0], transformValue.rot.x);
                variantManager->setValue(rotations[1], transformValue.rot.y);
                variantManager->setValue(rotations[2], transformValue.rot.z);

                QList<QtProperty *> scales    = children[2]->subProperties();
                variantManager->setValue(scales[0], transformValue.scale.x);
                variantManager->setValue(scales[1], transformValue.scale.y);
                variantManager->setValue(scales[2], transformValue.scale.z);
            }
        }
    }
}

template<> void ECAttributeEditor<Transform>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        QtVariantPropertyManager *variantManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        propertyMgr_ = variantManager;
        factory_ = variantFactory;

        rootProperty_ = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), name_);
        if(rootProperty_)
        {
            QtVariantProperty *childProperty = 0;
            QtVariantProperty *positionProperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), "Position");
            rootProperty_->addSubProperty(positionProperty);
            childProperty = variantManager->addProperty(QVariant::Double, "x");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            positionProperty->addSubProperty(childProperty);
                
            childProperty = variantManager->addProperty(QVariant::Double, "y");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            positionProperty->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "z");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            positionProperty->addSubProperty(childProperty);

            QtVariantProperty *rotationProperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), "Rotation");
            rootProperty_->addSubProperty(rotationProperty);
            childProperty = variantManager->addProperty(QVariant::Double, "x");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rotationProperty->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "y");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rotationProperty->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "z");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            rotationProperty->addSubProperty(childProperty);

            QtVariantProperty *scaleProperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), "Scale");
            rootProperty_->addSubProperty(scaleProperty);
            childProperty = variantManager->addProperty(QVariant::Double, "x");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            scaleProperty->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "y");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            scaleProperty->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "z");
            childProperty->setAttribute("minimum", -(FLT_MAX));
            childProperty->setAttribute("maximum", FLT_MAX);
            scaleProperty->addSubProperty(childProperty);

            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(variantManager, variantFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<Transform>::Set(QtProperty *property)
{
    if(listenEditorChangedSignal_)
    {
        QList<QtProperty *> children = rootProperty_->subProperties();
        if(children.size() >= 3)
        {
            Attribute<Transform> *attribute = FindAttribute<Transform>(components_[0].lock());
            if(!attribute)
            {
                LogWarning("Failed to update attribute value in ECEditor. Couldn't dynamic_cast attribute pointer to Attribute<Transform> format.");
                return;
            }
            Transform trans = attribute->Get();

            int foundIndex = -1;
            for(uint i = 0; i < (uint)children.size(); ++i)
            {
                QList<QtProperty *> properties = children[i]->subProperties();
                for(uint j = 0; j < (uint)properties.size(); ++j)
                {
                    if(properties[j] == property)
                    {
                        foundIndex = (i * 3) + j;
                        break;
                    }
                }
            }
            if(foundIndex != -1)
            {
                bool success = false;
                float value = QLocale::system().toFloat(property->valueText(), &success);
                if(!success)
                {
                    LogError("ECAttributeEditor<Transform>: Failed to convert the property value text to float.");
                    return;
                }

                switch(foundIndex)
                {
                case 0:
                    trans.pos.x = value;
                    break;
                case 1:
                    trans.pos.y = value;
                    break;
                case 2:
                    trans.pos.z = value;
                    break;
                case 3:
                    trans.rot.x = value;
                    break;
                case 4:
                    trans.rot.y = value;
                    break;
                case 5:
                    trans.rot.z = value;
                    break;
                case 6:
                    trans.scale.x = value;
                    break;
                case 7:
                    trans.scale.y = value;
                    break;
                case 8:
                    trans.scale.z = value;
                    break;
                }
                SetValue(trans);
            }
        }
    }
}

//-------------------------QVARIANT ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<QVariant>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        QtStringPropertyManager *qStringPropertyManager = new QtStringPropertyManager(this);
        LineEditPropertyFactory *lineEditFactory = new LineEditPropertyFactory(this);
        propertyMgr_ = qStringPropertyManager;
        factory_ = lineEditFactory;
        rootProperty_ = qStringPropertyManager->addProperty(name_);
        if(rootProperty_)
        {
            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(qStringPropertyManager, lineEditFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<QVariant>::Set(QtProperty *property)
{
    if (listenEditorChangedSignal_)
    {
        QVariant value(property->valueText());
        SetValue(value);
    }
}

template<> void ECAttributeEditor<QVariant>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<QVariant> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<QVariant>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<QVariant>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<QVariant> format.");
            return;
        }

        QtStringPropertyManager *qStringPropertyManager = dynamic_cast<QtStringPropertyManager *>(propertyMgr_);
        assert(qStringPropertyManager);
        if(!qStringPropertyManager)
            return;

        if (rootProperty_)
            qStringPropertyManager->setValue(rootProperty_, attribute->Get().toString());
    }
}

//-------------------------QVARIANTLIST ATTRIBUTE TYPE---------------------------

template<> void ECAttributeEditor<QVariantList>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
    }
    else
    {
        QtGroupPropertyManager *groupManager = new QtGroupPropertyManager(this);
        QtStringPropertyManager *stringManager = new QtStringPropertyManager(this);
        LineEditPropertyFactory *lineEditFactory = new LineEditPropertyFactory(this);
        propertyMgr_ = groupManager;
        factory_ = lineEditFactory;
        optionalPropertyManagers_.push_back(stringManager);

        rootProperty_ = groupManager->addProperty();
        if(rootProperty_)
        {
            rootProperty_->setPropertyName(name_);
            QtProperty *childProperty = 0;
            // Get number of elements in attribute array and create for property for each array element.
            Attribute<QVariantList > *attribute = FindAttribute<QVariantList>(components_[0].lock());
            if(!attribute)
            {
                LogWarning("Failed to update attribute value in ECEditor. Couldn't dynamic_cast attribute pointer to Attribute<QVariantList> format.");
                return;
            }
            QVariantList variantArray = attribute->Get();
            for(uint i = 0; i < (uint)variantArray.size(); ++i)
            {
                childProperty = stringManager->addProperty(QString("[%1]").arg(i));
                rootProperty_->addSubProperty(childProperty);
            }
            childProperty = stringManager->addProperty(QString("[%1]").arg(variantArray.size()));
            rootProperty_->addSubProperty(childProperty);

            Update();
            connect(stringManager, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(stringManager, lineEditFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<QVariantList>::Set(QtProperty * /*property*/)
{
    if (listenEditorChangedSignal_)
    {
        Attribute<QVariantList > *attribute = FindAttribute<QVariantList>(components_[0].lock());
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor. Couldn't dynamic_cast attribute pointer to Attribute<QVariantList> format.");
            return;
        }
        QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(optionalPropertyManagers_[0]);
        QList<QtProperty*> children = rootProperty_->subProperties();
        QVariantList value;
        for(int i = 0; i < children.size(); ++i)
        {
            QVariant variant = QVariant(stringManager->value(children[i]));
            if(variant.toString() == "" && i == children.size() - 1)
                continue;
            value.push_back(variant.toString());
        }
        //We wont allow double empty array elements.
        if(value.size() >= 1)
            if(value[value.size() - 1] == "")
                value.pop_back();
        SetValue(value);
        Update();
    }
}

template<> void ECAttributeEditor<QVariantList>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<QVariantList> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<QVariantList>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<QVariantList>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<QVariantList> format.");
            return;
        }

        QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(optionalPropertyManagers_[0]);
        QList<QtProperty*> children = rootProperty_->subProperties();
        QVariantList value = attribute->Get();
        /// @todo It tends to be heavy operation to reinitialize all ui elements when new parameters have been added.
        /// replace this so that only single vector's element is added/deleted from the editor.
        if(value.size() + 1 != children.size())
        {
            UnInitialize();
            Initialize();
        }

        if(value.size() <= children.size())
            for(uint i = 0; i < (uint)value.size(); ++i)
                stringManager->setValue(children[i], value[i].toString());
    }
}

//-------------------------ASSETREFERENCE ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<AssetReference>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<AssetReference> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<AssetReference>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<AssetReference>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<AssetReference> format.");
            return;
        }

        QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(propertyMgr_);
        if (!stringManager)
        {
            LogWarning("Failed to update attribute value in ECEditor, couldn't find stringmanager");
            return;
        }

        stringManager->setValue(rootProperty_, attribute->Get().ref);
    }
}

template<> void ECAttributeEditor<AssetReference>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
        if (factory_)
            connect(factory_, SIGNAL(EditorCreated(QtProperty *, QObject *)), SLOT(HandleNewEditor(QtProperty *, QObject *)));
    }
    else
    {
        QtStringPropertyManager *stringManager = new QtStringPropertyManager(this);
        LineEditPropertyFactory *lineEditFactory = new LineEditPropertyFactory(this);
        connect(lineEditFactory, SIGNAL(EditorCreated(QtProperty *, QObject *)), SLOT(HandleNewEditor(QtProperty *, QObject *)));
        propertyMgr_ = stringManager;
        factory_ = lineEditFactory;

        if (components_.size() && !components_[0].expired())
        {
            IAttribute *attr = components_[0].lock()->AttributeByName(name_);
            if (attr && attr->Metadata())
                //lineEditFactory->SetComponents(rootProperty_, components_);
                lineEditFactory->AddButtons(attr->Metadata()->buttons);
        }

        lineEditFactory->SetComponents(rootProperty_, components_);

        connect(this, SIGNAL(OnComponentAdded(QtProperty*, IComponent*)), lineEditFactory, SLOT(ComponentAdded(QtProperty*, IComponent*)));
        connect(this, SIGNAL(OnComponentRemoved(QtProperty*, IComponent*)), lineEditFactory, SLOT(ComponentRemoved(QtProperty*, IComponent*)));

        rootProperty_ = propertyMgr_->addProperty(name_);
        assert(rootProperty_);
        if (rootProperty_)
        {
            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), SLOT(PropertyChanged(QtProperty*)));
        }

        owner_->setFactoryForManager(stringManager, lineEditFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<AssetReference>::Set(QtProperty *property)
{
    if (listenEditorChangedSignal_)
        SetValue(AssetReference(property->valueText()));
}

void AssetReferenceAttributeEditor::TextEdited(QString text)
{
    if(useMultiEditor_)
    {
        MultiEditPropertyFactory *multiEditFactory = qobject_cast<MultiEditPropertyFactory *>(sender());

        if(multiEditFactory && multiEditFactory->buttonFactory)
        {
            QList<QPushButton*> multiEditButtons = multiEditFactory->buttonFactory->GetButtons(); 
            for(int i = 0; i < multiEditButtons.size(); i++)
            {
                if(multiEditButtons[i]->text() == "E")
                    multiEditButtons[i]->setDisabled(true);
            }
        }
    }else
    {
        LineEditPropertyFactory *lineEditFactory = qobject_cast<LineEditPropertyFactory*>(sender());

        if(lineEditFactory && lineEditFactory->buttonFactory)
        {
            QList<QPushButton*> lineEditButtons = lineEditFactory->buttonFactory->GetButtons(); 
            for(int i = 0; i < lineEditButtons.size(); i++)
            {
                if(lineEditButtons[i]->text() == "E")
                    lineEditButtons[i]->setDisabled(true);
            }
        }
    }
}

void AssetReferenceAttributeEditor::HandleNewEditor(QtProperty *prop, QObject *factory)
{
    QPushButton *pickButton = 0, *editButton = 0;

    if (useMultiEditor_)
    {
        MultiEditPropertyFactory *multiEditFactory = qobject_cast<MultiEditPropertyFactory *>(factory);
        if (multiEditFactory && multiEditFactory->buttonFactory)
        {
            connect(multiEditFactory, SIGNAL(TextEdited(QString)), SLOT(TextEdited(QString)));

            pickButton = multiEditFactory->buttonFactory->AddButton(prop->propertyName(), "...");
            if (IsAssetEditorAvailable())
                editButton = multiEditFactory->buttonFactory->AddButton(prop->propertyName(), tr("E"));
        }
    }
    else
    {
        LineEditPropertyFactory *lineEditFactory = qobject_cast<LineEditPropertyFactory *>(factory);
        if (lineEditFactory && lineEditFactory->buttonFactory)
        {
            connect(lineEditFactory, SIGNAL(TextEdited(QString)), SLOT(TextEdited(QString)));

            pickButton = lineEditFactory->buttonFactory->AddButton(prop->propertyName(), "...");
            if (IsAssetEditorAvailable())
                editButton = lineEditFactory->buttonFactory->AddButton(prop->propertyName(), tr("E"));
        }
    }

    if (pickButton)
    {
        pickButton->setAttribute(Qt::WA_LayoutUsesWidgetRect, true);
        connect(pickButton, SIGNAL(clicked(bool)), SLOT(OpenAssetsWindow()));
    }
    if (editButton)
    {
        editButton->setAttribute(Qt::WA_LayoutUsesWidgetRect, true);
        connect(editButton, SIGNAL(clicked(bool)), SLOT(OpenEditor()));
    }
}

void AssetReferenceAttributeEditor::OpenAssetsWindow()
{
    if (!components_[0].lock().get())
    {
        LogWarning("Cannot create AssetsWindow, no component.");
        return;
    }
    
    Attribute<AssetReference> *assetRef= FindAttribute<AssetReference>(components_[0].lock());
    if (assetRef)
    {
    
        QString assetType = components_[0].lock()->GetFramework()->Asset()->GetResourceTypeFromAssetRef(assetRef->Get());
        LogDebug("Creating AssetsWindow for asset type " + assetType);
        AssetsWindow *assetsWindow = new AssetsWindow(assetType, fw, fw->Ui()->MainWindow());
        connect(assetsWindow, SIGNAL(SelectedAssetChanged(AssetPtr)), SLOT(HandleAssetSelected(AssetPtr)));
        connect(assetsWindow, SIGNAL(AssetPicked(AssetPtr)), SLOT(HandleAssetPicked(AssetPtr)));
        connect(assetsWindow, SIGNAL(PickCanceled()), SLOT(RestoreOriginalValue()));
        assetsWindow->setAttribute(Qt::WA_DeleteOnClose);
        assetsWindow->setWindowFlags(Qt::Tool);
        assetsWindow->show();

        SaveOriginalValue();
    }
}

void AssetReferenceAttributeEditor::SaveOriginalValue()
{
    originalValues.clear();
    
    // Save the original asset ref(s), if we decide to cancel
    foreach(const ComponentWeakPtr &c, components_)
        if (c.lock())
        {
            Attribute<AssetReference> *ref = FindAttribute<AssetReference>(c.lock());
            if (ref)
                originalValues[c] = ref->Get();
        }
}

void AssetReferenceAttributeEditor::HandleAssetPicked(AssetPtr asset)
{
    // Choice was final, set new original values
    originalValues.clear();
    HandleAssetSelected(asset);
    SaveOriginalValue();
}

void AssetReferenceAttributeEditor::HandleAssetSelected(AssetPtr asset)
{
    if (asset)
    {
        LogDebug("AssetReferenceAttributeEditor: Setting new value " + asset->Name());
        SetValue(AssetReference(asset->Name()));
        // Update() does not update immediately, need to reinit
        UnInitialize();
        Initialize();
    }
}


void AssetReferenceAttributeEditor::RestoreOriginalValue()
{
    Attribute<AssetReference> *assetRef= FindAttribute<AssetReference>(components_[0].lock());
    if (!assetRef)
        return;

    for(std::map<ComponentWeakPtr, AssetReference, ComponentWeakPtrLessThan>::iterator it =  originalValues.begin(); it != originalValues.end(); ++it)
        if (!it->first.expired())
            SetValue(it->first.lock(), it->second);

    originalValues.clear();

    Update();
}

void AssetReferenceAttributeEditor::OpenEditor()
{
    Attribute<AssetReference> *assetRef= FindAttribute<AssetReference>(components_[0].lock());
    if (assetRef)
    {
        AssetPtr asset = fw->Asset()->GetAsset(assetRef->Get().ref);
        if (asset)
        {
            QMenu menu;
            fw->Ui()->EmitContextMenuAboutToOpen(&menu, QObjectList(QObjectList() << asset.get()));
            QAction *editAction = menu.findChild<QAction *>("Edit");
            if (editAction)
                editAction->trigger();
        }
    }
}

bool AssetReferenceAttributeEditor::IsAssetEditorAvailable() const
{
    Attribute<AssetReference> *assetRef= FindAttribute<AssetReference>(components_[0].lock());
    if (!assetRef)
        return false;
    AssetPtr asset = fw->Asset()->GetAsset(assetRef->Get().ref);
    if (!asset)
        return false;

    QMenu menu;
    fw->Ui()->EmitContextMenuAboutToOpen(&menu, QObjectList(QObjectList() << asset.get()));
    return menu.findChild<QAction *>("Edit") != 0;
}

//-------------------------ASSETREFERENCELIST ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<AssetReferenceList>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<AssetReferenceList> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<AssetReferenceList>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<AssetReferenceList> *>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<AssetReferenceList> format.");
            return;
        }

        QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(optionalPropertyManagers_[0]);
        QList<QtProperty*> children = rootProperty_->subProperties();
        const AssetReferenceList &value = attribute->Get();
        /// @todo It tends to be heavy operation to reinitialize all ui elements when new parameters have been added.
        /// replace this so that only single vector's element is added/deleted from the editor.
        if (value.Size() + 1 != children.size())
        {
            UnInitialize();
            Initialize();
        }

        if (!stringManager)
        {
            LogWarning("Failed to update attribute value in ECEditor, couldn't find stringmanager");
            return;
        }
        
        if (value.Size() <= children.size())
            for(uint i = 0; i < (uint)value.Size(); ++i)
                stringManager->setValue(children[i], value[i].ref);
    }
}

template<> void ECAttributeEditor<AssetReferenceList>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
        if (factory_)
            connect(factory_, SIGNAL(EditorCreated(QtProperty *, QObject *)), SLOT(HandleNewEditor(QtProperty *, QObject *)));
    }
    else
    {
        QtGroupPropertyManager *groupManager = new QtGroupPropertyManager(this);
        QtStringPropertyManager *stringManager = new QtStringPropertyManager(this);
        LineEditPropertyFactory *lineEditFactory = new LineEditPropertyFactory(this);
        connect(lineEditFactory, SIGNAL(EditorCreated(QtProperty *, QObject *)), SLOT(HandleNewEditor(QtProperty *, QObject *)));
        propertyMgr_ = groupManager;
        factory_ = lineEditFactory;
        optionalPropertyManagers_.push_back(stringManager);

        rootProperty_ = groupManager->addProperty();
        rootProperty_->setPropertyName(name_);
        if (rootProperty_)
        {
            // Get number of elements in attribute array and create for property for each array element.
            Attribute<AssetReferenceList> *attribute = FindAttribute<AssetReferenceList>(components_[0].lock());
            if (!attribute)
            {
                LogWarning("Failed to update attribute value in ECEditor. Couldn't dynamic_cast attribute pointer to Attribute<AssetReferenceList> format.");
                return;
            }

            QtProperty *childProperty = 0;
            const AssetReferenceList &refList = attribute->Get();
            for(uint i = 0; i < (uint)refList.Size(); ++i)
            {
                childProperty = stringManager->addProperty(QString("[%1]").arg(i));
                rootProperty_->addSubProperty(childProperty);
            }

            childProperty = stringManager->addProperty(QString("[%1]").arg(refList.Size()));
            rootProperty_->addSubProperty(childProperty);

            Update();
            connect(stringManager, SIGNAL(propertyChanged(QtProperty*)), SLOT(PropertyChanged(QtProperty*)));
        }

        owner_->setFactoryForManager(stringManager, lineEditFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<AssetReferenceList>::Set(QtProperty * /*property*/)
{
    if (listenEditorChangedSignal_)
    {
        Attribute<AssetReferenceList> *attribute = FindAttribute<AssetReferenceList>(components_[0].lock());
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor. Couldn't dynamic_cast attribute pointer to Attribute<AssetReferenceList> format.");
            return;
        }

        QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(optionalPropertyManagers_[0]);
        QList<QtProperty*> children = rootProperty_->subProperties();
        AssetReferenceList value;
        for(int i = 0; i < children.size(); ++i)
        {
            QVariant variant = QVariant(stringManager->value(children[i]));
            if (variant.toString().isEmpty() && i == children.size() - 1)
                continue;

            value.Append(AssetReference(variant.toString()));
        }

        if (!value.IsEmpty() && value[value.Size() - 1].ref.trimmed().isEmpty())
            value.RemoveLast(); // Two consecutive empty values in array not allowed

        SetValue(value);
        Update();
    }
}

void AssetReferenceListAttributeEditor::TextEdited(QString text)
{
    if(useMultiEditor_)
    {
        MultiEditPropertyFactory *multiEditFactory = qobject_cast<MultiEditPropertyFactory *>(sender());

        if(multiEditFactory && multiEditFactory->buttonFactory)
        {
            QList<QPushButton*> multiEditButtons = multiEditFactory->buttonFactory->GetButtons(); 
            for(int i = 0; i < multiEditButtons.size(); i++)
            {
                if(multiEditButtons[i]->text() == "E")
                    multiEditButtons[i]->setDisabled(true);
            }
        }
    }else
    {
        LineEditPropertyFactory *lineEditFactory = qobject_cast<LineEditPropertyFactory*>(sender());

        if(lineEditFactory && lineEditFactory->buttonFactory)
        {
            QList<QPushButton*> lineEditButtons = lineEditFactory->buttonFactory->GetButtons(); 
            for(int i = 0; i < lineEditButtons.size(); i++)
            {
                if(lineEditButtons[i]->text() == "E")
                    lineEditButtons[i]->setDisabled(true);
            }
        }
    }
}

void AssetReferenceListAttributeEditor::HandleNewEditor(QtProperty *prop, QObject *factory)
{
    // Add button for picking assets always, but editing button only if we have asset editor available.
    QPushButton *pickButton = 0, *editButton = 0;
    if (useMultiEditor_)
    {
        MultiEditPropertyFactory *multiEditFactory = qobject_cast<MultiEditPropertyFactory *>(factory);
        if (multiEditFactory && multiEditFactory->buttonFactory)
        {
            connect(multiEditFactory, SIGNAL(TextEdited(QString)), SLOT(TextEdited(QString)));

            pickButton = multiEditFactory->buttonFactory->AddButton(prop->propertyName(), "...");
            if (IsAssetEditorAvailable())
                editButton = multiEditFactory->buttonFactory->AddButton(prop->propertyName(), tr("E"));
        }
    }
    else
    {
        LineEditPropertyFactory *lineEditFactory = qobject_cast<LineEditPropertyFactory *>(factory);
        if (lineEditFactory && lineEditFactory->buttonFactory)
        {
            connect(lineEditFactory, SIGNAL(TextEdited(QString)), SLOT(TextEdited(QString)));

            pickButton = lineEditFactory->buttonFactory->AddButton(prop->propertyName(), "...");
            if (IsAssetEditorAvailable())
                editButton = lineEditFactory->buttonFactory->AddButton(prop->propertyName(), tr("E"));
        }
    }

    if (pickButton)
        connect(pickButton, SIGNAL(clicked(bool)), SLOT(OpenAssetsWindow()));
    if (editButton)
        connect(editButton, SIGNAL(clicked(bool)), SLOT(OpenEditor()));
}

namespace
{

int ParseIndex(const QString &objName)
{
    int start = objName.lastIndexOf('[');
    int end = objName.lastIndexOf(']');
    if (start == -1 || end == -1)
        return -1;

    bool ok;
    int idx = objName.mid(start+1, end-start-1).toInt(&ok);
    if (!ok || idx < 0)
        return -1;

    return idx;
}

}

void AssetReferenceListAttributeEditor::OpenAssetsWindow()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (!button)
        return;

    if (!components_[0].lock().get())
    {
        LogWarning("Cannot open AssetsWindow, no component.");
        return;
    }
    
    Attribute<AssetReferenceList> *refList = FindAttribute<AssetReferenceList>(components_[0].lock());
    if (!refList)
        return;

    // In multi-edit we have only one index.
    currentIndex = useMultiEditor_ ? 0 : ParseIndex(button->objectName());
    if (currentIndex == -1)
        return;

    QString assetType = refList->Get().type;
    // If no type defined to the AssetReferenceList, try to retrieve it from the first item.
    if (assetType.isEmpty() && !refList->Get().IsEmpty())
    {
        assetType = refList->Get()[0].type;
        // As the last resort, try to figure it out using AssetAPI.
        if (assetType.isEmpty())
            assetType = components_[0].lock()->GetFramework()->Asset()->GetResourceTypeFromAssetRef(refList->Get()[0]);
    }

    LogDebug("OpenAssetsWindow, index " + QString::number(currentIndex));
    LogDebug("Creating AssetsWindow for asset type " + assetType);
    AssetsWindow *assetsWindow = new AssetsWindow(assetType, fw, fw->Ui()->MainWindow());
    connect(assetsWindow, SIGNAL(SelectedAssetChanged(AssetPtr)), SLOT(HandleAssetSelected(AssetPtr)));
    connect(assetsWindow, SIGNAL(AssetPicked(AssetPtr)), SLOT(HandleAssetPicked(AssetPtr)));
    connect(assetsWindow, SIGNAL(PickCanceled()), SLOT(RestoreOriginalValue()));
    assetsWindow->setAttribute(Qt::WA_DeleteOnClose);
    assetsWindow->setWindowFlags(Qt::Tool);
    assetsWindow->show();

    SaveOriginalValue();
}

void AssetReferenceListAttributeEditor::SaveOriginalValue()
{
    originalValues.clear();

    // Save the original asset ref(s), if we decide to cancel
    foreach(const ComponentWeakPtr &c, components_)
        if (!c.expired())
        {
            Attribute<AssetReferenceList> *refs = FindAttribute<AssetReferenceList>(c.lock());
            if (refs)
                originalValues[c] = refs->Get();
        }
}

void AssetReferenceListAttributeEditor::HandleAssetPicked(AssetPtr asset)
{
    // Choice was final, set new original values
    originalValues.clear();
    HandleAssetSelected(asset);
    SaveOriginalValue();
}

void AssetReferenceListAttributeEditor::HandleAssetSelected(AssetPtr asset)
{
    Attribute<AssetReferenceList> *refList = FindAttribute<AssetReferenceList>(components_[0].lock());
    if (!refList)
    {
        currentIndex = -1;
        return;
    }

    if (currentIndex < 0 || (!refList->Get().IsEmpty() && currentIndex >= refList->Get().Size()))
    {
        currentIndex = -1;
        return;
    }

    if (asset)
    {
        LogDebug("AssetReferenceListAttributeEditor: Setting new value " + asset->Name() + " for index " + QString::number(currentIndex));
        AssetReferenceList newRefList = refList->Get();
        if (newRefList.IsEmpty())
            newRefList.Append(AssetReference(asset->Name()));
        else
            newRefList.Set(currentIndex, AssetReference(asset->Name()));

        SetValue(newRefList);
        // Update() does not update immediately, need to reinit
        UnInitialize();
        Initialize();
    }
}

void AssetReferenceListAttributeEditor::RestoreOriginalValue()
{
    Attribute<AssetReferenceList> *refList = FindAttribute<AssetReferenceList>(components_[0].lock());
    if (refList)
        for(std::map<ComponentWeakPtr, AssetReferenceList, ComponentWeakPtrLessThan>::iterator it = originalValues.begin(); it != originalValues.end(); ++it)
            if (!it->first.expired())
                SetValue(it->first.lock(), it->second);

    originalValues.clear();

    Update();
}

void AssetReferenceListAttributeEditor::OpenEditor()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (!button)
        return;

    Attribute<AssetReferenceList> *assetRefList = FindAttribute<AssetReferenceList>(components_[0].lock());
    if (!assetRefList)
        return;

    // In multi-edit we have only one index.
    currentIndex = useMultiEditor_ ? 0 : ParseIndex(button->objectName());
    if (currentIndex == -1)
        return;

    if (assetRefList->Get().IsEmpty())
        return; // If list is empty, do not open (would cause assert in debug mode)

    AssetReference assetRef = assetRefList->Get()[currentIndex];
    AssetPtr asset = fw->Asset()->GetAsset(assetRef.ref);
    if (asset)
    {
        QMenu menu;
        fw->Ui()->EmitContextMenuAboutToOpen(&menu, QObjectList(QObjectList() << asset.get()));
        QAction *editAction = menu.findChild<QAction *>("Edit");
        if (editAction)
            editAction->trigger();
    }
}

bool AssetReferenceListAttributeEditor::IsAssetEditorAvailable() const
{
    Attribute<AssetReferenceList> *assetRefList = FindAttribute<AssetReferenceList>(components_[0].lock());
    if (!assetRefList)
        return false;
    if (assetRefList->Get().IsEmpty())
        return false;
    // Use blindly the first asset ref, to see what kind of assets we're dealing with.
    AssetPtr asset = fw->Asset()->GetAsset(assetRefList->Get()[0].ref);
    if (!asset)
        return false;

    QMenu menu;
    fw->Ui()->EmitContextMenuAboutToOpen(&menu, QObjectList(QObjectList() << asset.get()));
    return menu.findChild<QAction *>("Edit") != 0;
}

//-------------------------ENTITYREFERENCE ATTRIBUTE TYPE------------------------

template<> void ECAttributeEditor<EntityReference>::Update(IAttribute *attr)
{
    if (useMultiEditor_)
    {
        UpdateMultiEditorValue(attr);
    }
    else
    {
        Attribute<EntityReference> *attribute = 0;
        if (!attr)
            attribute = FindAttribute<EntityReference>(components_[0].lock());
        else
            attribute = dynamic_cast<Attribute<EntityReference>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<EntityReference> format.");
            return;
        }

        QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(propertyMgr_);
        if (!stringManager)
        {
            LogWarning("Failed to update attribute value in ECEditor, couldn't find stringmanager");
            return;
        }

        stringManager->setValue(rootProperty_, attribute->Get().ref);
    }
}

template<> void ECAttributeEditor<EntityReference>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (useMultiEditor_)
    {
        InitializeMultiEditor();
//        if (factory_)
//            connect(factory_, SIGNAL(EditorCreated(QtProperty *, QObject *)), SLOT(HandleNewEditor(QtProperty *, QObject *)));
    }
    else
    {
        QtStringPropertyManager *stringManager = new QtStringPropertyManager(this);
        LineEditPropertyFactory *lineEditFactory = new LineEditPropertyFactory(this);
//        connect(lineEditFactory, SIGNAL(EditorCreated(QtProperty *, QObject *)), SLOT(HandleNewEditor(QtProperty *, QObject *)));
        propertyMgr_ = stringManager;
        factory_ = lineEditFactory;

        if (components_.size() && !components_[0].expired())
        {
            IAttribute *attr = components_[0].lock()->AttributeByName(name_);
            if (attr && attr->Metadata())
                //lineEditFactory->SetComponents(rootProperty_, components_);
                lineEditFactory->AddButtons(attr->Metadata()->buttons);
        }

        lineEditFactory->SetComponents(rootProperty_, components_);

        connect(this, SIGNAL(OnComponentAdded(QtProperty*, IComponent*)), lineEditFactory, SLOT(ComponentAdded(QtProperty*, IComponent*)));
        connect(this, SIGNAL(OnComponentRemoved(QtProperty*, IComponent*)), lineEditFactory, SLOT(ComponentRemoved(QtProperty*, IComponent*)));

        rootProperty_ = propertyMgr_->addProperty(name_);
        assert(rootProperty_);
        if (rootProperty_)
        {
            Update();
            connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), SLOT(PropertyChanged(QtProperty*)));
        }

        owner_->setFactoryForManager(stringManager, lineEditFactory);
    }

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<EntityReference>::Set(QtProperty *property)
{
    if (listenEditorChangedSignal_)
        SetValue(EntityReference(property->valueText()));
}
