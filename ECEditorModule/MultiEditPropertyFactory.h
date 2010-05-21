// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_MultiEditPropertyFactory_h
#define incl_ECEditorModule_MultiEditPropertyFactory_h

#define QT_QTPROPERTYBROWSER_IMPORT

#include <qtpropertybrowser.h>
#include "MultiEditPropertyManager.h"

namespace ECEditor
{
    class MultiEditWidget;

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
        void UpdateAttributeValues(const QtProperty *property, const QStringList &attributes);
        void EditorDestroyed(QObject *object);

    signals:
        void ValueSelected(QtProperty *property, const QString &value);

    private:
        QMap<const QtProperty *, MultiEditWidget *> createdEditors_;
        QMap<MultiEditWidget *,const QtProperty *> editorToProperty_;
    };

    class MultiEditPropertyFactory : public QtVariantEditorFactory
    {
    public:
        MultiEditPropertyFactory(QWidget *parent = 0);
        ~MultiEditPropertyFactory();

    protected:
        virtual void connectPropertyManager(QtVariantPropertyManager *manager);
        virtual QWidget *createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent);
        virtual void disconnectPropertyManager(QtVariantPropertyManager *manager);
    };
}

#endif