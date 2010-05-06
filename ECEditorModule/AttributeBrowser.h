// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_AttributePropertyBrowser_h
#define incl_ECEditorModule_AttributePropertyBrowser_h

#include <QWidget>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class QtTreePropertyBrowser;

namespace Foundation
{
    class AttributeInterface;
    class ComponentInterface;
    typedef boost::shared_ptr<ComponentInterface> ComponentInterfacePtr;
    typedef boost::weak_ptr<ComponentInterface> ComponentWeakPtr;
}

namespace ECEditor
{
    class ECAttributeEditorInterface;

    class AttributeBrowser : public QWidget
    {
        Q_OBJECT
    public:
        AttributeBrowser(QWidget *parent = 0);
        ~AttributeBrowser();

        void AddEntityComponent(Foundation::ComponentInterfacePtr entityComponent);
        void RemoveEntityComponent(Foundation::ComponentInterfacePtr entityComponent);
        QtTreePropertyBrowser *GetPropertyBroweser(){return propertyBrowser_;}

    public slots:
        void RefreshAttributeComponents();
        void ClearBrowser();

    signals:
        void AttributesChanged();

    private:
        void InitializeEditor();
        void AddNewAttribute(Foundation::AttributeInterface *attribute);
        ECAttributeEditorInterface *CreateAttributeEditor(Foundation::AttributeInterface &attribute);

        typedef std::map<QString, Foundation::ComponentWeakPtr> EntityComponentMap;
        EntityComponentMap SelectedEntityComponents_;
        typedef std::map<QString, ECAttributeEditorInterface*> AttributeEditorMap;
        AttributeEditorMap attributes_;
        QtTreePropertyBrowser *propertyBrowser_;
    };
}

#endif