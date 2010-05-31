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

        //! add entity components that we want to edit with attribute browser.
        //! @param entityComponents list of components that we want to edit.
        void AddEntityComponents(std::vector<Foundation::ComponentInterfacePtr> entityComponents);

        //! Returns QtTreePropertyBrowser object.
        //! @return property browser pointer.
        QtTreePropertyBrowser *GetPropertyBrowser() const { return propertyBrowser_; }

    public slots:
        //! Clear property browser and reinitialize all components and their attributes. 
        //! Note! This method take some time cause all tree property browser's elements are reintialized (use this only when absolutely necessary).
        void RedrawBrowserUi();

        //! Clears all entity components from this objects map and clears the all property browser's elements.
        void ClearBrowser();

    signals:
        //! Some of this browser's attribute has been changed by user.
        //! @param attributeName attribute name that has been changed.
        void AttributesChanged(const std::string &attributeName);

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