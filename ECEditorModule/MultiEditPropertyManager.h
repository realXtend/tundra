#ifndef incl_ECEditorModule_MultiEditPropertyManager_h
#define incl_ECEditorModule_MultiEditPropertyManager_h

#define QT_QTPROPERTYBROWSER_IMPORT

#include <QtVariantPropertyManager>
#include "Color.h"
#include <map>

Q_DECLARE_METATYPE(Color)

namespace ECEditor
{
    class MultiEditPropertyManager: public QtAbstractPropertyManager
    {
        Q_OBJECT
    public:
        MultiEditPropertyManager(QObject *parent = 0): QtAbstractPropertyManager(parent){}
        ~MultiEditPropertyManager(){}

        QString Value(QtProperty *property) const;
        QStringList AttributeValue(QtProperty *property) const;

    public slots:
        void SetValue(QtProperty *property, const QString &value);
        void SetAttributeValues(QtProperty *property, const QStringList &attributes);

    signals:
        void ValueChanged(QtProperty *property, const QString &value);
        void AttributeValuesUpdated(QtProperty *property, const QStringList &attributes);

    protected:
        QString valueText(QtProperty *property) const;
        void initializeProperty(QtProperty *property);
        void uninitializeProperty(QtProperty *property);

    private:
        struct Data
        {
            QString value;
            QStringList attributeValues_;
        };

        typedef QMap<const QtProperty *, Data> PropertyToDataMap;
        PropertyToDataMap values_;
    };

    class ExpandedVariantPropertyManager : public QtVariantPropertyManager
    {
        Q_OBJECT
    public:
        ExpandedVariantPropertyManager(QObject *parent = 0);
        ~ExpandedVariantPropertyManager();

        virtual QVariant value(const QtProperty *property) const;
        virtual int valueType(int propertyType) const;
        virtual bool isPropertyTypeSupported(int propertyType) const;

        QString valueText(const QtProperty *property) const;

        //! Get metadata id for custom color QVariant type.
        static int ColorTypeId();

    public slots:
        virtual void setValue(QtProperty *property, const QVariant &val);

    protected:
        virtual void initializeProperty(QtProperty *property);
        virtual void uninitializeProperty(QtProperty *property);

    private:
        QMap<const QtProperty *, QVariant> propertyToVariant_;
        typedef QMap<const QtProperty *, QSet<QtProperty *>> ParrentChilrenMap;
        ParrentChilrenMap parrentChildrenMap_;
    };
}

#endif