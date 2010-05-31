
#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MultiEditPropertyManager.h"

#include "MemoryLeakCheck.h"


namespace ECEditor
{
    QString MultiEditPropertyManager::Value(const QtProperty *property) const
    {
        if(!values_.contains(property))
            return "";

        QString values = values_[property].value;
        return values;
    }

    QStringList MultiEditPropertyManager::AttributeValue(const QtProperty *property) const
    {
        QStringList attributes;
        if(!values_.contains(property))
            return attributes;

        return values_[property].attributeValues_;
    }

    void MultiEditPropertyManager::SetValue(QtProperty *property, const QString &value)
    {
        if(!values_.contains(property)) 
            return;
        if(values_[property].value == value)
            return;

        values_[property].value = value;
        emit ValueChanged(property, value);
    }

    void MultiEditPropertyManager::SetAttributeValues(QtProperty *property, const QStringList &attributes)
    {
        if(!values_.contains(property))
            return;
        if(values_[property].attributeValues_ == attributes)
            return;

        values_[property].attributeValues_ = attributes;
        emit AttributeValuesUpdated(property, attributes);
        property->setModified(true);
    }

    QString MultiEditPropertyManager::valueText(const QtProperty *property) const
    {
        if(!values_.contains(property))
            return "(0 values)";

        // Hackish way to get change shown on editor window.
        const_cast<QtProperty *>(property)->setModified(false);
        if(values_[property].value != "")
            return values_[property].value;
        else if(values_[property].attributeValues_.size() > 0)
        {
            QString value = QString("(%1 values)").arg(values_[property].attributeValues_.size());
            /*for(uint i = 0; i < values_[property].attributeValues_.size(); i++)
            {
                value += values_[property].attributeValues_[i] + QString(" ");
            }*/
            return value;
        }
        return "(0 values)";
    }

    void MultiEditPropertyManager::initializeProperty(QtProperty *property)
    {
        Data data = Data();
        data.value = QString("");
        data.attributeValues_ = QStringList();
        values_[property] = data;
    }
    
    void MultiEditPropertyManager::uninitializeProperty(QtProperty *property)
    {
        values_.remove(property);
        QtAbstractPropertyManager::uninitializeProperty(property);
    }

    //Remove below.

    ExpandedVariantPropertyManager::ExpandedVariantPropertyManager(QObject *parent)
        : QtVariantPropertyManager(parent)
    {
        /*connect(this, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
                    this, SLOT(slotValueChanged(QtProperty *, const QVariant &)));
        connect(this, SIGNAL(propertyDestroyed(QtProperty *)),
                    this, SLOT(slotPropertyDestroyed(QtProperty *)));*/
    }

    ExpandedVariantPropertyManager::~ExpandedVariantPropertyManager()
    {

    }

    QVariant ExpandedVariantPropertyManager::value(const QtProperty *property) const
    {
        if (propertyToVariant_.contains(property))
            return propertyToVariant_[property];
        return QtVariantPropertyManager::value(property);
    }

    int ExpandedVariantPropertyManager::valueType(int propertyType) const
    {
        if (propertyType == ColorTypeId())
            return ColorTypeId();
        return QtVariantPropertyManager::valueType(propertyType);
    }

    bool ExpandedVariantPropertyManager::isPropertyTypeSupported(int propertyType) const
    {
        if (propertyType == ColorTypeId())
            return true;
        return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
    }

    QString ExpandedVariantPropertyManager::valueText(const QtProperty *property) const
    {
        /*if (propertyToVariant_.contains(property)) 
        {
            QString text;
            if(propertyToVariant_[property].userType() == ColorTypeId())
            {
                QVariant value = propertyToVariant_[property];
                Color color = qVariantValue<Color>(value);
                text = QString(tr("%1, %2, %3, %4").arg(QString::number(color.r)).arg(QString::number(color.b)).arg(color.g).arg(color.a));
            }
            return text;
        }*/
        return QtVariantPropertyManager::valueText(property);
    }

    int ExpandedVariantPropertyManager::ColorTypeId()
    {
        return qMetaTypeId<Color>();
    }

    void ExpandedVariantPropertyManager::setValue(QtProperty *property, const QVariant &value)
    {
        /*const QtProperty *parrent = 0;
        if(propertyToVariant_.contains(property)) 
        {
            parrent = property;
        }
        else
        {
            ParrentChilrenMap::const_iterator iter = parrentChildrenMap_.constBegin();
            while(iter != parrentChildrenMap_.constEnd())
            {
                if(iter->contains(property))
                {
                    parrent = iter.key();
                    break;
                }
                iter++;
            }
        }

        if (value.userType() == ColorTypeId()) //&& !value.canConvert(ColorTypeId()))
        {
            Color color = qVariantValue<Color>(value);
            propertyToVariant_[property] = value;//.setValue<Color>(color);
            QList<QtProperty*> children = property->subProperties();
            if(children.size() == 4)
            {
                QtVariantProperty *variant = dynamic_cast<QtVariantProperty *>(children[0]);
                variant->setValue(color.r);
                variant = dynamic_cast<QtVariantProperty *>(children[1]);
                variant->setValue(color.b);
                variant = dynamic_cast<QtVariantProperty *>(children[2]);
                variant->setValue(color.g);
                variant = dynamic_cast<QtVariantProperty *>(children[3]);
                variant->setValue(color.a);
            }

            emit propertyChanged(property);
            emit valueChanged(property, value);
            return;
        }*/

        QtVariantPropertyManager::setValue(property, value);
    }

    void ExpandedVariantPropertyManager::initializeProperty(QtProperty *property)
    {
        /*if(propertyType(property) == ColorTypeId())
        {
            Color color;

            QtVariantProperty *item = addProperty(QVariant::Double, "Red");
            property->addSubProperty(item);
            parrentChildrenMap_[property].insert(item);

            item = addProperty(QVariant::Double, "Blue");
            property->addSubProperty(item);
            parrentChildrenMap_[property].insert(item);

            item = addProperty(QVariant::Double, "Green");
            property->addSubProperty(item);
            parrentChildrenMap_[property].insert(item);

            item = addProperty(QVariant::Double, "Alfa");
            property->addSubProperty(item);
            parrentChildrenMap_[property].insert(item);

            QVariant value = QVariant::fromValue<Color>(color);
            propertyToVariant_[property] = value;
        }*/
        QtVariantPropertyManager::initializeProperty(property);
    }

    void ExpandedVariantPropertyManager::uninitializeProperty(QtProperty *property)
    {
        /*if(propertyToVariant_.contains(property)) 
        {
            propertyToVariant_.remove(property);
            parrentChildrenMap_.remove(property);
        }*/
        QtVariantPropertyManager::uninitializeProperty(property);
    }
}