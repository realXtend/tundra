#include "StableHeaders.h"
#include "ECAttributeEditor.h"
#include "AttributeInterface.h"

#include <QtDoublePropertyManager>
#include <QtDoubleSpinBoxFactory>
#include <QtVariantPropertyManager>
#include <QtBoolPropertyManager>
#include <QtCheckBoxFactory>
#include <QtProperty>

namespace ECEditor
{
    ECAttributeEditorInterface::ECAttributeEditorInterface(const QString &attributeName,
                       QtAbstractPropertyBrowser *owner,
                       QObject *parent):
        QObject(parent),
        owner_(owner),
        attributeName_(attributeName),
        property_(0),
        factory_(0),
        propertyMgr_(0),
        listenEditorChangedSignal_(false)
    {
        
    }

    ECAttributeEditorInterface::~ECAttributeEditorInterface()
    {
        SAFE_DELETE(property_)
        SAFE_DELETE(propertyMgr_)
        SAFE_DELETE(factory_)
    }

    template<> void ECAttributeEditor<Real>::InitializeEditor()
    {
        QtDoublePropertyManager *realPropertyManager = new QtDoublePropertyManager();//dynamic_cast<QtDoublePropertyManager *>(propertyMgr_);
        QtDoubleSpinBoxFactory *spinBoxFactory = new QtDoubleSpinBoxFactory();
        propertyMgr_ = realPropertyManager;
        factory_ = spinBoxFactory;
        property_ = realPropertyManager->addProperty(attributeName_);
        if(property_)
        {
            UpdateEditorValue();
            QObject::connect(propertyMgr_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(SendNewAttributeValue(QtProperty*)));
        }
        owner_->setFactoryForManager(realPropertyManager, spinBoxFactory);
        owner_->addProperty(property_);
    }

    template<> void ECAttributeEditor<Real>::UpdateEditorValue()
    {
        QtDoublePropertyManager *realPropertyManager = dynamic_cast<QtDoublePropertyManager *>(propertyMgr_);
        if(property_ && attribute_)
        {
            realPropertyManager->setValue(property_, attribute_->Get());
        }
    }

    template<> virtual void ECAttributeEditor<Real>::SendNewValueToAttribute(QtProperty *property)
    {
        if(listenEditorChangedSignal_)
        {
            Real newValue = ParseString<Real>(property->valueText().toStdString());
            SetValue(newValue);
        }
    }

    template<> void ECAttributeEditor<bool>::InitializeEditor()
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

    template<> void ECAttributeEditor<bool>::UpdateEditorValue()
    {
        QtVariantPropertyManager *boolPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(property_ && attribute_)
        {
            boolPropertyManager->setValue(property_, attribute_->Get());
        }
    }

    template<> virtual void ECAttributeEditor<bool>::SendNewValueToAttribute(QtProperty *property)
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

    template<> void ECAttributeEditor<int>::UpdateEditorValue()
    {
        QtVariantPropertyManager *intPropertyManager = dynamic_cast<QtVariantPropertyManager *>(propertyMgr_);
        if(property_ && attribute_)
        {
            intPropertyManager->setValue(property_, attribute_->Get());
        }
    }

    template<> virtual void ECAttributeEditor<int>::SendNewValueToAttribute(QtProperty *property)
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

    template<> virtual void ECAttributeEditor<Vector3df>::SendNewValueToAttribute(QtProperty *property)
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

    template<> virtual void ECAttributeEditor<Color>::SendNewValueToAttribute(QtProperty *property)
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
}