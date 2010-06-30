// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_LineEditFactory_h
#define incl_ECEditorModule_LineEditFactory_h

#define QT_QTPROPERTYBROWSER_IMPORT
#include <QtLineEditFactory>
//#include <QLineEdit>
#include <QMap>

class QLineEdit;

namespace ECEditor
{
    class LineEditPropertyFactory: public QtAbstractEditorFactory<QtStringPropertyManager>
    {
        Q_OBJECT
    public:
        LineEditPropertyFactory(QObject *parent = 0): QtAbstractEditorFactory<QtStringPropertyManager>(parent)
        {

        }

        ~LineEditPropertyFactory()
        {

        }

    protected:
        virtual void connectPropertyManager(QtStringPropertyManager *manager);
        virtual QWidget *createEditor(QtStringPropertyManager *manager, QtProperty *property, QWidget *parent);
        virtual void disconnectPropertyManager(QtStringPropertyManager *manager);

    private slots:
        //void EditingFinnished();
        void EditorDestoryed(QObject *object);

    private:
        QMap<QtProperty *, QLineEdit *> propertyToEditor_;
        QMap<QLineEdit *, QtProperty *> editorToProperty_;
    };
}

#endif