// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECAttributeEditor_h
#define incl_ECEditorModule_ECAttributeEditor_h

#include "Vector3D.h"
#include "IComponent.h"
#include "Transform.h"
#include "AssetReference.h"
#include "AssetFwd.h"
#include "MultiEditPropertyManager.h"
#include "MultiEditPropertyFactory.h"

#include <QObject>
#include <QVariant>
#include <QSize>
#include <QPoint>

#include <map>

class QtDoublePropertyManager;
class QtVariantPropertyManager;
class QtProperty;
class QtAbstractPropertyManager;
class QtAbstractEditorFactoryBase;
class QtAbstractPropertyBrowser;

class Framework;
class Color;

typedef unsigned char MetaDataFlag;
enum MetaDataFlags
{
    None             = 0,
    UsingEnums       = 1 << 0,
    UsingMaxValue    = 1 << 1,
    UsingMinValue    = 1 << 2,
    UsingStepValue   = 1 << 3,
    UsingDescription = 1 << 4
};

/// Abstract base class for attribute editing.
/** Class is responsible to listen attribute changed signals and update editor's state based on those changes.
    @todo Remove QtAbstractPropertyBrowser pointer from the attribute editor, this means that manager and factory connections need to 
    be registered in elsewhere eg. inside the ECComponentEditor's addAttribute mehtod.
    @ingroup ECEditorModuleClient.
*/
class ECAttributeEditorBase: public QObject
{
    Q_OBJECT

public:
    ECAttributeEditorBase(QtAbstractPropertyBrowser *owner,
                          ComponentPtr component,
                          const QString &name,
                          const QString &type,
                          QObject *parent = 0);

    virtual ~ECAttributeEditorBase();

    /// Get attribute name.
    /// @return attribute type name.
    QString GetAttributeName() const { return name_; }

    /// Get attribute type. If any attributes were not found, return empty string.
    QString GetAttributeType() const { return typeName_; };

    /// Get editor's root property.
    /// @return editor's root property pointer.
    QtProperty *GetProperty() const { return rootProperty_; }

    /// Check if this editor's manager contains this spesific property.
    /// @param property Property that we are looking for.
    bool ContainsProperty(QtProperty *property) const; 

    /// Updates editor's ui elements to fit new attribute values
    /** If different component's attribute values differ from each other,
        the editor begin to use multiedit mode and editor need to create new ui elements.
    */
    void UpdateEditorUI(IAttribute *attr = 0);

public slots:
    void AddComponent(ComponentPtr component);
    void RemoveComponent(ComponentPtr component);
    bool HasComponent(ComponentPtr component);
    void AttributeChanged(IAttribute* attribute);

signals:
    /// Signal is emmitted when editor has been reinitialized.
    /** @param name Attribute name.
     */
    void EditorChanged(const QString &name);
    void OnComponentAdded(QtProperty*, IComponent *comp);
    void OnComponentRemoved(QtProperty*, IComponent *comp);

private slots:
    /// Called when user has picked one of the multiselect values.
    /** @param value new value that has been picked.
    */
    void MultiEditValueSelected(const QString &value);

    /// Listens if any of editor's values has been changed and the value change need to forward to the a attribute.
    void PropertyChanged(QtProperty *property) { Set(property); }

protected:
    /// Initialize attribute editor's ui elements.
    virtual void Initialize() = 0;

    /// Read current value from the ui and set it to IAttribute.
    virtual void Set(QtProperty *property) = 0;

    /// Read attribute value from IAttribute and set it to ui.
    virtual void Update(IAttribute *attr = 0) = 0;

    /// Checks if all components are holding the same attribute value.
    /** @return true if all attribute values are same, false otherwise. */
    virtual bool HasIdenticalAttributes() const = 0;

    void CleanExpiredComponents();

    /// PreInitialize should be called before the Initialize.
    void PreInitialize();

    /// Delete property manager and its factory.
    void UnInitialize();

    /// Try to find attribute in component and if found return it's pointer.
    IAttribute *FindAttribute(ComponentPtr component) const;
    QList<ComponentWeakPtr>::iterator FindComponent(ComponentPtr component);

    QtAbstractPropertyBrowser *owner_;
    QtAbstractPropertyManager *propertyMgr_;
    QtAbstractEditorFactoryBase *factory_;
    std::vector<QtAbstractPropertyManager*> optionalPropertyManagers_;
    std::vector<QtAbstractEditorFactoryBase*> optionalPropertyFactories_;
    QtProperty *rootProperty_;
    QString name_;
    QString typeName_;
    bool listenEditorChangedSignal_;
    typedef QList<ComponentWeakPtr> ComponentWeakPtrList;
    ComponentWeakPtrList components_;
    bool useMultiEditor_;
    MetaDataFlag metaDataFlag_;
};

/// Implements attribute editor UI elements for attribute type of type @c T and forwards attribute changes to IAttribute objects.
/** Support editing of multiple attributes at the same time and extra attribute objects can be passed using a AddComponent method,
    removing editable components can be done via RemoveComponent method.
    To add a new attribute types to the ECEditor you need to define following methods:
     - Initialize: For intializing all ui elements for the editor. In this method the spesific QtPropertyManagers and QtPropertyFactories
       are created and registered to QtAbstractPropertyBrowser object. QtPropertyManager is used to create a new QtProperties. More info can be
       found at QtPropertyBrowser documentation.
     - Set: Setter method that will get editor's value from QtProperty object and pass it to IAttribute.
     - Update: Getter method that will read value from the IAttribute and pass it to Editor using QtProperty's setValue method.
*/
template<typename T>
class ECAttributeEditor : public ECAttributeEditorBase
{
public:
    ECAttributeEditor(QtAbstractPropertyBrowser *owner,
                      ComponentPtr component,
                      const QString &name,
                      const QString &type,
                      QObject *parent = 0) :
        ECAttributeEditorBase(owner, component, name, type, parent)
    {
        listenEditorChangedSignal_ = true;
    }

    ~ECAttributeEditor() {}

protected:
    /// Sends a new value to each component and emit AttributeChanged signal.
    /** @param value_ new value that is sended over to component.
    */
    void SetValue(const T &value);

    /// ECAttributeEditorBase override.
    virtual void Initialize();

    /// ECAttributeEditorBase override.
    virtual void Update(IAttribute *attr = 0);

private:
    /// ECAttributeEditorBase override.
    virtual void Set(QtProperty *property);

    /// ECAttributeEditorBase override.
    /// @todo If some of the components have expired this method will return false. this should get fixed in some way.
    bool HasIdenticalAttributes() const;

    /// Create multiedit property manager and factory if listenEditorChangedSignal_ flag is rised.
    void InitializeMultiEditor();

    /// Get each components attribute value and convert it to a string value and put that value in a string vector.
    /// @todo Optimize this function.
    void UpdateMultiEditorValue(IAttribute *attribute = 0);
};

#include "ECAttributeEditor.inl"

template<> void ECAttributeEditor<float>::Update(IAttribute *attr);
template<> void ECAttributeEditor<float>::Initialize();
template<> void ECAttributeEditor<float>::Set(QtProperty *property);

template<> void ECAttributeEditor<int>::Update(IAttribute *attr);
template<> void ECAttributeEditor<int>::Initialize();
template<> void ECAttributeEditor<int>::Set(QtProperty *property);

template<> void ECAttributeEditor<bool>::Update(IAttribute *attr);
template<> void ECAttributeEditor<bool>::Initialize();
template<> void ECAttributeEditor<bool>::Set(QtProperty *property);

template<> void ECAttributeEditor<Vector3df>::Update(IAttribute *attr);
template<> void ECAttributeEditor<Vector3df>::Initialize();
template<> void ECAttributeEditor<Vector3df>::Set(QtProperty *property);

template<> void ECAttributeEditor<Color>::Update(IAttribute *attr);
template<> void ECAttributeEditor<Color>::Initialize();
template<> void ECAttributeEditor<Color>::Set(QtProperty *property);

template<> void ECAttributeEditor<QSize>::Update(IAttribute *attr);
template<> void ECAttributeEditor<QSize>::Initialize();
template<> void ECAttributeEditor<QSize>::Set(QtProperty *property);

template<> void ECAttributeEditor<QSizeF>::Update(IAttribute *attr);
template<> void ECAttributeEditor<QSizeF>::Initialize();
template<> void ECAttributeEditor<QSizeF>::Set(QtProperty *property);

template<> void ECAttributeEditor<QPoint>::Update(IAttribute *attr);
template<> void ECAttributeEditor<QPoint>::Initialize();
template<> void ECAttributeEditor<QPoint>::Set(QtProperty *property);

template<> void ECAttributeEditor<QPointF>::Update(IAttribute *attr);
template<> void ECAttributeEditor<QPointF>::Initialize();
template<> void ECAttributeEditor<QPointF>::Set(QtProperty *property);

template<> void ECAttributeEditor<QString>::Update(IAttribute *attr);
template<> void ECAttributeEditor<QString>::Initialize();
template<> void ECAttributeEditor<QString>::Set(QtProperty *property);

template<> void ECAttributeEditor<Transform>::Update(IAttribute *attr);
template<> void ECAttributeEditor<Transform>::Initialize();
template<> void ECAttributeEditor<Transform>::Set(QtProperty *property);

template<> void ECAttributeEditor<QVariant>::Update(IAttribute *attr);
template<> void ECAttributeEditor<QVariant>::Initialize();
template<> void ECAttributeEditor<QVariant>::Set(QtProperty *property);

template<> void ECAttributeEditor<QVariantList>::Update(IAttribute *attr);
template<> void ECAttributeEditor<QVariantList>::Initialize();
template<> void ECAttributeEditor<QVariantList>::Set(QtProperty *property);

template<> void ECAttributeEditor<AssetReference>::Update(IAttribute *attr);
template<> void ECAttributeEditor<AssetReference>::Initialize();
template<> void ECAttributeEditor<AssetReference>::Set(QtProperty *property);

class LineEditWithButtons;

/// Special case editor for AssetReference attributes.
class AssetReferenceAttributeEditor : public ECAttributeEditor<AssetReference>
{
    Q_OBJECT

public:
    AssetReferenceAttributeEditor(QtAbstractPropertyBrowser *owner, ComponentPtr component,
        const QString &name, const QString &type, QObject *parent = 0) :
        ECAttributeEditor<AssetReference>(owner, component, name, type, parent),
        fw(component->GetFramework())
    {
    }

private slots:
    void OpenAssetsWindow();
    void HandleNewEditor(QtProperty *prop, LineEditWithButtons *);
    void HandleAssetPicked(AssetPtr asset);
    void RestoreOriginalValue();
    void OpenEditor();

private:
    Framework *fw;
    QString originalRef;
};

template<> void ECAttributeEditor<AssetReferenceList>::Update(IAttribute *attr);
template<> void ECAttributeEditor<AssetReferenceList>::Initialize();
template<> void ECAttributeEditor<AssetReferenceList>::Set(QtProperty *property);

/// Special case editor for AssetReferenceList attributes.
class AssetReferenceListAttributeEditor : public ECAttributeEditor<AssetReferenceList>
{
    Q_OBJECT

public:
    AssetReferenceListAttributeEditor(QtAbstractPropertyBrowser *owner, ComponentPtr component,
        const QString &name, const QString &type, QObject *parent = 0) :
        ECAttributeEditor<AssetReferenceList>(owner, component, name, type, parent),
        fw(component->GetFramework())
    {
    }

private:
    int currentIndex;

private slots:
    void OpenAssetsWindow();
    void HandleNewEditor(QtProperty *prop, LineEditWithButtons *);
    void HandleAssetPicked(AssetPtr asset);
    void RestoreOriginalValue();
    void OpenEditor();

private:
    Framework *fw;
    QString originalRef;
};

#endif
