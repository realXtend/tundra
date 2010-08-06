
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
        emit ValueChanged(value);
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
}