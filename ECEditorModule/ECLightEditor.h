// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECLightEditor_h
#define incl_ECEditorModule_ECLightEditor_h

#include "CoreTypes.h"
#include "qtvariantproperty.h"
#include <QVector3D>
#include <QtProperty>
#include <QObject>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace Foundation
{
    class ComponentInterface;
    typedef boost::shared_ptr<ComponentInterface> ComponentInterfacePtr;
    typedef boost::weak_ptr<ComponentInterface> ComponentWeakPtr;
}

namespace ECEditor
{
    class AbstractECEditor : public QObject
    {
        Q_OBJECT
    public:
        explicit AbstractECEditor(QtVariantPropertyManager *propertyManager, Foundation::ComponentInterfacePtr component);
        virtual ~AbstractECEditor();

        QtVariantProperty *GetRootProperty() const;
        bool ContainProperty(QtProperty *property) const;
        void SetPropertyValue(QtProperty *property);

    public slots:
        virtual void UpdateProperties() = 0;

    protected:
        virtual void Initialize() = 0;
        virtual void SetValue(QtProperty *property) = 0;

        QtVariantProperty *rootProperty_;
        typedef QMap<QString, QtVariantProperty *> VariantPropertyMap;
        VariantPropertyMap propertyMap_;
        QtVariantPropertyManager *variantManager_;
        Foundation::ComponentWeakPtr componentInterface_;
    };

    class ECLightEditor : public AbstractECEditor
    {
        Q_OBJECT
    public:
        ECLightEditor(QtVariantPropertyManager *propertyManager, Foundation::ComponentInterfacePtr component);
        ~ECLightEditor();

    public slots:
        virtual void UpdateProperties();

    private:
        void Initialize();
        virtual void SetValue(QtProperty *property);
    };
}

#endif