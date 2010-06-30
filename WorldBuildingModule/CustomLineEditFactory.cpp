// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CustomLineEditFactory.h"

#include <QList>
#include <QLineEdit>
#include <QtProperty>

namespace WorldBuilding
{
    CustomLineEditFactory::CustomLineEditFactory(QObject *parent) :
        QtAbstractEditorFactory<QtStringPropertyManager>(parent)
    {
    }

    CustomLineEditFactory::~CustomLineEditFactory()
    {
    }

    // Overrides

    QWidget *CustomLineEditFactory::createEditor(QtStringPropertyManager *manager, QtProperty *prop, QWidget *parent)
    {
        QLineEdit *editor = new QLineEdit(manager->value(prop), parent);
        property_to_editor_[prop] = editor;
        connect(editor, SIGNAL(destroyed(QObject*)), this, SLOT(EditorDestoryed(QObject*)));
        return editor;
    }

    void CustomLineEditFactory::connectPropertyManager(QtStringPropertyManager *manager)
    {
        // Left empty for a reason
    }

    void CustomLineEditFactory::disconnectPropertyManager(QtStringPropertyManager *manager)
    {
        // Left empty for a reason
    }

    // Private Slots

    void CustomLineEditFactory::EditorDestoryed(QObject *object)
    {
        QtProperty *prop = 0;
        foreach(QLineEdit *editor, property_to_editor_.values())
        {
            if (editor != object)
                continue;

            prop = property_to_editor_.key(editor);
            if (!prop)
                continue;

            QtStringPropertyManager *string_manager = 
                qobject_cast<QtStringPropertyManager*>(prop->propertyManager());
            if (!string_manager)
                break;

            string_manager->setValue(prop, editor->text());
            break;
        }

        if (prop)
            property_to_editor_.remove(prop);
    }
}