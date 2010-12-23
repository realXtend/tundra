// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECComponentEditor_h
#define incl_ECEditorModule_ECComponentEditor_h

#include "ForwardDefines.h"
#include "AttributeChangeType.h"

#include <QObject>
#include <QMap>
#include <QList>

class QtAbstractPropertyBrowser;
class QtProperty;
class QtGroupPropertyManager;

class ECAttributeEditorBase;

//! ECComponentEditor is responsible to create the all attribute editors for each component (Note! every component must contain exatly the same attributes).
/*! If the attribute type is supported in ECAttributeEditor component editor will create a new instance of it and store it to a list.
 *  \ingroup ECEditorModuleClient.
 */
class ECComponentEditor : public QObject
{
    Q_OBJECT
public:
    ECComponentEditor(ComponentPtr component, QtAbstractPropertyBrowser *propertyBrowser);
    virtual ~ECComponentEditor();

    //! Check if this component editor is holding spesific property as it's root property.
    bool ContainProperty(QtProperty *property) const;

    //! Get group property pointer.
    QtProperty *RootProperty() const {return groupProperty_;}

    //! Add new component to the editor.
    void AddNewComponent(ComponentPtr component);

    //! Remove component from the editor.
    void RemoveComponent(ComponentPtr component);

    void UpdateUi();

    //! Return attribute type for given name.
    QString GetAttributeType(const QString &name) const;

public slots:
    //! If ECAttributeEditor has been reinitialized ComponentEditor need to add new QProperty to it's GroupProperty.
    //! This ensures that newly createated attribute eidtor will get displayed on the ECBrowser.
    void OnEditorChanged(const QString &name);

private:
    //! Factory method that is trying to find the right attribute type by using a dynamic_cast and if the attribute is succefully casted 
    //! new ECAttributeEditor instance is created and returned to user.
    //! @return ECAttributeEditor object is returned if attribute type is supported by the ECEditor. Incase attribute type
    //! isn't supported this method will return a null pointer.
    static ECAttributeEditorBase *CreateAttributeEditor(QtAbstractPropertyBrowser *browser,
                                                        ECComponentEditor *editor,
                                                        ComponentPtr component,
                                                        const QString &name,
                                                        const QString &type);

    //! Initialize component editor and create attribute editors.
    //! @param component component is used to figure out what attrubtes it contain and what
    //! attribute editors need to get created to this component editor.
    void InitializeEditor(ComponentPtr component);

    //! Create new attribute editors for spesific component.
    //! @param component Compoent that we need to use, to get all attributes that we want to edit.
    void CreateAttributeEditors(ComponentPtr component);

    void UpdateGroupPropertyText();

    typedef QMap<QString, ECAttributeEditorBase*> AttributeEditorMap;
    AttributeEditorMap attributeEditors_;
    typedef QList<ComponentWeakPtr> ComponentSet;
    ComponentSet components_;
    QtProperty *groupProperty_;
    QtGroupPropertyManager *groupPropertyManager_;
    QtAbstractPropertyBrowser *propertyBrowser_;
    QString typeName_;
    QString name_;
};

#endif