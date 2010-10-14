// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECAttributeEditor_h
#define incl_ECEditorModule_ECAttributeEditor_h

#include "ForwardDefines.h"
#include <QObject>

#include "Vector3D.h"
#include "CoreStringUtils.h"
#include "IComponent.h"
#include "AssetInterface.h"
#include "Transform.h"
#include <map>

#include "MultiEditPropertyManager.h"
#include "MultiEditPropertyFactory.h"

class QtDoublePropertyManager;
class QtVariantPropertyManager;
class QtProperty;
class QtAbstractPropertyManager;
class QtAbstractEditorFactoryBase;
class QtAbstractPropertyBrowser;

class Color;

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
    /*! Abstract base class for attribute editing. User can add editable components using the AddNewComponent method and the component is inculded
     *  inside the object's map. Note! ECAttributeEditor wont update the ui until UpdateEditorUI method is called.
     *  If attribute is edited outside of the editor, user need to inform the editor by calling a AttributeValueChanged method, witch will get a new attribute values from the
     *  AttributeInterface and update it's ui. If you are planing to add new attribute type to editor, you should take a look at ECAttributeEditor's template implementation code
     *  to see how other attribute types have been included into the editor.
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
                              IAttribute *attribute,
                              QObject *parent = 0);

        virtual ~ECAttributeEditorBase();

        //! Get attribute name.
        //! @return attribute type name.
        QString GetAttributeName() const { return attributeName_; }

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
        //! Listens if any of editor's values has been changed and the value change need to forward to the a attribute.
        void SetAttribute(QtProperty *property){ Set(property); }
        //! Add new attribute to the editor. If attribute has already added do nothing.
        /*! @param attribute Attribute that we want to add to editor.
         */
        void AddNewAttribute(IAttribute *attribute);
        //! Remove attribute from the editor.
        /*! @param attribute Attribute that we want to remove from the editor.
         */
        void RemoveAttribute(IAttribute *attribute);

    private slots:
        //! Called when user has picked one of the multiselect values.
        //! @param value new value that has been picked.
        void MultiEditValueSelected(const QString &value) 
        {
            //ECAttributeMap::iterator iter = attributeMap_.begin();
            AttributeList::iterator iter = attributes_.begin();
            for(;iter != attributes_.end(); iter++)
            {
                (*iter)->FromString(value.toStdString(), AttributeChange::Default);
            }
        }

    signals:
        //! Attribute value has been changed in the editor.
        //void AttributeChanged(const std::string &attributeName);

    protected:
        //! Initialize attribute editor's components.
        virtual void Initialize() = 0;
        //! Send new value to each component's attribute.
        virtual void Set(QtProperty *property) = 0;
        //! Get new attribute values and update them in browser window. 
        virtual void Update() = 0;
        //! Multiedit value has been selected and it need to be type casted from string to it's original form using lexical_cast.
        //virtual void FromString(const QString &value) = 0;
        //! PreInitialize should be called before the Initialize.
        void PreInitialize();
        //! Delete property manager and it's factory.
        void UnInitialize();
        //! Check if all attribute are holding exactly the same value.
        //! @return return true if all attributes have the same value and false if not.
        bool IsIdentical() const;

        QtAbstractPropertyBrowser *owner_;
        QtAbstractPropertyManager *propertyMgr_;
        QtAbstractEditorFactoryBase *factory_;
        std::vector<QtAbstractPropertyManager*> optionalPropertyManagers_;
        std::vector<QtAbstractEditorFactoryBase*> optionalPropertyFactories_;
        QtProperty *rootProperty_;
        QString attributeName_;
        bool listenEditorChangedSignal_;
        typedef std::list<IAttribute*> AttributeList;
        AttributeList attributes_;
        bool useMultiEditor_;
        AttributeEditorState editorState_;
        MetaDataFlag metaDataFlag_;
    };

    //! ECAttributeEditor template class that initializes each attribute type's visual elements and handle those changes by using AttributeInterface.
    /*! To add support for a new attribute types you need to reimpement following methods:
     *   - Initialize: For intializing all ui elements for the editor. In this class the user need to choose right 
     *     PropertyManager and PropertyFactory that are reponssible for registering and creating all visual elements to the QtPropertyBrowser.
     *   - Set: Is a setter funtion for editor to AttributeInterface switch will send all user's
     *     made changes to actual object.
     *   - Update: Getter function between AttributeInterface and Editor. Editor will ask attribute's value and
     *     set it to editor's ui element.
     *   - FromString: This method is used for multiediting. When user has picked one of the multiedit options we need to 
     *     convert the string value to actual attribute value (usually this is done by using a boost's lexical_cast).
     */
    template<typename T> class ECAttributeEditor : public ECAttributeEditorBase
    {
    public:
        ECAttributeEditor(QtAbstractPropertyBrowser *owner,
                          IAttribute *attribute,
                          QObject *parent = 0):
            ECAttributeEditorBase(owner, attribute, parent)
        {
            listenEditorChangedSignal_ = true;
        }

        ~ECAttributeEditor()
        {
            
        }

    private:
        virtual void Initialize();
        virtual void Set(QtProperty *property);
        virtual void Update();

        //! Sends a new value to each component and emit AttributeChanged signal.
        //! @param value_ new value that is sended into component's attribute.
        void SetValue(const T &value)
        {
            AttributeList::iterator iter = attributes_.begin();
            while(iter != attributes_.end())
            {
                Attribute<T> *attribute = dynamic_cast<Attribute<T>*>(*iter);
                if(attribute)
                {
                    listenEditorChangedSignal_ = false;
                    attribute->Set(value, AttributeChange::Default);
                    //attribute->GetOwner()->ComponentChanged(AttributeChange::Default);
                    listenEditorChangedSignal_ = true;
                }
                iter++;
            }
            editorState_ = AttributeEdited;
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

                rootProperty_ = multiEditManager->addProperty(attributeName_);
                owner_->setFactoryForManager(multiEditManager, multiEditFactory);
                UpdateMultiEditorValue();
                QObject::connect(multiEditManager, SIGNAL(ValueChanged(const QString &)), this, SLOT(MultiEditValueSelected(const QString &)));
            }
        }

        //! Get each components atttribute value and convert it to string and put it send the string vector to
        //! multieditor manager.
        void UpdateMultiEditorValue()
        {
            if(!rootProperty_ || !useMultiEditor_)
                return;

            QStringList stringList;
            AttributeList::iterator iter = attributes_.begin();
            MultiEditPropertyManager *propertyManager = dynamic_cast<MultiEditPropertyManager *>(propertyMgr_);
            while(iter != attributes_.end())
            {
                Attribute<T> *attribute = dynamic_cast<Attribute<T>*>(*iter);
                if(!attribute)
                {
                    iter++;
                    continue;
                }
                QString newValue = QString::fromStdString(attribute->ToString());
                //Make sure that we wont insert same strings into the list.
                if(!stringList.contains(newValue))
                    stringList << newValue;
                iter++;
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

    template<> void ECAttributeEditor<Foundation::AssetReference>::Update();
    template<> void ECAttributeEditor<Foundation::AssetReference>::Initialize();
    template<> void ECAttributeEditor<Foundation::AssetReference>::Set(QtProperty *property);
}

#endif