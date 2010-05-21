#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MultiEditPropertyFactory.h"
#include "qteditorfactory.h"
#include "MultiEditWidget.h"

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    MultiEditPropertyFact::MultiEditPropertyFact(QObject *parent):
        QtAbstractEditorFactory<MultiEditPropertyManager>(parent)
    {
        
    }

    MultiEditPropertyFact::~MultiEditPropertyFact()
    {
        qDeleteAll(editorToProperty_.keys());
    }

    void MultiEditPropertyFact::connectPropertyManager(MultiEditPropertyManager *manager)
    {
        QObject::connect(manager, SIGNAL(ValueChanged(QtProperty *, const QString &)),
                            this, SLOT(Updatevalue(QtProperty *, const QString &)));
        QObject::connect(manager, SIGNAL(AttributeValuesUpdated(QtProperty *, const QStringList &)),
                            this, SLOT(UpdateAttributeValues(QtProperty *, const QStringList &)));
    }

    QWidget *MultiEditPropertyFact::createEditor(MultiEditPropertyManager *manager, QtProperty *property, QWidget *parent)
    {
        /*if(createdEditors_.contains(property))
           delete createdEditors_[property];*/

        MultiEditWidget *multiEditWidget = new MultiEditWidget(parent);
        multiEditWidget->SetAttributeValues(manager->AttributeValue(property));
        createdEditors_[property] = multiEditWidget;
        editorToProperty_[multiEditWidget] = property;
        //QObject::connect(multiEditWidget, SIGNAL(ValueSelected(const QString&)), this, SLOT());
        QObject::connect(multiEditWidget, SIGNAL(destroyed(QObject *)), this, SLOT(EditorDestroyed(QObject *)));
        
        UpdateAttributeValues(property, manager->AttributeValue(property));

        return multiEditWidget;
    }

    void MultiEditPropertyFact::disconnectPropertyManager(MultiEditPropertyManager *manager)
    {
        QObject::disconnect(manager, SIGNAL(ValueChanged(QtProperty *, const QString &)),
                               this, SLOT(Updatevalue(QtProperty *, const QString &)));
        QObject::disconnect(manager, SIGNAL(AttributeValuesUpdated(QtProperty *, const QStringList &)),
                               this, SLOT(UpdateAttributeValues(QtProperty *, const QStringList &)));
    }

    void MultiEditPropertyFact::Updatevalue(QtProperty *property, const QString &value)
    {
        
    }

    void MultiEditPropertyFact::Updatevalue(const QString &value)
    {
        MultiEditWidget *editor = dynamic_cast<MultiEditWidget*>(QObject::sender());
        if(!editor)
            return;

        QString result = editor->GetFinalResult();
    }

    void MultiEditPropertyFact::UpdateAttributeValues(QtProperty *property, const QStringList &attributes)
    {
        if(!createdEditors_.contains(property))
            return;

        QString buttonText;
        for(uint i = 0; i < attributes.size(); i++)
            buttonText += attributes[i] + QString(" ");
        if(buttonText.size() > 30)
            buttonText.chop(buttonText.size() - (buttonText.size() - 30));
        createdEditors_[property]->setText(buttonText);
    }

    void MultiEditPropertyFact::EditorDestroyed(QObject *object)
    {
        QMap<MultiEditWidget *, const QtProperty *>::ConstIterator iter = editorToProperty_.constBegin();
        while (iter != editorToProperty_.constEnd()) 
        {
            if (iter.key() == object)
            {
                MultiEditWidget *editor = iter.key();
                const QtProperty *property = iter.value();
                editorToProperty_.remove(editor);
                createdEditors_.remove(property);
                break;
            }
            iter++;
        }
        /*MultiEditWidget *editor = dynamic_cast<MultiEditWidget*>(object);
        if(!editorToProperty_.contains(editor))
            return;

        const QtProperty *property = editorToProperty_[editor];
        editorToProperty_.remove(editor);
        createdEditors_.remove(property);*/
    }

    //REMOVE BELOW.

    MultiEditPropertyFactory::MultiEditPropertyFactory(QWidget *parent):
        QtVariantEditorFactory(parent)
    {

    }
    
    MultiEditPropertyFactory::~MultiEditPropertyFactory()
    {

    }

    void MultiEditPropertyFactory::connectPropertyManager(QtVariantPropertyManager *manager)
    {
        QtVariantEditorFactory::connectPropertyManager(manager);
    }

    QWidget *MultiEditPropertyFactory::createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent)
    {
        if (manager->propertyType(property) == ExpandedVariantPropertyManager::ColorTypeId())
        {
            MultiEditWidget *dialogButton = new MultiEditWidget(parent);
            dialogButton->setText("Multiedit");
            return dialogButton;
        }
        return QtVariantEditorFactory::createEditor(manager, property, parent);
    }

    void MultiEditPropertyFactory::disconnectPropertyManager(QtVariantPropertyManager *manager)
    {
        QtVariantEditorFactory::disconnectPropertyManager(manager);
    }
}