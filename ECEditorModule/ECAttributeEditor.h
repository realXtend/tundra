// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECAttributeEditor_h
#define incl_ECEditorModule_ECAttributeEditor_h

#include "ForwardDefines.h"
#include "Vector3D.h"
#include "CoreStringUtils.h"
#include "IComponent.h"

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

    /*class IUiElement : public QObject
    {
        Q_OBJECT
    public:
        IUiElement(QtAbstractPropertyBrowser *owner, const QString &name):
            owner_(owner),
            QObject(owner),
            root_(0),
            name_(name)
        {
            Initialize();
        }

        virtual ~IUiElement()
        {
            owner_->unsetFactoryForManager(manager_);
            SAFE_DELETE(manager_)
            SAFE_DELETE(factory_)
        }

        QtAbstractPropertyManager *GetManager() const {return manager_;}
        QtAbstractEditorFactoryBase *GetFactory() const {return factory_;}
        QtAbstractPropertyBrowser *GetOwner() const {return owner_;}

    public slots:
        void UpdateUi(){ Update(); }

    signals:
        void ElementChanged(QtProperty *property);

    protected:
        virtual void Initialize() = 0;
        virtual void Update() = 0;
        virtual void Set(QtProperty *property) = 0;

        void SetMetaFlag(MetaDataFlag flag)
        {
            flags_ = flag;
        }

        MetaDataFlag SetMetaFlag(MetaDataFlag flag) const { return flags_; }

        QString name_;
        QtProperty *root_;
        QtAbstractPropertyBrowser *owner_;
        QtAbstractPropertyManager *manager_;
        QtAbstractEditorFactoryBase *factory_;
        MetaDataFlag flags_;
    };

    class IntUiElement : public IUiElement
    {
    public:
        IntUiElement(QtAbstractPropertyBrowser *owner, const QString &name):
            IUiElement(owner, name)
        {
        }

    protected:
        virtual void Initialize()
        {
            QtVariantPropertyManager *var_manager = new QtVariantPropertyManager(this);
            QtVariantEditorFactory *var_factory = new QtVariantEditorFactory(this);
            manager_ = var_manager;
            factory_ = var_factory;
            root_ = var_manager->addProperty(QVariant::Int, name_);

            if(root_)
            {
                Update();
                QObject::connect(manager_, SIGNAL(propertyChanged(QtProperty*)), this, SLOT(ElementChanged(QtProperty*)));
            }
            owner_->setFactoryForManager(var_manager, var_factory);
        }

        virtual void Update()
        {
            
        }

        virtual void Set(QtProperty *property)
        {
            
        }
    };

    typedef QList<ComponentWeakPtr> ComponentWeakPtrList;
    class AttributeEditor : public QObject
    {
        Q_OBJECT

    public:
        enum EditorState
        {
            ValuesSame = 0,
            ValuesDiffer = 1 << 0
        };

        ~AttributeEditor()
        {
            SAFE_DELETE(ui_)
        }

    public slots:
        bool HasComponent(ComponentPtr component)
        {
            ComponentWeakPtrList::iterator iter = FindComponent(component);
            if(iter != components_.end())
                return true;
            return false;
        }

        void AddComponent(ComponentPtr component)
        {
            // Before we add new component we make sure that it's not already added
            // and it contains right attribute.
            if(!HasComponent(component) && component->GetAttribute(typename_))
                components_.push_back(ComponentWeakPtr(component));
        }

        void RemoveComponent(ComponentPtr component)
        {
            ComponentWeakPtrList::iterator iter = FindComponent(component);
            if(iter != components_.end())
                components_.erase(iter);
        }

        void OnAttributeChanged(IAttribute *attribute, AttributeChange::Type type)
        {
            assert(ui_);
            // Make sure that the attribute is right type.
            if(attribute->GetNameString() == name_.toStdString() && attribute->TypenameToString() == typename_.toStdString())
                ui_->UpdateUi();
        }

    protected:
        Q_DISABLE_COPY(AttributeEditor);

        // Checks if every attribute have same attribute values.
        bool HasSameValues() const
        {
            if(components_.size() <= 1)
                return true;

            for(uint i = 0; i < (components_.size() - 1); i++)
                for(uint j = 1; j < components_.size(); j++)
                    if (!components_[i].expired() &&
                        !components_[j].expired() &&
                        components_[i].lock()->GetAttribute(name_)->ToString() != components_[j].lock()->GetAttribute(name_)->ToString())
                        return false;
            return true;
        }

        ComponentWeakPtrList::iterator FindComponent(ComponentPtr component)
        {
            ComponentWeakPtrList::iterator iter = components_.begin();
            for(; iter != components_.end(); iter++)
                if(!(*iter).expired() && component.get() == (*iter).lock().get())
                    return iter;
            return components_.end();
        }

        AttributeEditor(const QString &name, const QString &attr_type):
            name_(name),
            typename_(attr_type)
        {}

        QString name_;
        QString typename_;
        EditorState state_;
        IUiElement *ui_;
        ComponentWeakPtrList components_;
    };*/

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
        enum AttributeEditorState
        {
            Uninitialized,
            AttributeEdited,
            AttributeUpdated,
            WaitingForResponse
        };

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
        void UpdateEditorUI();

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
        virtual void Update() = 0;
        //! PreInitialize should be called before the Initialize.
        void PreInitialize();
        //! Delete property manager and it's factory.
        void UnInitialize();
        //! Check if all attribute are holding exactly the same value.
        //! @return return true if all attributes have the same value and false if not.
        bool IsIdentical() const;

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
        AttributeEditorState editorState_;
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
        virtual void Update();

        //! Sends a new value to each component and emit AttributeChanged signal.
        //! @param value_ new value that is sended over to component.
        void SetValue(const T &value)
        {
            ComponentWeakPtr comp;
            foreach(comp, components_)
            {
                if(!comp.expired())
                {
                    IAttribute *attribute = FindAttribute(comp.lock());
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

        //! Get each components atttribute value and convert it to string and put it send the string vector to
        //! multieditor manager.
        void UpdateMultiEditorValue()
        {
            QStringList stringList;
            MultiEditPropertyManager *propertyManager = dynamic_cast<MultiEditPropertyManager *>(propertyMgr_);
            if(!propertyManager)
            {
                //! @todo add log warning.
                return;
            }

            ComponentWeakPtr comp;
            foreach(comp, components_)
            {
                Attribute<T> *attribute = dynamic_cast<Attribute<T>*>(FindAttribute(comp.lock()));
                if(!attribute)
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
    };

    template<> void ECAttributeEditor<float>::Update();
    template<> void ECAttributeEditor<float>::Initialize();
    template<> void ECAttributeEditor<float>::Set(QtProperty *property);

    template<> void ECAttributeEditor<int>::Update();
    template<> void ECAttributeEditor<int>::Initialize();
    template<> void ECAttributeEditor<int>::Set(QtProperty *property);

    template<> void ECAttributeEditor<bool>::Update();
    template<> void ECAttributeEditor<bool>::Initialize();
    template<> void ECAttributeEditor<bool>::Set(QtProperty *property);

    template<> void ECAttributeEditor<Vector3df>::Update();
    template<> void ECAttributeEditor<Vector3df>::Initialize();
    template<> void ECAttributeEditor<Vector3df>::Set(QtProperty *property);

    template<> void ECAttributeEditor<Color>::Update();
    template<> void ECAttributeEditor<Color>::Initialize();
    template<> void ECAttributeEditor<Color>::Set(QtProperty *property);

    template<> void ECAttributeEditor<QString>::Update();
    template<> void ECAttributeEditor<QString>::Initialize();
    template<> void ECAttributeEditor<QString>::Set(QtProperty *property);

    template<> void ECAttributeEditor<QVariant>::Update();
    template<> void ECAttributeEditor<QVariant>::Initialize();
    template<> void ECAttributeEditor<QVariant>::Set(QtProperty *property);

    template<> void ECAttributeEditor<Transform>::Update();
    template<> void ECAttributeEditor<Transform>::Initialize();
    template<> void ECAttributeEditor<Transform>::Set(QtProperty *property);

    template<> void ECAttributeEditor<QVariantList >::Update();
    template<> void ECAttributeEditor<QVariantList >::Initialize();
    template<> void ECAttributeEditor<QVariantList >::Set(QtProperty *property);

    template<> void ECAttributeEditor<AssetReference>::Update();
    template<> void ECAttributeEditor<AssetReference>::Initialize();
    template<> void ECAttributeEditor<AssetReference>::Set(QtProperty *property);
}

#endif