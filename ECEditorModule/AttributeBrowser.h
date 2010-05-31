// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_AttributePropertyBrowser_h
#define incl_ECEditorModule_AttributePropertyBrowser_h

#include <QWidget>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class QtTreePropertyBrowser;
class QtGroupPropertyManager;

namespace Foundation
{
    class AttributeInterface;
    class ComponentInterface;
    typedef boost::shared_ptr<ComponentInterface> ComponentInterfacePtr;
    typedef boost::weak_ptr<ComponentInterface> ComponentWeakPtr;
}

namespace ECEditor
{
    class ECAttributeEditorBase;
    class ECComponentEditor;
    typedef std::vector<ECComponentEditor*> ComponentEditorVector;

    class AttributeBrowser : public QWidget
    {
        Q_OBJECT
    public:
        AttributeBrowser(QWidget *parent = 0);
        ~AttributeBrowser();

        void AddEntityComponents(std::vector<Foundation::ComponentInterfacePtr> entityComponents);
        QtTreePropertyBrowser *GetPropertyBrowser() const { return propertyBrowser_; }

    public slots:
        void RefreshAttributeComponents();
        void ClearBrowser();

    signals:
        void AttributesChanged();

    private slots:
        //! Remove component editor from the map if it's emitted destoyed signal.
        void ComponentEditorDestoryed(QObject * obj = 0);

    private:
        void InitializeEditor();

        typedef std::map<std::string, std::vector<Foundation::ComponentWeakPtr>> EntityComponentMap;
        EntityComponentMap entityComponents_;
        typedef std::map<std::string, ECComponentEditor *> ComponentEditorMap;
        ComponentEditorMap componentEditors_;
        QtTreePropertyBrowser *propertyBrowser_;
    };
}

#endif