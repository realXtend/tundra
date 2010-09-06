// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECEditorModule.h"
#include "ECAttributeEditor.h"
#include "AttributeInterface.h"
#include "MultiEditPropertyManager.h"
#include "MultiEditPropertyFactory.h"
#include "LineEditPropertyFactory.h"
#include "Transform.h"

// QtPropertyBrowser headers.
#include <qtvariantproperty.h>
#include <qtpropertymanager.h>
#include <qtpropertybrowser.h>
#include <qteditorfactory.h>

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    ECAttributeEditorBase::ECAttributeEditorBase(QtAbstractPropertyBrowser *owner,
                                                 Foundation::AttributeInterface *attribute,
                                                 QObject *parent):
        QObject(parent),
        owner_(owner),
        rootProperty_(0),
        factory_(0),
        propertyMgr_(0),
        listenEditorChangedSignal_(false),
        useMultiEditor_(false),
        editorState_(Uninitialized),
        metaDataFlag_(0)
    {
        attributeName_ = attribute->GetName();
        attributes_.push_back(attribute);
    }

    ECAttributeEditorBase::~ECAttributeEditorBase()
    {
        UnInitialize();
    }

    bool ECAttributeEditorBase::ContainProperty(QtProperty *property) const
    {
        QSet<QtProperty *> properties = propertyMgr_->properties();
        QSet<QtProperty *>::const_iterator iter = properties.find(property);
        if(iter != properties.end())
            return true;
        return false;
    }

    void ECAttributeEditorBase::UpdateEditorUI()
    {
        if(attributes_.size() == 1)
        {
            if(!useMultiEditor_ && editorState_ != Uninitialized)
                Update(); 
            else
            {
                useMultiEditor_ = false;
                Initialize();
            }
            emit AttributeChanged(attributeName_.toStdString());
        }
        else if(attributes_.size() > 1)
        {
            if(!IsIdentical())
            {
                if(!useMultiEditor_)
                {
                    useMultiEditor_ = true;
                    UnInitialize();
                }
            }
            else
            {
                if(useMultiEditor_)
                {
                    useMultiEditor_ = false;
                    UnInitialize();
                }
            }

            if(editorState_ == Uninitialized)
                Initialize();
            else
                Update();
            emit AttributeChanged(attributeName_.toStdString()); 
        }
    }

    void ECAttributeEditorBase::AddNewAttribute(Foundation::AttributeInterface *attribute)
    {
        AttributeList::iterator iter = attributes_.begin();
        for(;iter != attributes_.end(); iter++)
        {
            if(*iter == attribute)
                return;
        }
        attributes_.push_back(attribute);
        UpdateEditorUI();
    }

    void ECAttributeEditorBase::RemoveAttribute(Foundation::AttributeInterface *attribute)
    {
        AttributeList::iterator iter = attributes_.begin();
        for(;iter != attributes_.end(); iter++)
        {
            if(*iter == attribute)
            {
                attributes_.erase(iter);
                UpdateEditorUI();
                return;
            }
        }
    }

    bool ECAttributeEditorBase::IsIdentical() const
    {
        if(attributes_.size() > 1)
        {
            AttributeList::const_iterator iter = attributes_.begin();
            std::string value = (*iter)->ToString();
            while(iter != attributes_.end())
            {
                if(value != (*iter)->ToString())
                    return false;
                iter++;
            }
        }
        return true;
    }

    void ECAttributeEditorBase::PreInitialize()
    {
        if(propertyMgr_ || factory_ || rootProperty_)
            UnInitialize();
        editorState_ = WaitingForResponse;
    }

    void ECAttributeEditorBase::UnInitialize()
    {
        if(owner_)
        {
            owner_->unsetFactoryForManager(propertyMgr_);
            for(uint i = 0; i < optionalPropertyManagers_.size(); i++)
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
        editorState_ = Uninitialized;
    }

    //-------------------------REAL ATTRIBUTE TYPE-------------------------

    template<> void ECAttributeEditor<Real>::Initialize()
    {
        ECAttributeEditorBase::PreInitialize();
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *realPropertyManager = new QtVariantPropertyManager(this);
            QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
            propertyMgr_ = realPropertyManager;
            factory_ = variantFactory;
            rootProperty_ = realPropertyManager->addProperty(QVariant::Double, attributeName_);

            Foundation::AttributeMetadata *metaData = (*attributes_.begin())->GetMetadata();
            if(metaData)
            {
                if(!metaData->min.isEmpty())
                    metaDataFlag_ |= UsingMinValue;
                if(!metaData->max.isEmpty())
                    metaDataFlag_ |= UsingMaxValue;
                if(!metaData->step.isEmpty())
                    metaDataFlag_ |= UsingStepValue;
                if((metaDataFlag_ & UsingMinValue) != 0)
                    realPropertyManager->setAttribute(rootProperty_, "minimum", ::ParseString<Real>(metaData->min.toStdString()));
                if((metaDataFlag_ & UsingMaxValue) != 0)
                    realPropertyManager->setAttribute(rootProperty_, "maximum", ::ParseString<Real>(metaData->max.toStdString()));
                if((metaDataFlag_ & UsingStepValue) != 0)
                    realPropertyManager->setAttribute(rootProperty_, "singleStep", ::ParseString<Real>(metaData->step.toStdString()));
            }

            if(rootProperty_)
            {
                Update();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SetAttribute(QtProperty*)));
            }
            owner_->setFactoryForManager(realPropertyManager, variantFactory);
        }
        else
        {
            InitializeMultiEditor();
        }
    }


    template<> void ECAttributeEditor<Real>::Update()
    {
        if(!useMultiEditor_)
        {
            AttributeList::iterator iter = attributes_.begin();
            QtVariantPropertyManager *realPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            assert(realPropertyManager);
            if(!realPropertyManager)
                return;

            if(iter != attributes_.end())
            {
                if(rootProperty_)
                {
                    Foundation::Attribute<Real> *attribute = dynamic_cast<Foundation::Attribute<Real>*>(*iter);
                    realPropertyManager->setValue(rootProperty_, attribute->Get());
                }
            }
        }
        else
            UpdateMultiEditorValue();
    }

    template<> void ECAttributeEditor<Real>::Set(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            Real newValue = ParseString<Real>(property->valueText().toStdString());
            SetValue(newValue);
        }
    }

    //-------------------------INT ATTRIBUTE TYPE-------------------------

    template<> void ECAttributeEditor<int>::Update()
    {
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *intPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            AttributeList::iterator iter = attributes_.begin();
            assert(intPropertyManager);
            if(!intPropertyManager)
                return;

            if(iter != attributes_.end())
            {
                if(rootProperty_)
                {
                    Foundation::Attribute<int> *attribute = dynamic_cast<Foundation::Attribute<int>*>(*iter);
                    intPropertyManager->setValue(rootProperty_, attribute->Get());
                }
            }
        }
        else
            UpdateMultiEditorValue();
    }

    template<> void ECAttributeEditor<int>::Initialize()
    {
        ECAttributeEditorBase::PreInitialize();
        if(!useMultiEditor_)
        {
            //Check if int need to have min and max value setted and also enum types are presented as a int value.
            Foundation::AttributeMetadata *metaData = (*attributes_.begin())->GetMetadata();
            if(metaData)
            {
                if(!metaData->enums.empty())
                    metaDataFlag_ |= UsingEnums;
                else
                {
                    if(!metaData->min.isEmpty())
                        metaDataFlag_ |= UsingMinValue;
                    if(!metaData->max.isEmpty())
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
                prop = intPropertyManager->addProperty(QtVariantPropertyManager::enumTypeId(), attributeName_);
                rootProperty_ = prop;
                QStringList enumNames;
                Foundation::EnumDescMap_t::iterator iter = metaData->enums.begin();
                for(; iter != metaData->enums.end(); iter++)
                {
                    QString enumValue = QString::fromStdString(iter->second);
                    enumNames << enumValue;
                }
                prop->setAttribute(QString("enumNames"), enumNames);
            }
            else
            {
                rootProperty_ = intPropertyManager->addProperty(QVariant::Int, attributeName_);
                if((metaDataFlag_ & UsingMinValue) != 0)
                    intPropertyManager->setAttribute(rootProperty_, "minimum", ::ParseString<int>(metaData->min.toStdString()));
                if((metaDataFlag_ & UsingMaxValue) != 0)
                    intPropertyManager->setAttribute(rootProperty_, "maximum", ::ParseString<int>(metaData->max.toStdString()));
                if((metaDataFlag_ & UsingStepValue) != 0)
                    intPropertyManager->setAttribute(rootProperty_, "singleStep", ::ParseString<int>(metaData->step.toStdString()));
            }
            propertyMgr_ = intPropertyManager;
            factory_ = variantFactory;
            if(rootProperty_)
            {
                Update();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SetAttribute(QtProperty*)));
            }
            owner_->setFactoryForManager(intPropertyManager, variantFactory);
        }
        else
        {
            InitializeMultiEditor();
        }
    }

    template<> void ECAttributeEditor<int>::Set(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            int newValue = 0;
            std::string valueString = property->valueText().toStdString();
            if((metaDataFlag_ & UsingEnums) != 0)
            {
                Foundation::AttributeMetadata *metaData = (*attributes_.begin())->GetMetadata();
                Foundation::EnumDescMap_t::iterator iter = metaData->enums.begin();
                for(; iter != metaData->enums.end(); iter++)
                {
                    if(valueString == iter->second)
                        newValue = iter->first;
                }
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
            rootProperty_ = boolPropertyManager->addProperty(QVariant::Bool, attributeName_);
            if(rootProperty_)
            {
                Update();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SetAttribute(QtProperty*)));
            }
            owner_->setFactoryForManager(boolPropertyManager, variantFactory);
        }
        else
        {
            InitializeMultiEditor();
        }
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

    template<> void ECAttributeEditor<bool>::Update()
    {
        if(!useMultiEditor_)
        {
            AttributeList::iterator iter = attributes_.begin();
            QtVariantPropertyManager *boolPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            if(!boolPropertyManager)
                return;

            if(iter != attributes_.end())
            {
                if(rootProperty_)
                {
                    Foundation::Attribute<bool> *attribute = dynamic_cast<Foundation::Attribute<bool>*>(*iter);
                    boolPropertyManager->setValue(rootProperty_, attribute->Get());
                }
            }
        }
        else
            UpdateMultiEditorValue();
    }

    //-------------------------VECTOR3DF ATTRIBUTE TYPE-------------------------

    template<> void ECAttributeEditor<Vector3df>::Update()
    {
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            if(rootProperty_)
            {
                QList<QtProperty *> children = rootProperty_->subProperties();
                if(children.size() >= 3)
                {
                    Foundation::Attribute<Vector3df> *attribute = dynamic_cast<Foundation::Attribute<Vector3df> *>(*(attributes_.begin()));
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

    template<> void ECAttributeEditor<Vector3df>::Initialize()
    {
        ECAttributeEditorBase::PreInitialize();
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
                Update();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SetAttribute(QtProperty*)));
            }
            owner_->setFactoryForManager(variantManager, variantFactory);
        }
        else
        {
            InitializeMultiEditor();
        }
    }

    template<> void ECAttributeEditor<Vector3df>::Set(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            QList<QtProperty *> children = rootProperty_->subProperties();
            if(children.size() >= 3)
            {
                Foundation::Attribute<Vector3df> *attribute = dynamic_cast<Foundation::Attribute<Vector3df> *>(*(attributes_.begin()));
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

    //-------------------------COLOR ATTRIBUTE TYPE-------------------------

    template<> void ECAttributeEditor<Color>::Update()
    {
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            if(rootProperty_)
            {
                QList<QtProperty *> children = rootProperty_->subProperties();
                if(children.size() >= 4)
                {
                    Foundation::Attribute<Color> *attribute = dynamic_cast<Foundation::Attribute<Color> *>(*(attributes_.begin()));
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

    template<> void ECAttributeEditor<Color>::Initialize()
    {
        ECAttributeEditorBase::PreInitialize();
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

                Update();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SetAttribute(QtProperty*)));
            }
            owner_->setFactoryForManager(variantManager, variantFactory);
        }
        else
        {
            InitializeMultiEditor();
        }
    }

    template<> void ECAttributeEditor<Color>::Set(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            QList<QtProperty *> children = rootProperty_->subProperties();
            if(children.size() >= 4)
            {
                Foundation::Attribute<Color> *attribute = dynamic_cast<Foundation::Attribute<Color> *>(*(attributes_.begin()));
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

    //-------------------------QSTRING ATTRIBUTE TYPE-------------------------

    template<> void ECAttributeEditor<QString>::Initialize()
    {
        ECAttributeEditorBase::PreInitialize();
        if(!useMultiEditor_)
        {
            QtStringPropertyManager *qStringPropertyManager = new QtStringPropertyManager(this);
            ECEditor::LineEditPropertyFactory *lineEditFactory = new ECEditor::LineEditPropertyFactory(this);
            propertyMgr_ = qStringPropertyManager;
            factory_ = lineEditFactory;
            rootProperty_ = qStringPropertyManager->addProperty(attributeName_);
            if(rootProperty_)
            {
                Update();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SetAttribute(QtProperty*)));
            }
            owner_->setFactoryForManager(qStringPropertyManager, lineEditFactory);
        }
        else
        {
            InitializeMultiEditor();
        }
    }

    template<> void ECAttributeEditor<QString>::Set(QtProperty *property)
    {
        if (listenEditorChangedSignal_)
            SetValue(property->valueText());
    }

    template<> void ECAttributeEditor<QString>::Update()
    {
        if(!useMultiEditor_)
        {
            AttributeList::iterator iter = attributes_.begin();
            QtStringPropertyManager *qStringPropertyManager = dynamic_cast<QtStringPropertyManager *>(propertyMgr_);

            assert(qStringPropertyManager);
            if(!qStringPropertyManager)
                return;

            if(iter != attributes_.end())
            {
                if (rootProperty_)
                {
                    Foundation::Attribute<QString> *attribute = dynamic_cast<Foundation::Attribute<QString>*>(*iter);
                    qStringPropertyManager->setValue(rootProperty_, attribute->Get());
                }
                iter++;
            }
        }
        else
            UpdateMultiEditorValue();
    }

    //-------------------------QVARIANT ATTRIBUTE TYPE-------------------------

    template<> void ECAttributeEditor<QVariant>::Initialize()
    {
        ECAttributeEditorBase::PreInitialize();
        if(!useMultiEditor_)
        {
            QtStringPropertyManager *qStringPropertyManager = new QtStringPropertyManager(this);
            ECEditor::LineEditPropertyFactory *lineEditFactory = new ECEditor::LineEditPropertyFactory(this);
            propertyMgr_ = qStringPropertyManager;
            factory_ = lineEditFactory;
            rootProperty_ = qStringPropertyManager->addProperty(attributeName_);
            if(rootProperty_)
            {
                Update();
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SetAttribute(QtProperty*)));
            }
            owner_->setFactoryForManager(qStringPropertyManager, lineEditFactory);
        }
        else
        {
            InitializeMultiEditor();
        }
    }

    template<> void ECAttributeEditor<QVariant>::Set(QtProperty *property)
    {
        if (listenEditorChangedSignal_)
        {
            QVariant value(property->valueText());
            SetValue(value);
        }
    }

    template<> void ECAttributeEditor<QVariant>::Update()
    {
        if(!useMultiEditor_)
        {
            AttributeList::iterator iter = attributes_.begin();
            QtStringPropertyManager *qStringPropertyManager = dynamic_cast<QtStringPropertyManager *>(propertyMgr_);
            assert(qStringPropertyManager);
            if(!qStringPropertyManager)
                return;

            if(iter != attributes_.end())
            {
                if (rootProperty_)
                {
                    Foundation::Attribute<QVariant> *attribute = dynamic_cast<Foundation::Attribute<QVariant>*>(*iter);
                    qStringPropertyManager->setValue(rootProperty_, attribute->Get().toString());
                }
                iter++;
            }
        }
        else
            UpdateMultiEditorValue();
    }

    //-------------------------QVARIANTARRAY ATTRIBUTE TYPE-------------------------

    template<> void ECAttributeEditor<std::vector<QVariant> >::Initialize()
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
            rootProperty_->setPropertyName(attributeName_);
            if(rootProperty_)
            {
                QtProperty *childProperty = 0;
                // Get number of elements in attribute array and create for property for each array element.
                Foundation::Attribute<std::vector<QVariant> > *attribute = dynamic_cast<Foundation::Attribute<std::vector<QVariant> >*>(*(attributes_.begin()));
                std::vector<QVariant> variantArray = attribute->Get();
                for(uint i = 0; i < variantArray.size(); i++)
                {
                    childProperty = stringManager->addProperty(QString::fromStdString("[" + ::ToString<uint>(i) + "]"));
                    rootProperty_->addSubProperty(childProperty);
                }
                childProperty = stringManager->addProperty(QString::fromStdString("[" + ::ToString<uint>(variantArray.size()) + "]"));
                rootProperty_->addSubProperty(childProperty);

                Update();
                QObject::connect(stringManager, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SetAttribute(QtProperty*)));
            }
            owner_->setFactoryForManager(stringManager, lineEditFactory);
        }
        else
            InitializeMultiEditor();
    }

    template<> void ECAttributeEditor<std::vector<QVariant> >::Set(QtProperty *property)
    {
        if (listenEditorChangedSignal_)
        {
            Foundation::Attribute<std::vector<QVariant> > *attribute = dynamic_cast<Foundation::Attribute<std::vector<QVariant> >*>(*(attributes_.begin()));
            QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(optionalPropertyManagers_[0]);
            QList<QtProperty*> children = rootProperty_->subProperties();
            std::vector<QVariant> value;
            for(uint i = 0; i < children.size(); i++)
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
        }
    }

    template<> void ECAttributeEditor<std::vector<QVariant> >::Update()
    {
        if(!useMultiEditor_)
        {
            Foundation::Attribute<std::vector<QVariant> > *attribute = dynamic_cast<Foundation::Attribute<std::vector<QVariant> >*>(*(attributes_.begin()));
            QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(optionalPropertyManagers_[0]);
            QList<QtProperty*> children = rootProperty_->subProperties();
            std::vector<QVariant> value = attribute->Get();
            //! @todo It's tend to be heavy operation to reinitialize all ui elements when new parameters have been added.
            //! replace this so that only single vector's element is added/deleted from the editor.
            if(value.size() + 1 != children.size())
            {
                UnInitialize();
                Initialize();
            }
            if(value.size() <= children.size())
            {
                for(uint i = 0; i < value.size(); i++)
                {
                    stringManager->setValue(children[i], value[i].toString());
                }
            }
        }
        else
            UpdateMultiEditorValue();
    }

    //-------------------------ASSETREFERENCE ATTRIBUTE TYPE-------------------------

    template<> void ECAttributeEditor<Foundation::AssetReference>::Update()
    {
        if(!useMultiEditor_)
        {
            QList<QtProperty*> children = rootProperty_->subProperties();
            if(children.size() == 2)
            {
                QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(children[0]->propertyManager());
                Foundation::Attribute<Foundation::AssetReference> *attribute = dynamic_cast<Foundation::Attribute<Foundation::AssetReference> *>(*(attributes_.begin()));
                if(!attribute || !stringManager)
                    return;

                Foundation::AssetReference value = attribute->Get();
                stringManager->setValue(children[0], QString::fromStdString(value.id_));
                stringManager->setValue(children[1], QString::fromStdString(value.type_));
            }
        }
        else
            UpdateMultiEditorValue();
    }

    template<> void ECAttributeEditor<Foundation::AssetReference>::Initialize()
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
            rootProperty_->setPropertyName(attributeName_);
            if(rootProperty_)
            {
                QtProperty *childProperty = 0;
                childProperty = stringManager->addProperty("Asset ID");
                rootProperty_->addSubProperty(childProperty);

                childProperty = stringManager->addProperty("Asset type");
                rootProperty_->addSubProperty(childProperty);

                Update();
                QObject::connect(stringManager, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SetAttribute(QtProperty*)));
            }
            owner_->setFactoryForManager(stringManager, lineEditFactory);
        }
        else
            InitializeMultiEditor();
    }

    template<> void ECAttributeEditor<Foundation::AssetReference>::Set(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            QList<QtProperty*> children = rootProperty_->subProperties();
            if(children.size() == 2)
            {
                QtStringPropertyManager *stringManager = dynamic_cast<QtStringPropertyManager *>(children[0]->propertyManager());
                Foundation::Attribute<Foundation::AssetReference> *attribute = dynamic_cast<Foundation::Attribute<Foundation::AssetReference> *>(*(attributes_.begin()));
                if(!attribute || !stringManager)
                    return;

                Foundation::AssetReference value;
                value.id_ = stringManager->value(children[0]).toStdString();
                value.type_ = stringManager->value(children[1]).toStdString();
                SetValue(value);
            }
        }
    }

    //---------------------------TRANSFORM----------------------------
    template<> void ECAttributeEditor<Transform>::Update()
    {
        if(!useMultiEditor_)
        {
            QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            if(rootProperty_)
            {
                QList<QtProperty *> children = rootProperty_->subProperties();
                if(children.size() >= 3)
                {
                    Foundation::Attribute<Transform> *attribute = dynamic_cast<Foundation::Attribute<Transform> *>(*(attributes_.begin()));
                    if(!attribute)
                        return;

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
            UpdateMultiEditorValue();
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

            rootProperty_ = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), attributeName_);
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
                QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SetAttribute(QtProperty*)));
            }
            owner_->setFactoryForManager(variantManager, variantFactory);
        }
        else
        {
            InitializeMultiEditor();
        }
    }

    template<> void ECAttributeEditor<Transform>::Set(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            QtVariantPropertyManager *variantManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
            QList<QtProperty *> children = rootProperty_->subProperties();
            if(children.size() >= 3)
            {
                Foundation::Attribute<Transform> *attribute = dynamic_cast<Foundation::Attribute<Transform> *>(*(attributes_.begin()));
                if(!attribute)
                    return;
                Transform trans = attribute->Get();

                int foundIndex = -1;
                for(uint i = 0; i < children.size(); i++)
                {
                    QList<QtProperty *> properties = children[i]->subProperties();
                    for(uint j = 0; j < properties.size(); j++)
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
                    Real value = property->valueText().toFloat(&success);
                    if(!success)
                    {
                        ECEditorModule::LogError("Failed to convert the property value in float format.");
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
}
