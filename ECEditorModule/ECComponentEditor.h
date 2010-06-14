// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECComponentEditor_h
#define incl_ECEditorModule_ECComponentEditor_h

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <QObject>
#include <map>
#include <set>

class QtAbstractPropertyBrowser;
class QtProperty;
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

    //! ECComponentEditor get components as it input and it is responsible to create all attribute editors for that compoennt.
    /*! ECComponentEditor will only accept same type of component. When object is first time initialized user need tell the typename
     *  of the component that we want to edit. ECComponentEditor is holding a GetAttributeEditor static method switch will check if 
     *  component's attribute is holding a implementation for that spesific attriubte type (ECAttributeEditor) if attribute is supported
     *  mehtod will create a new attriubte editor and return it's pointer to user.
     *  \ingroup ECEditorModuleClient.
     */
    class ECComponentEditor: public QObject
    {
        Q_OBJECT
    public:
        ECComponentEditor(Foundation::ComponentInterfacePtr component, const std::string &typeName, QtAbstractPropertyBrowser *propertyBrowser);
        virtual ~ECComponentEditor();

        //! Check if this component editor is holding spesific property inside it's root property.
        bool ContainProperty(QtProperty *property) const;

        //! Get group property pointer.
        QtProperty *GetRootProperty() const {return groupProperty_;}

        int NumberOfComponents() const { return components_.size(); }

        //! Insert new component to this editor.
        void AddNewComponent(Foundation::ComponentInterfacePtr component);

        //! Remove component from editor's map and update attribute eidtors.
        void RemoveComponent(Foundation::ComponentInterfacePtr component);

    public slots:
        //! Sometimes attribute editor need to recreate it's property and we need to insert this property into same location as previous the one was located.
        //! \bug When user reinitialize the attribute editor's properties the order will go invalid.
        void AttributeEditorUpdated(const std::string &attributeName);

    signals:
        //! Attribute editor has changed attribute value.
        void AttributeChanged(const std::string &attributeName);

        //! Emit a signal when component is removed from the editor's set.
        void ComponentRemoved(Foundation::ComponentInterface *component);

    private:
        //! Trying to find the right attribute type by using a dynamic_cast and if object is succefully casted 
        //! new ECAttributeEditor is created and it's pointer returned. If attribute type is not supported return a null pointer.
        //! @return return attribute pointer if attribute type is supported if not return null pointer.
        static ECAttributeEditorBase *GetAttributeEditor( QtAbstractPropertyBrowser *browser, 
                                                          ECComponentEditor *editor, 
                                                          const Foundation::AttributeInterface &attribute,
                                                          Foundation::ComponentInterfacePtr component );

        //! Initialize editor and create attribute editors.
        //! @param component component is used to figure out what attrubtes it contain and what
        //! attribute editors need to be created to this component editor.
        void InitializeEditor(Foundation::ComponentInterfacePtr component);

        //! Create new attribute editors for spesific component.
        void CreateAttriubteEditors(Foundation::ComponentInterfacePtr component);

        void UpdateGroupPropertyText();

        typedef std::map<std::string, ECAttributeEditorBase*> AttributeEditorMap;
        AttributeEditorMap          attributeEditors_;
        typedef std::set<Foundation::ComponentWeakPtr> ComponentSet;
        ComponentSet                components_;
        QtProperty                  *groupProperty_;
        QtGroupPropertyManager      *groupPropertyManager_;
        QtAbstractPropertyBrowser   *propertyBrowser_;
        std::string                 typeName_;
    };
}

#endif