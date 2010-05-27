#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MultiEditPropertyFactory.h"
#include "qteditorfactory.h"
#include "MultiEditWidget.h"
#include <QLayout>

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
        /*QObject::connect(manager, SIGNAL(AttributeValuesUpdated(const QtProperty *, const QStringList &)),
                            this, SLOT(UpdateAttributeValues(const QtProperty *, const QStringList &)));*/
    }

    QWidget *MultiEditPropertyFact::createEditor(MultiEditPropertyManager *manager, QtProperty *property, QWidget *parent)
    {
        MultiEditWidget *multiEdit = new MultiEditWidget(parent);
        
        QInputDialog *dialog = new QInputDialog(parent);
        QStringList attributes = manager->AttributeValue(property);
        dialog->setComboBoxItems(attributes);
        dialog->setInputMode(QInputDialog::TextInput);
        dialog->setComboBoxEditable(true);
        QObject::connect(multiEdit, SIGNAL(ButtonClicked()), dialog, SLOT(open()));

        //QString buttonText = QString("(%1 values)").arg(attributes.size());
        multiEdit->SetLabelText(QString("(%1 values)").arg(attributes.size()));

        createdEditors_[property] = dialog;
        editorToProperty_[dialog] = property;
        QObject::connect(dialog, SIGNAL(textValueSelected(const QString &)), 
                         this, SLOT(DialogValueSelected(const QString &)));
        QObject::connect(this, SIGNAL(ValueSelected(QtProperty *, const QString &)), 
                         manager, SLOT(SetValue(QtProperty *, const QString &)));
        QObject::connect(dialog, SIGNAL(destroyed(QObject *)),
                         this, SLOT(EditorDestroyed(QObject *)));
        //UpdateAttributeValues(property, manager->AttributeValue(property));

        return multiEdit;
    }

    void MultiEditPropertyFact::disconnectPropertyManager(MultiEditPropertyManager *manager)
    {
        /*QObject::disconnect(manager, SIGNAL(AttributeValuesUpdated(const QtProperty *, const QStringList &)),
                               this, SLOT(UpdateAttributeValues(const QtProperty *, const QStringList &)));*/
    }

    void MultiEditPropertyFact::DialogValueSelected(const QString &value)
    {
        QInputDialog *dialog = dynamic_cast<QInputDialog *>(QObject::sender());
        if(!dialog)
            return;
        if(!editorToProperty_.contains(dialog))
            return;

        QtProperty *property = const_cast<QtProperty *>(editorToProperty_[dialog]);
        
        //When the final pick is done there is no need to keep the dialog in map anymore.
        editorToProperty_.remove(dialog);
        createdEditors_.remove(editorToProperty_[dialog]);

        emit ValueSelected(property, value);
    }

    /*void MultiEditPropertyFact::UpdateAttributeValues(const QtProperty *property, const QStringList &attributes)
    {
        /*if(!createdEditors_.contains(property))
            return;

        QString buttonText;
        for(uint i = 0; i < attributes.size(); i++)
            buttonText += attributes[i] + QString(" ");
        if(buttonText.size() > 30)
            buttonText.chop(buttonText.size() - (buttonText.size() - 30));
        createdEditors_[property]->setText(buttonText);
    }*/

    void MultiEditPropertyFact::EditorDestroyed(QObject *object)
    {
        QMap<QDialog *, const QtProperty *>::ConstIterator iter = editorToProperty_.constBegin();
        while (iter != editorToProperty_.constEnd()) 
        {
            if (iter.key() == object)
            {
                QDialog *dialog = iter.key();
                const QtProperty *property = iter.value();
                editorToProperty_.remove(dialog);
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

    /*MultiEditPropertyFactory::MultiEditPropertyFactory(QWidget *parent):
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
    }*/
}