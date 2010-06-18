#ifndef incl_ECEditorModule_MultiEditPropertyManager_h
#define incl_ECEditorModule_MultiEditPropertyManager_h

#define QT_QTPROPERTYBROWSER_IMPORT

#include <QtVariantPropertyManager>
#include "Color.h"
#include <map>

Q_DECLARE_METATYPE(Color)

namespace ECEditor
{
    //! MultiEditorPropertyManager will listen if property value has been changed by user and it will inform them to a MultiEditPropertyFactory.
    /*! MultiEditorPropertyManager will create new properties and return their pointer to user. When user change that property parameters manager
     *  will inform the registered factory about those changes switch will update them to ui widget.
     *  \ingroup ECEditorModuleClient.
     */
    class MultiEditPropertyManager: public QtAbstractPropertyManager
    {
        Q_OBJECT
    public:
        MultiEditPropertyManager(QObject *parent = 0): QtAbstractPropertyManager(parent){}
        ~MultiEditPropertyManager(){}

        QString Value(const QtProperty *property) const;
        QStringList AttributeValue(const QtProperty *property) const;

    public slots:
        void SetValue(QtProperty *property, const QString &value);
        void SetAttributeValues(QtProperty *property, const QStringList &attributes);

    signals:
        void ValueChanged(const QtProperty *property, const QString &value);
        void AttributeValuesUpdated(const QtProperty *property, const QStringList &attributes);

    protected:
        virtual QString valueText(const QtProperty *property) const;
        virtual void initializeProperty(QtProperty *property);
        virtual void uninitializeProperty(QtProperty *property);

    private:
        struct Data
        {
            QString value;
            QStringList attributeValues_;
        };

        typedef QMap<const QtProperty *, Data> PropertyToDataMap;
        PropertyToDataMap values_;
    };
}

#endif