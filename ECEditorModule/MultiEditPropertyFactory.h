// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_MultiEditPropertyFactory_h
#define incl_ECEditorModule_MultiEditPropertyFactory_h

#define QT_QTPROPERTYBROWSER_IMPORT

#include <qtpropertybrowser.h>
#include "MultiEditPropertyManager.h"

namespace ECEditor
{
    class MultiEditPropertyFact: public QtAbstractEditorFactory<MultiEditPropertyManager>
    {
        Q_OBJECT
    public:
        MultiEditPropertyFact(QObject *parent = 0);
        ~MultiEditPropertyFact();

    protected:
        virtual void connectPropertyManager(MultiEditPropertyManager *manager);
        virtual QWidget *createEditor(MultiEditPropertyManager *manager, QtProperty *proeprty, QWidget *parent);
        virtual void disconnectPropertyManager(MultiEditPropertyManager *manager);

    private slots:
        void DialogValueSelected(const QString &value);
        void EditorDestroyed(QObject *object);

    signals:
        void ValueSelected(QtProperty *property, const QString &value);

    private:
        QMap<const QtProperty *, QDialog *> createdEditors_;
        QMap<QDialog *,const QtProperty *> editorToProperty_;
    };
}

#endif