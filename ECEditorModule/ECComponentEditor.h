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

    //! ECComponentEditor is responsible to create the all attribute editors for each component (Note! each component must contain same attributes).
    /*! ECComponentEditor will only accept same type of components. When the object is first time initialized, user need tell the typename
     *  of the component that we want to edit. ECComponentEditor is holding a GetAttributeEditor static method switch will check if 
     *  component's attribute implementation is supported in ECAttributeEditor class. If the attribute is supported, mehtod will create a new 
     *  ECAttributeEditor instance for that attribute type.
     *  \ingroup ECEditorModuleClient.
     */
    class ECComponentEditor: public QObject
    {
        Q_OBJECT
    public:
        ECComponentEditor(Foundation::ComponentInterfacePtr component, QtAbstractPropertyBrowser *propertyBrowser);
        virtual ~ECComponentEditor();

        //! Check if this component editor is holding spesific property as it's root property.
        bool ContainProperty(QtProperty *property) const;

        //! Get group property pointer.
        QtProperty *RootProperty() const {return groupProperty_;}

        int ComponentsCount() const { return components_.size(); }

        int AttributeCount() const { return attributeEditors_.size(); }

        //! Add new component into the editor.
        void AddNewComponent(Foundation::ComponentInterfacePtr component, bool updateUi = true);

        //! Remove component from the editor.
        void RemoveComponent(Foundation::ComponentInterface *component);

        void UpdateEditorUI();

    public slots:
        //! W
        //! \bug When user reinitialize the attribute editor's properties the order will go invalid.
        void AttributeEditorUpdated(const std::string &attributeName);

    private slots:
        //! When component's state has been changed, this method is called.
        //! Method will check what component's attribute value has been changed
        //! and will ask the ECAttributeEditor to update it's fields to new attribute valeus (UpdateEditorUI).
        void ComponentChanged();

    private:
        //! Method is trying to find the right attribute type by using a dynamic_cast and if attribute is succefully casted 
        //! a new ECAttributeEditor instance is created and it's pointer returned to a user. If attribute type is not supported
        //! the method will return a null pointer.
        //! @return return attribute pointer if attribute type is supported and if not return null pointer.
        static ECAttributeEditorBase *CreateAttributeEditor( QtAbstractPropertyBrowser *browser, 
                                                             ECComponentEditor *editor, 
                                                             Foundation::AttributeInterface &attribute);

        //! Initialize editor and create attribute editors.
        //! @param component component is used to figure out what attrubtes it contain and what
        //! attribute editors need to be created to this component editor.
        void InitializeEditor(Foundation::ComponentInterfacePtr component);

        //! Create new attribute editors for spesific component.
        //! @param component Compoent that we need to use, to get all attributes that we want to edit.
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
        std::string                 name_;
    };
}

#endif