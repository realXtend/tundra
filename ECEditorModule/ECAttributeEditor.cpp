// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECAttributeEditor.h"
#include "MultiEditPropertyManager.h"
#include "MultiEditPropertyFactory.h"
#include "LineEditPropertyFactory.h"

#include "IComponent.h"
#include "IAttribute.h"
#include "Transform.h"
#include "AssetReference.h"

// QtPropertyBrowser headers.
#include <qtvariantproperty.h>
#include <qtpropertymanager.h>
#include <qtpropertybrowser.h>
#include <qteditorfactory.h>


#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("ECAttributeEditor")

#include "MemoryLeakCheck.h"

ECAttributeEditorBase::ECAttributeEditorBase(QtAbstractPropertyBrowser *owner,
                                             ComponentPtr component,
                                             const QString &name,
                                             const QString &type,
                                             QObject *parent):
    QObject(parent),
    owner_(owner),
    name_(name),
    typeName_(type),
    rootProperty_(0),
    factory_(0),
    propertyMgr_(0),
    listenEditorChangedSignal_(false),
    useMultiEditor_(false),
    metaDataFlag_(0)
{
    AddComponent(component);
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

void ECAttributeEditorBase::AddComponent(ComponentPtr component)
{
    PROFILE(ECAttributeEditor_AddComponent);
    // Before we add new component we make sure that it's not already added
    // and it contains right attribute.
    if(!HasComponent(component) && component->GetAttribute(name_))
    {
        components_.push_back(ComponentWeakPtr(component));
        connect(component.get(), SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), 
                this, SLOT(AttributeChanged(IAttribute*)),
                Qt::UniqueConnection);
        emit OnComponentAdded(rootProperty_, component.get());
    }
}

void ECAttributeEditorBase::RemoveComponent(ComponentPtr component)
{
    ComponentWeakPtrList::iterator iter = FindComponent(component);
    if(iter != components_.end())
    {
        emit OnComponentRemoved(rootProperty_, component.get());
        components_.erase(iter);
        disconnect(component.get(), SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), 
                   this, SLOT(AttributeChanged(IAttribute*)));
    }

    if (components_.isEmpty())
        deleteLater();
}

bool ECAttributeEditorBase::HasComponent(ComponentPtr component)
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
        if (attribute->GetName() == this->name_)
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

IAttribute *ECAttributeEditorBase::FindAttribute(ComponentPtr component) const
{
    PROFILE(ECAttributeEditor_FindAttribute);
    if(component)
        return component->GetAttribute(name_);
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
    if(propertyMgr_ || factory_ || rootProperty_)
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
    if (!useMultiEditor_)
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
            LogError("Could not find attribute by " + name_.toStdString());
            return;
        }

        AttributeMetadata *metaData = attribute->GetMetadata();
        if (metaData)
        {
            if(!metaData->minimum.isEmpty())
                metaDataFlag_ |= UsingMinValue;
            if(!metaData->maximum.isEmpty())
                metaDataFlag_ |= UsingMaxValue;
            if(!metaData->step.isEmpty())
                metaDataFlag_ |= UsingStepValue;
            if((metaDataFlag_ & UsingMinValue) != 0)
                realPropertyManager->setAttribute(rootProperty_, "minimum", ::ParseString<float>(metaData->minimum.toStdString()));
            if((metaDataFlag_ & UsingMaxValue) != 0)
                realPropertyManager->setAttribute(rootProperty_, "maximum", ::ParseString<float>(metaData->maximum.toStdString()));
            if((metaDataFlag_ & UsingStepValue) != 0)
                realPropertyManager->setAttribute(rootProperty_, "singleStep", ::ParseString<float>(metaData->step.toStdString()));
        }

        if(rootProperty_)
        {
            Update();
            QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(realPropertyManager, variantFactory);
    }
    else
    {
        InitializeMultiEditor();
    }
    emit EditorChanged(name_);
}


template<> void ECAttributeEditor<float>::Update(IAttribute *attr)
{
    Attribute<float> *attribute = 0;
    if (!attr)
        attribute = dynamic_cast<Attribute<float>*>(FindAttribute(components_[0].lock()));
    else
        attribute = dynamic_cast<Attribute<float>*>(attr);
    if (!attribute)
    {
        LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer into Attribute<float> format.");
        return;
    }

    if(!useMultiEditor_)
    {
        QtVariantPropertyManager *realPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        assert(realPropertyManager);
        if (!realPropertyManager) 
            return;

        if (rootProperty_)
            realPropertyManager->setValue(rootProperty_, attribute->Get());
    }
    else
        UpdateMultiEditorValue(attr);
}

template<> void ECAttributeEditor<float>::Set(QtProperty *property)
{
    if(listenEditorChangedSignal_)
    {
        float newValue = ParseString<float>(property->valueText().toStdString());
        SetValue(newValue);
    }
}

//-------------------------INT ATTRIBUTE TYPE-------------------------
template<> void ECAttributeEditor<int>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if(!useMultiEditor_)
    {
        ComponentPtr comp = components_[0].lock();
        IAttribute *attribute = FindAttribute(comp);
        if (!attribute)
        {
            LogError("Could not find attribute by " + name_.toStdString());
            return;
        }

        //Check if int need to have min and max value set and also enum types are presented as a int value.
        AttributeMetadata *metaData = attribute->GetMetadata();
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
            AttributeMetadata::EnumDescMap_t::iterator iter = metaData->enums.begin();
            for(; iter != metaData->enums.end(); ++iter)
                enumNames << QString::fromStdString(iter->second);

            prop->setAttribute(QString("enumNames"), enumNames);
        }
        else
        {
            rootProperty_ = intPropertyManager->addProperty(QVariant::Int, name_);
            if((metaDataFlag_ & UsingMinValue) != 0)
                intPropertyManager->setAttribute(rootProperty_, "minimum", ::ParseString<int>(metaData->minimum.toStdString()));
            if((metaDataFlag_ & UsingMaxValue) != 0)
                intPropertyManager->setAttribute(rootProperty_, "maximum", ::ParseString<int>(metaData->maximum.toStdString()));
            if((metaDataFlag_ & UsingStepValue) != 0)
                intPropertyManager->setAttribute(rootProperty_, "singleStep", ::ParseString<int>(metaData->step.toStdString()));
        }
        propertyMgr_ = intPropertyManager;
        factory_ = variantFactory;
        if(rootProperty_)
        {
            Update();
            QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(intPropertyManager, variantFactory);
    }
    else
    {
        InitializeMultiEditor();
    }
    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<int>::Update(IAttribute *attr)
{
    if(!useMultiEditor_)
    {
        Attribute<int> *attribute = 0;
        if (!attr)
            attribute = dynamic_cast<Attribute<int>*>(FindAttribute(components_[0].lock()));
        else
            attribute = dynamic_cast<Attribute<int>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer into Attribute<int> format.");
            return;
        }

        QtVariantPropertyManager *intPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        assert(intPropertyManager);
        if (!intPropertyManager)
            return;

        if(rootProperty_)
            intPropertyManager->setValue(rootProperty_, attribute->Get());
    }
    else
        UpdateMultiEditorValue(attr);
}

template<> void ECAttributeEditor<int>::Set(QtProperty *property)
{
    if (listenEditorChangedSignal_)
    {
        int newValue = 0;
        std::string valueString = property->valueText().toStdString();
        if ((metaDataFlag_ & UsingEnums) != 0)
        {
            ComponentPtr comp = components_[0].lock();
            IAttribute *attribute = FindAttribute(comp);
            if (!attribute)
            {
                LogError("Could not find attribute by " + name_.toStdString());
                return;
            }

            AttributeMetadata *metaData = attribute->GetMetadata();
            AttributeMetadata::EnumDescMap_t::iterator iter = metaData->enums.begin();
            for(; iter != metaData->enums.end(); ++iter)
                if (valueString == iter->second)
                    newValue = iter->first;
        }
        else
            newValue = ParseString<int>(valueString);
        SetValue(newValue);
    }
}

//-------------------------BOOL ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<bool>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if(!useMultiEditor_)
    {
        QtVariantPropertyManager *boolPropertyManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        propertyMgr_ = boolPropertyManager;
        factory_ = variantFactory;
        rootProperty_ = boolPropertyManager->addProperty(QVariant::Bool, name_);
        if(rootProperty_)
        {
            Update();
            QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(boolPropertyManager, variantFactory);
    }
    else
    {
        InitializeMultiEditor();
    }
    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<bool>::Set(QtProperty *property)
{
    if(listenEditorChangedSignal_)
    {
        if(property->valueText().toStdString() == "True")
            SetValue(true);
        else
            SetValue(false);
    }
}

template<> void ECAttributeEditor<bool>::Update(IAttribute *attr)
{
    if(!useMultiEditor_)
    {
        Attribute<bool> *attribute = 0;
        if (!attr)
            attribute = dynamic_cast<Attribute<bool>*>(FindAttribute(components_[0].lock()));
        else
            attribute = dynamic_cast<Attribute<bool>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<bool> format.");
            return;
        }

        QtVariantPropertyManager *boolPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if (!boolPropertyManager)
            return;

        if (rootProperty_ && components_.size() > 0)
            boolPropertyManager->setValue(rootProperty_, attribute->Get());
    }
    else
        UpdateMultiEditorValue(attr);
}

//-------------------------VECTOR3DF ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<Vector3df>::Update(IAttribute *attr)
{
    if(!useMultiEditor_)
    {
        Attribute<Vector3df> *attribute = 0;
        if (!attr)
            attribute = dynamic_cast<Attribute<Vector3df>*>(FindAttribute(components_[0].lock()));
        else
            attribute = dynamic_cast<Attribute<Vector3df>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<Vector3df> format.");
            return;
        }

        QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(rootProperty_)
        {
            QList<QtProperty *> children = rootProperty_->subProperties();
            if(children.size() >= 3)
            {
                Vector3df vectorValue = attribute->Get();
                variantManager->setValue(children[0], vectorValue.x);
                variantManager->setValue(children[1], vectorValue.y);
                variantManager->setValue(children[2], vectorValue.z);
            }
        }
    }
    else
        UpdateMultiEditorValue(attr);
}

template<> void ECAttributeEditor<Vector3df>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if(!useMultiEditor_)
    {
        QtVariantPropertyManager *variantManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        propertyMgr_ = variantManager;
        factory_ = variantFactory;
        rootProperty_ = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), name_);

        if(rootProperty_)
        {
            QtProperty *childProperty = 0;
            childProperty = variantManager->addProperty(QVariant::Double, "x");
            rootProperty_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "y");
            rootProperty_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "z");
            rootProperty_->addSubProperty(childProperty);
            Update();
            QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(variantManager, variantFactory);
    }
    else
    {
        InitializeMultiEditor();
    }
    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<Vector3df>::Set(QtProperty *property)
{
    if(listenEditorChangedSignal_)
    {
        QList<QtProperty *> children = rootProperty_->subProperties();
        if(children.size() >= 3)
        {
            ComponentPtr comp = components_[0].lock();
            Attribute<Vector3df> *attribute = dynamic_cast<Attribute<Vector3df> *>(FindAttribute(comp));
            if (!attribute)
                return;

            Vector3df newValue = attribute->Get();
            QString propertyName = property->propertyName();
            if(propertyName == "x")
                newValue.x = ParseString<float>(property->valueText().toStdString());
            else if(propertyName == "y")
                newValue.y = ParseString<float>(property->valueText().toStdString());
            else if(propertyName == "z")
                newValue.z = ParseString<float>(property->valueText().toStdString());
            SetValue(newValue);
        }
    }
}



//-------------------------QVECTOR3D ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<QVector3D>::Update(IAttribute *attr)
{
    if(!useMultiEditor_)
    {
        Attribute<QVector3D> *attribute = 0;
        if (!attr)
            attribute = dynamic_cast<Attribute<QVector3D>*>(FindAttribute(components_[0].lock()));
        else
            attribute = dynamic_cast<Attribute<QVector3D>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<QVector3D> format.");
            return;
        }

        QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(rootProperty_)
        {
            QList<QtProperty *> children = rootProperty_->subProperties();
            if(children.size() >= 3)
            {
                QVector3D vectorValue = attribute->Get();
                variantManager->setValue(children[0], vectorValue.x());
                variantManager->setValue(children[1], vectorValue.y());
                variantManager->setValue(children[2], vectorValue.z());
            }
        }
    }
    else
        UpdateMultiEditorValue(attr);
}

template<> void ECAttributeEditor<QVector3D>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if(!useMultiEditor_)
    {
        QtVariantPropertyManager *variantManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        propertyMgr_ = variantManager;
        factory_ = variantFactory;
        rootProperty_ = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), name_);

        if(rootProperty_)
        {
            QtProperty *childProperty = 0;
            childProperty = variantManager->addProperty(QVariant::Double, "x");
            rootProperty_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "y");
            rootProperty_->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "z");
            rootProperty_->addSubProperty(childProperty);
            Update();
            QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(variantManager, variantFactory);
    }
    else
    {
        InitializeMultiEditor();
    }
    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<QVector3D>::Set(QtProperty *property)
{
    if(listenEditorChangedSignal_)
    {
        QList<QtProperty *> children = rootProperty_->subProperties();
        if(children.size() >= 3)
        {
            ComponentPtr comp = components_[0].lock();
            Attribute<QVector3D> *attribute = dynamic_cast<Attribute<QVector3D> *>(FindAttribute(comp));
            if (!attribute)
                return;

            QVector3D newValue = attribute->Get();
            QString propertyName = property->propertyName();
            if(propertyName == "x")
                newValue.setX(ParseString<float>(property->valueText().toStdString()));
            else if(propertyName == "y")
                newValue.setY(ParseString<float>(property->valueText().toStdString()));
            else if(propertyName == "z")
                newValue.setZ(ParseString<float>(property->valueText().toStdString()));
            SetValue(newValue);
        }
    }
}


//-------------------------COLOR ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<Color>::Update(IAttribute *attr)
{
    if(!useMultiEditor_)
    {
        Attribute<Color> *attribute = 0;
        if (!attr)
            attribute = dynamic_cast<Attribute<Color>*>(FindAttribute(components_[0].lock()));
        else
            attribute = dynamic_cast<Attribute<Color>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<Color> format.");
            return;
        }

        QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if (rootProperty_)
        {
            Color colorValue = attribute->Get();
            variantManager->setValue(rootProperty_, QVariant::fromValue<QColor>(QColor(colorValue.r * 255, colorValue.g * 255, colorValue.b * 255, colorValue.a * 255)));
        }
    }
    else
        UpdateMultiEditorValue(attr); 
}

template<> void ECAttributeEditor<Color>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if(!useMultiEditor_)
    {
        QtVariantPropertyManager *variantManager = new QtVariantPropertyManager(this);
        QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
        rootProperty_ = variantManager->addProperty(QVariant::Color, name_);
        propertyMgr_ = variantManager;
        factory_ = variantFactory;
        Update();
        QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        owner_->setFactoryForManager(variantManager, variantFactory);
    }
    else
        InitializeMultiEditor();
    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<Color>::Set(QtProperty *property)
{
    if(listenEditorChangedSignal_)
    {
        QtVariantProperty *prop = dynamic_cast<QtVariantProperty*>(rootProperty_);
        QColor value = prop->value().value<QColor>();
        SetValue(Color(value.red() / 255, value.green() / 255, value.blue() / 255, value.alpha() / 255)); 
    }
}

//-------------------------QSTRING ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<QString>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (!useMultiEditor_)
    {
        QtStringPropertyManager *qStringPropertyManager = new QtStringPropertyManager(this);
        LineEditPropertyFactory *lineEditFactory = new LineEditPropertyFactory(this);
        if (components_.size())
        {
            ComponentPtr comp = components_[0].lock();
            if (comp)
            {
                IAttribute *attr = comp->GetAttribute(name_);
                if (attr && attr->HasMetadata())
                {
                    AttributeMetadata *meta = attr->GetMetadata();
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
    else
        InitializeMultiEditor();

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<QString>::Set(QtProperty *property)
{
    if (listenEditorChangedSignal_)
        SetValue(property->valueText());
}

template<> void ECAttributeEditor<QString>::Update(IAttribute *attr)
{
    if (!useMultiEditor_)
    {
        Attribute<QString> *attribute = 0;
        if (!attr)
            attribute = dynamic_cast<Attribute<QString>*>(FindAttribute(components_[0].lock()));
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
    else
        UpdateMultiEditorValue(attr);
}

//---------------------------TRANSFORM----------------------------
template<> void ECAttributeEditor<Transform>::Update(IAttribute *attr)
{
    if(!useMultiEditor_)
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
                variantManager->setValue(positions[0], transformValue.position.x);
                variantManager->setValue(positions[1], transformValue.position.y);
                variantManager->setValue(positions[2], transformValue.position.z);

                QList<QtProperty *> rotations = children[1]->subProperties();
                variantManager->setValue(rotations[0], transformValue.rotation.x);
                variantManager->setValue(rotations[1], transformValue.rotation.y);
                variantManager->setValue(rotations[2], transformValue.rotation.z);

                QList<QtProperty *> scales    = children[2]->subProperties();
                variantManager->setValue(scales[0], transformValue.scale.x);
                variantManager->setValue(scales[1], transformValue.scale.y);
                variantManager->setValue(scales[2], transformValue.scale.z);
            }
        }
    }
    else
        UpdateMultiEditorValue(attr);
}

template<> void ECAttributeEditor<Transform>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if(!useMultiEditor_)
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
            positionProperty->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "y");
            positionProperty->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "z");
            positionProperty->addSubProperty(childProperty);

            QtVariantProperty *rotationProperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), "Rotation");
            rootProperty_->addSubProperty(rotationProperty);
            childProperty = variantManager->addProperty(QVariant::Double, "x");
            rotationProperty->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "y");
            rotationProperty->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "z");
            rotationProperty->addSubProperty(childProperty);

            QtVariantProperty *scaleProperty = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), "Scale");
            rootProperty_->addSubProperty(scaleProperty);
            childProperty = variantManager->addProperty(QVariant::Double, "x");
            scaleProperty->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "y");
            scaleProperty->addSubProperty(childProperty);

            childProperty = variantManager->addProperty(QVariant::Double, "z");
            scaleProperty->addSubProperty(childProperty);

            Update();
            QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(variantManager, variantFactory);
    }
    else
    {
        InitializeMultiEditor();
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
            ComponentPtr comp = components_[0].lock();
            Attribute<Transform> *attribute = dynamic_cast<Attribute<Transform> *>(FindAttribute(comp));
            if(!attribute)
            {
                LogWarning("Failed to update attribute value in ECEditor. Couldn't dynamic_cast attribute pointer to Attribute<Transform> format.");
                return;
            }
            Transform trans = attribute->Get();

            int foundIndex = -1;
            for(uint i = 0; i < children.size(); ++i)
            {
                QList<QtProperty *> properties = children[i]->subProperties();
                for(uint j = 0; j < properties.size(); ++j)
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
                float value = property->valueText().toFloat(&success);
                if(!success)
                {
                    LogError("Failed to convert the property value in float format.");
                    return;
                }

                switch(foundIndex)
                {
                case 0:
                    trans.position.x = value;
                    break;
                case 1:
                    trans.position.y = value;
                    break;
                case 2:
                    trans.position.z = value;
                    break;
                case 3:
                    trans.rotation.x = value;
                    break;
                case 4:
                    trans.rotation.y = value;
                    break;
                case 5:
                    trans.rotation.z = value;
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
    if(!useMultiEditor_)
    {
        QtStringPropertyManager *qStringPropertyManager = new QtStringPropertyManager(this);
        LineEditPropertyFactory *lineEditFactory = new LineEditPropertyFactory(this);
        propertyMgr_ = qStringPropertyManager;
        factory_ = lineEditFactory;
        rootProperty_ = qStringPropertyManager->addProperty(name_);
        if(rootProperty_)
        {
            Update();
            QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(qStringPropertyManager, lineEditFactory);
    }
    else
    {
        InitializeMultiEditor();
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
    if(!useMultiEditor_)
    {
        Attribute<QVariant> *attribute = 0;
        if (!attr)
            attribute = dynamic_cast<Attribute<QVariant>*>(FindAttribute(components_[0].lock()));
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
    else
        UpdateMultiEditorValue(attr);
}

//-------------------------QVARIANTLIST ATTRIBUTE TYPE---------------------------

template<> void ECAttributeEditor<QVariantList>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if(!useMultiEditor_)
    {
        QtGroupPropertyManager *groupManager = new QtGroupPropertyManager(this);
        QtStringPropertyManager *stringManager = new QtStringPropertyManager(this);
        LineEditPropertyFactory *lineEditFactory = new LineEditPropertyFactory(this);
        propertyMgr_ = groupManager;
        factory_ = lineEditFactory;
        optionalPropertyManagers_.push_back(stringManager);

        rootProperty_ = groupManager->addProperty();
        rootProperty_->setPropertyName(name_);
        if(rootProperty_)
        {
            QtProperty *childProperty = 0;
            // Get number of elements in attribute array and create for property for each array element.
            ComponentPtr comp = components_[0].lock();
            Attribute<QVariantList > *attribute = dynamic_cast<Attribute<QVariantList >*>(FindAttribute(comp));
            if(!attribute)
            {
                LogWarning("Failed to update attribute value in ECEditor. Couldn't dynamic_cast attribute pointer to Attribute<QVariantList> format.");
                return;
            }
            QVariantList variantArray = attribute->Get();
            for(uint i = 0; i < variantArray.size(); ++i)
            {
                childProperty = stringManager->addProperty(QString::fromStdString("[" + ::ToString<uint>(i) + "]"));
                rootProperty_->addSubProperty(childProperty);
            }
            childProperty = stringManager->addProperty(QString::fromStdString("[" + ::ToString<uint>(variantArray.size()) + "]"));
            rootProperty_->addSubProperty(childProperty);

            Update();
            QObject::connect(stringManager, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }
        owner_->setFactoryForManager(stringManager, lineEditFactory);
    }
    else
        InitializeMultiEditor();

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<QVariantList>::Set(QtProperty *property)
{
    if (listenEditorChangedSignal_)
    {
        ComponentPtr comp = components_[0].lock();
        Attribute<QVariantList > *attribute = dynamic_cast<Attribute<QVariantList >*>(FindAttribute(comp));
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
    if(!useMultiEditor_)
    {
        Attribute<QVariantList> *attribute = 0;
        if (!attr)
            attribute = dynamic_cast<Attribute<QVariantList>*>(FindAttribute(components_[0].lock()));
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
        //! @todo It tends to be heavy operation to reinitialize all ui elements when new parameters have been added.
        //! replace this so that only single vector's element is added/deleted from the editor.
        if(value.size() + 1 != children.size())
        {
            UnInitialize();
            Initialize();
        }

        if(value.size() <= children.size())
            for(uint i = 0; i < value.size(); ++i)
                stringManager->setValue(children[i], value[i].toString());
    }
    else
        UpdateMultiEditorValue(attr);
}

//-------------------------ASSETREFERENCE ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<AssetReference>::Update(IAttribute *attr)
{
    if (!useMultiEditor_)
    {
        Attribute<AssetReference> *attribute = 0;
        if (!attr)
            attribute = dynamic_cast<Attribute<AssetReference>*>(FindAttribute(components_[0].lock()));
        else
            attribute = dynamic_cast<Attribute<AssetReference>*>(attr);
        if (!attribute)
        {
            LogWarning("Failed to update attribute value in ECEditor, Couldn't dynamic_cast attribute pointer to Attribute<AssetReference> format.");
            return;
        }

        QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(propertyMgr_);
        if (!stringManager)
            return;

        stringManager->setValue(rootProperty_, attribute->Get().ref);
    }
    else
        UpdateMultiEditorValue(attr);
}

template<> void ECAttributeEditor<AssetReference>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (!useMultiEditor_)
    {
        QtStringPropertyManager *stringManager = new QtStringPropertyManager(this);
        LineEditPropertyFactory *lineEditFactory = new LineEditPropertyFactory(this);
        propertyMgr_ = stringManager;
        factory_ = lineEditFactory;

        if (components_.size())
        {
            ComponentPtr comp = components_[0].lock();
            if (comp)
            {
                IAttribute *attr = comp->GetAttribute(name_);
                if (attr && attr->HasMetadata())
                {
                    AttributeMetadata *meta = attr->GetMetadata();
                    lineEditFactory->SetComponents(rootProperty_, components_);
                    lineEditFactory->AddButtons(meta->buttons);
                }
            }
        }
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
    else
        InitializeMultiEditor();

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<AssetReference>::Set(QtProperty *property)
{
    if (listenEditorChangedSignal_)
        SetValue(AssetReference(property->valueText()));
}

//-------------------------ASSETREFERENCELIST ATTRIBUTE TYPE-------------------------

template<> void ECAttributeEditor<AssetReferenceList>::Update(IAttribute *attr)
{
    if (!useMultiEditor_)
    {
        Attribute<AssetReferenceList> *attribute = 0;
        if (!attr)
            attribute = dynamic_cast<Attribute<AssetReferenceList> *>(FindAttribute(components_[0].lock()));
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
        //! @todo It tends to be heavy operation to reinitialize all ui elements when new parameters have been added.
        //! replace this so that only single vector's element is added/deleted from the editor.
        if (value.Size() + 1 != children.size())
        {
            UnInitialize();
            Initialize();
        }

        if (value.Size() <= children.size())
            for(uint i = 0; i < value.Size(); ++i)
                stringManager->setValue(children[i], value[i].ref);
    }
    else
        UpdateMultiEditorValue(attr);
}

template<> void ECAttributeEditor<AssetReferenceList>::Initialize()
{
    ECAttributeEditorBase::PreInitialize();
    if (!useMultiEditor_)
    {
        QtGroupPropertyManager *groupManager = new QtGroupPropertyManager(this);
        QtStringPropertyManager *stringManager = new QtStringPropertyManager(this);
        LineEditPropertyFactory *lineEditFactory = new LineEditPropertyFactory(this);
        propertyMgr_ = groupManager;
        factory_ = lineEditFactory;
        optionalPropertyManagers_.push_back(stringManager);

        rootProperty_ = groupManager->addProperty();
        rootProperty_->setPropertyName(name_);
        if (rootProperty_)
        {
            // Get number of elements in attribute array and create for property for each array element.
            ComponentPtr comp = components_[0].lock();
            Attribute<AssetReferenceList> *attribute = dynamic_cast<Attribute<AssetReferenceList> *>(FindAttribute(comp));
            if (!attribute)
            {
                LogWarning("Failed to update attribute value in ECEditor. Couldn't dynamic_cast attribute pointer to Attribute<AssetReferenceList> format.");
                return;
            }

            QtProperty *childProperty = 0;
            const AssetReferenceList &refList = attribute->Get();
            for(uint i = 0; i < refList.Size(); ++i)
            {
                childProperty = stringManager->addProperty(QString::fromStdString("[" + ::ToString<uint>(i) + "]"));
                rootProperty_->addSubProperty(childProperty);
            }

            childProperty = stringManager->addProperty(QString::fromStdString("[" + ::ToString<uint>(refList.Size()) + "]"));
            rootProperty_->addSubProperty(childProperty);

            Update();
            QObject::connect(stringManager, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(PropertyChanged(QtProperty*)));
        }

        owner_->setFactoryForManager(stringManager, lineEditFactory);
    }
    else
        InitializeMultiEditor();

    emit EditorChanged(name_);
}

template<> void ECAttributeEditor<AssetReferenceList>::Set(QtProperty *property)
{
    if (listenEditorChangedSignal_)
    {
        ComponentPtr comp = components_[0].lock();
        Attribute<AssetReferenceList> *attribute = dynamic_cast<Attribute<AssetReferenceList> *>(FindAttribute(comp));
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
            if (variant.toString() == "" && i == children.size() - 1)
                continue;

            value.Append(AssetReference(variant.toString()));
        }

        // We won't allow double empty array elements.
        if (!value.IsEmpty())
            if (value[value.Size() - 1].ref.trimmed().isEmpty())
                value.RemoveLast();

        SetValue(value);
        Update();
    }
}

