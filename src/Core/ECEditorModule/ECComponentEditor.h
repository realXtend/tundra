// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "SceneFwd.h"
#include "AttributeChangeType.h"

#include <QObject>
#include <QMap>
#include <QList>

class QtAbstractPropertyBrowser;
class QtProperty;
class QtGroupPropertyManager;

class ECAttributeEditorBase;

/// Responsible for creating all attribute editors for each component.
/** @note Every component must contain exatly the same attributes.
    If the attribute type is supported in ECAttributeEditor component editor will create a new instance of it and store it to a list.
    @ingroup ECEditorModuleClient. */
class ECComponentEditor : public QObject
{
    Q_OBJECT

public:
    ECComponentEditor(ComponentPtr component, QtAbstractPropertyBrowser *propertyBrowser);
    virtual ~ECComponentEditor();

    /// Check if this component editor is holding spesific property as it's root property.
    bool ContainProperty(QtProperty *property) const;

    /// Get group property pointer.
    QtProperty *RootProperty() const {return groupProperty_;}

    /// Add new component to the editor.
    void AddNewComponent(ComponentPtr component);

    /// Remove component from the editor.
    void RemoveComponent(ComponentPtr component);

    /// Removes attribute from the editor.
    /** Needed for dynamic components. Updates the UI right away. */
    void RemoveAttribute(ComponentPtr comp, IAttribute *attr);

    /// Return attribute type for given name.
    QString GetAttributeType(const QString &name) const;

public slots:
    /// Updates the UI.
    void UpdateUi();

    /// If ECAttributeEditor has been reinitialized ComponentEditor need to add new QProperty to it's GroupProperty.
    /// This ensures that newly createated attribute eidtor will get displayed on the ECBrowser.
    void OnEditorChanged(const QString &name);

signals:
    void AttributeAboutToBeEdited(IAttribute * attr);

private:
    /// Factory method for creating attribute editors.
    static ECAttributeEditorBase *CreateAttributeEditor(QtAbstractPropertyBrowser *browser, ECComponentEditor *editor, IAttribute *attribute);

    /// Initialize component editor and create attribute editors.
    /// @param component component is used to figure out what attributes it contain and what
    /// attribute editors need to get created to this component editor.
    void InitializeEditor(ComponentPtr component);

    /// Create new attribute editors for spesific component.
    /// @param component Compoent that we need to use, to get all attributes that we want to edit.
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
