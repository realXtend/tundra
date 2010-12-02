// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECAttributeEditor_h
#define incl_ECEditorModule_ECAttributeEditor_h

#include "ForwardDefines.h"
#include "Vector3D.h"
#include "CoreStringUtils.h"
#include "IComponent.h"
#include "Transform.h"
#include "AssetReference.h"

#include "MultiEditPropertyManager.h"
#include "MultiEditPropertyFactory.h"

#include <QObject>
#include <map>

class QtDoublePropertyManager;
class QtVariantPropertyManager;
class QtProperty;
class QtAbstractPropertyManager;
class QtAbstractEditorFactoryBase;
class QtAbstractPropertyBrowser;

class Color;
struct AssetReference;
struct Transform;

class IComponent;
typedef boost::shared_ptr<IComponent> ComponentPtr;
typedef boost::weak_ptr<IComponent> ComponentWeakPtr;

namespace ECEditor
{
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

    //! ECAttributeEditorBase class.
    /*! Abstract base class for attribute editing. User can add editable attributes using a AddNewAttribute method.
     *  \todo Remove QtAbstractPropertyBrowser pointer from the attribute editor, this means that manager and factory connections need to 
     *  be registered in elsewhere eg. inside the ECComponentEditor's addAttribute mehtod.
     *  \ingroup ECEditorModuleClient.
     */
    class ECAttributeEditorBase: public QObject
    {
        Q_OBJECT
    public:

        ECAttributeEditorBase(QtAbstractPropertyBrowser *owner,
                              ComponentPtr component,
                              const QString &name,
                              QObject *parent = 0);

        virtual ~ECAttributeEditorBase();

        //! Get attribute name.
        //! @return attribute type name.
        QString GetAttributeName() const { return name_; }

        //! Get editor's root property.
        //! @return editor's root property pointer.
        QtProperty *GetProperty() const { return rootProperty_; }

        //! Check if this editor's manager contain this spesific property.
        //! @param property Property that we are looking for.
        bool ContainProperty(QtProperty *property) const; 

        //! Update editor's ui elements to fit new attribute values, if different component's attribute valeus differ from
        //! each other the editor begin to use multiedit mode and editor need to create new ui elements.
        void UpdateEditorUI(IAttribute *attr = 0);

    public slots:
        //! Add new attribute to the editor. If attribute has already added do nothing.
        /*! @param attribute Attribute that we want to add to editor.
         */
        //void AddNewAttribute(IAttribute *attribute);
        //! Remove attribute from the editor.
        /*! @param attribute Attribute that we want to remove from the editor.
         */
        //void RemoveAttribute(IAttribute *attribute);
        void AddComponent(ComponentPtr component);
        void RemoveComponent(ComponentPtr component);
        bool HasComponent(ComponentPtr component);
        void AttributeChanged(IAttribute* attribute);

    signals:
        //! Signal is emmitted when editor has been reinitialized.
        /*! @param name Attribute name.
         */
        void EditorChanged(const QString &name);

    private slots:
        //! Called when user has picked one of the multiselect values.
        //! @param value new value that has been picked.
        void MultiEditValueSelected(const QString &value) 
        {
            ComponentWeakPtr comp;
            foreach(comp, components_)
            {
                if(!comp.expired())
                {
                    IAttribute *attribute = FindAttribute(comp.lock());
                    if(attribute)
                        attribute->FromString(value.toStdString(), AttributeChange::Default);
                }
            }
        }

        //! Listens if any of editor's values has been changed and the value change need to forward to the a attribute.
        void PropertyChanged(QtProperty *property){ Set(property); }

    protected:
        //! Initialize attribute editor's ui elements.
        virtual void Initialize() = 0;
        //! Read current value from the ui and set it to IAttribute.
        virtual void Set(QtProperty *property) = 0;
        //! Read attribute value from IAttribute and set it to ui.
        virtual void Update(IAttribute *attr = 0) = 0;
        //! Checks if all atributes value is same.
        //! @return return true if all attribute values are same else return false.
        virtual bool HasIdenticalAttributes() const = 0;

        void CleanExpiredComponents();
        //! PreInitialize should be called before the Initialize.
        void PreInitialize();
        //! Delete property manager and it's factory.
        void UnInitialize();

        //! Try to find attribute in component and if found return it's pointer.
        IAttribute *FindAttribute(ComponentPtr component);
        QList<ComponentWeakPtr>::iterator FindComponent(ComponentPtr component);

        QtAbstractPropertyBrowser *owner_;
        QtAbstractPropertyManager *propertyMgr_;
        QtAbstractEditorFactoryBase *factory_;
        std::vector<QtAbstractPropertyManager*> optionalPropertyManagers_;
        std::vector<QtAbstractEditorFactoryBase*> optionalPropertyFactories_;
        QtProperty *rootProperty_;
        QString name_;
        bool listenEditorChangedSignal_;
        typedef QList<ComponentWeakPtr> ComponentWeakPtrList;
        ComponentWeakPtrList components_;
        bool useMultiEditor_;
        MetaDataFlag metaDataFlag_;
    };

    //! ECAttributeEditor is a template class that implements attribute editor ui elements for specific attribute type and forward attribute changed to IAttribute objects.
    /*! ECAttributeEditor have support to edit multiple attribute at the same time and extra attribute objects can be passed using a AddNewAttribute method, removing attributes
     *  can be done by using RemoveAttribute mehtod.
     *  To add support for a new attribute types you need to reimpement following methods:
     *   - Initialize: For intializing all ui elements for the editor. In this class the user need to choose right 
     *     PropertyManager and PropertyFactory that are reponssible for registering and creating all visual elements to the QtPropertyBrowser.
     *   - Set: Is a setter funtion for editor to AttributeInterface switch will send all user's
     *     made changes to actual object.
     *   - Update: Getter function between AttributeInterface and Editor. Editor will ask attribute's value and
     *     set it to editor's ui element.
     */
    template<typename T> class ECAttributeEditor : public ECAttributeEditorBase
    {
    public:
        ECAttributeEditor(QtAbstractPropertyBrowser *owner,
                          ComponentPtr component,
                          const QString &name,
                          QObject *parent = 0):
            ECAttributeEditorBase(owner, component, name, parent)
        {
            listenEditorChangedSignal_ = true;
        }

        ~ECAttributeEditor()
        {
            
        }

    private:
        //! Override from ECAttributeEditorBase
        virtual void Initialize();

        //! Override from ECAttributeEditorBase
        virtual void Set(QtProperty *property);

        //! Override from ECAttributeEditorBase
        virtual void Update(IAttribute *attr = 0);

        bool HasIdenticalAttributes() const
        {
            //No point to continue if there is only single component added.
            if(components_.size() <= 1)
                return true;

            ComponentWeakPtrList::const_iterator lsh = components_.begin();
            IComponent *lsh_comp = dynamic_cast<IComponent*>((*lsh).lock().get());
            if(!lsh_comp)
                return false;
            Attribute<T> *lsh_attr = dynamic_cast<Attribute<T>*>(lsh_comp->GetAttribute(name_));

            IComponent *rsh_comp = 0;
            Attribute<T> *rsh_attr = 0;
            ComponentWeakPtrList::const_iterator rhs = ++components_.begin();
            for(;rhs != components_.end(); rhs++)
            {
                rsh_comp = (*rhs).lock().get();
                if (!rsh_comp) //Ensure that component hasn't expired.
                    continue;
                rsh_attr = dynamic_cast<Attribute<T>*>(rsh_comp->GetAttribute(name_));
                if (!rsh_attr)
                    continue;

                if (rsh_attr->Get() != lsh_attr->Get())
                    return false;
            }
            return true;
        }

        //! Sends a new value to each component and emit AttributeChanged signal.
        //! @param value_ new value that is sended over to component.
        void SetValue(const T &value)
        {
            ComponentWeakPtr comp;
            foreach(comp, components_)
            {
                ComponentPtr comp_ptr = comp.lock();
                if(comp_ptr)
                {
                    IAttribute *attribute = FindAttribute(comp_ptr);
                    if(attribute)
                    {
                        Attribute<T> *attr = dynamic_cast<Attribute<T>*>(attribute);
                        if(attr)
                        {
                            listenEditorChangedSignal_ = false;
                            attr->Set(value, AttributeChange::Default);
                            listenEditorChangedSignal_ = true;
                        }
                    }
                }
            }
        }

        //! Create multiedit property manager and factory if listenEditorChangedSignal_ flag is rised.
        void InitializeMultiEditor()
        {
            ECAttributeEditorBase::PreInitialize();
            if(useMultiEditor_)
            {
                MultiEditPropertyManager *multiEditManager = new MultiEditPropertyManager(this);
                MultiEditPropertyFact *multiEditFactory = new MultiEditPropertyFact(this);
                propertyMgr_ = multiEditManager;
                factory_ = multiEditFactory;

                rootProperty_ = multiEditManager->addProperty(name_);
                owner_->setFactoryForManager(multiEditManager, multiEditFactory);
                UpdateMultiEditorValue();
                QObject::connect(multiEditManager, SIGNAL(ValueChanged(const QString &)), this, SLOT(MultiEditValueSelected(const QString &)));
            }
        }

        //! Get each components atttribute value and convert it to a string value and put that value in a string vector.
        //! TODO: Optimize this piece of code.
        void UpdateMultiEditorValue(IAttribute *attribute = 0)
        {
            QStringList stringList;
            MultiEditPropertyManager *propertyManager = dynamic_cast<MultiEditPropertyManager *>(propertyMgr_);
            // If editor's ui isn't initilaized no point to continue.
            if (!propertyManager)
                return;

            if(attribute)
            {
                QStringList values = propertyManager->AttributeValue(rootProperty_);
                QString value = QString::fromStdString(attribute->ToString());
                if (!values.contains(value))
                {
                    values.push_back(value);
                    propertyManager->SetAttributeValues(rootProperty_, stringList);
                }
            }
            else // This should only get called only once when attribute editor has just switched to multieditor mode.
            {
                ComponentWeakPtr comp;
                foreach(comp, components_)
                {
                    Attribute<T> *attribute = dynamic_cast<Attribute<T>*>(FindAttribute(comp.lock()));
                    if (!attribute)
                    {
                        //! @todo add log warning.
                        continue;
                    }
                    QString newValue = QString::fromStdString(attribute->ToString());
                    //Make sure that we wont insert same strings into the list.
                    if(!stringList.contains(newValue))
                        stringList << newValue;
                }
                propertyManager->SetAttributeValues(rootProperty_, stringList);
            }
        }
    };

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

    template<> void ECAttributeEditor<QString>::Update(IAttribute *attr);
    template<> void ECAttributeEditor<QString>::Initialize();
    template<> void ECAttributeEditor<QString>::Set(QtProperty *property);

    template<> void ECAttributeEditor<QVariant>::Update(IAttribute *attr);
    template<> void ECAttributeEditor<QVariant>::Initialize();
    template<> void ECAttributeEditor<QVariant>::Set(QtProperty *property);

    template<> void ECAttributeEditor<Transform>::Update(IAttribute *attr);
    template<> void ECAttributeEditor<Transform>::Initialize();
    template<> void ECAttributeEditor<Transform>::Set(QtProperty *property);

    template<> void ECAttributeEditor<QVariantList >::Update(IAttribute *attr);
    template<> void ECAttributeEditor<QVariantList >::Initialize();
    template<> void ECAttributeEditor<QVariantList >::Set(QtProperty *property);

    template<> void ECAttributeEditor<AssetReference>::Update(IAttribute *attr);
    template<> void ECAttributeEditor<AssetReference>::Initialize();
    template<> void ECAttributeEditor<AssetReference>::Set(QtProperty *property);
}

#endif