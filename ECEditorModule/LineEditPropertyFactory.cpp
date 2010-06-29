#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LineEditPropertyFactory.h"

#include <QLineEdit>

#include "MemoryLeakCheck.h"


namespace ECEditor
{
    void LineEditPropertyFactory::connectPropertyManager(QtStringPropertyManager *manager)
    {
        
    }

    QWidget *LineEditPropertyFactory::createEditor(QtStringPropertyManager *manager, QtProperty *property, QWidget *parent)
    {
        QLineEdit *editor = new QLineEdit(parent);
        editor->setText(manager->value(property));
        propertyToEditor_[property] = editor;
        editorToProperty_[editor] = property;
        //connect(editor, SIGNAL(editingFinished()), this, SLOT(EditingFinnished()));
        connect(editor, SIGNAL(destroyed(QObject *)), this, SLOT(EditorDestoryed(QObject *)));
        return editor;
    }

    void LineEditPropertyFactory::disconnectPropertyManager(QtStringPropertyManager *manager)
    {
        
    }

    /*void LineEditPropertyFactory::EditingFinnished()
    {
        QObject *object = sender();
        QMap<QLineEdit*, QtProperty*>::iterator iter = editorToProperty_.begin();
        while(iter != editorToProperty_.end())
        {
            if(iter.key() == object)
            {
                QtProperty *property = iter.value();
                QtStringPropertyManager *stringManager = qobject_cast<QtStringPropertyManager*>(property->propertyManager());
                if(stringManager)
                    stringManager->setValue(property, iter.key()->text());
                break;
            }
            iter++;
        }
    }*/

    void LineEditPropertyFactory::EditorDestoryed(QObject *object)
    {
        QMap<QLineEdit*, QtProperty*>::ConstIterator iter = editorToProperty_.constBegin();
        while (iter != editorToProperty_.constEnd()) 
        {
            if (iter.key() == object)
            {
                QLineEdit *editor = iter.key();
                QtProperty *property = iter.value();
                QtStringPropertyManager *stringManager = qobject_cast<QtStringPropertyManager*>(property->propertyManager());
                if(stringManager)
                    stringManager->setValue(property, iter.key()->text());

                editorToProperty_.remove(editor);
                propertyToEditor_.remove(property);
                int size = editorToProperty_.size();
                size = propertyToEditor_.size();
                break;
            }
            iter++;
        }
    }
}