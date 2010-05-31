// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECComponentEditor_h
#define incl_ECEditorModule_ECComponentEditor_h

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <QObject>
#include <map>
#include <vector>

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

    class ECComponentEditor: public QObject
    {
        Q_OBJECT
    public:
        ECComponentEditor(std::vector<Foundation::ComponentInterfacePtr> components, QtAbstractPropertyBrowser *propertyBrowser, QObject *parent = 0);
        virtual ~ECComponentEditor();

        void CreateECAttributes(Foundation::ComponentInterfacePtr components);

    public slots:
        //! Sometimes attribute editor need to recreate a property and we need to insert this new property into groupProperty.
        void AttributeEditorUpdated(const std::string &attributeName);

    signals:
        void AttributeChanged(const std::string &attributeName);

    private:
        //! Trying to find the right attribute type by doing a dynamic cast and if object is succefully casted 
        //! it will create a new ECAttributeEditor object and return it's pointer to user.
        //! @return return attribute pointer if attribute type is supported if not return null pointer.
        static ECAttributeEditorBase *GetAttributeEditor( QtAbstractPropertyBrowser *browser, 
                                                          ECComponentEditor *editor, 
                                                          const Foundation::AttributeInterface &attribute,
                                                          Foundation::ComponentInterfacePtr component );

        void InitializeEditor(std::vector<Foundation::ComponentInterfacePtr> components);

        typedef std::map<std::string, ECAttributeEditorBase*> AttributeEditorMap;
        AttributeEditorMap attributeEditors_;
        QtProperty *groupProperty_;
        QtGroupPropertyManager *groupPropertyManager_;
        QtAbstractPropertyBrowser *propertyBrowser_;
    };
}

#endif