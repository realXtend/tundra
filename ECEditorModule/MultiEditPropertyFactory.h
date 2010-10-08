// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_MultiEditPropertyFactory_h
#define incl_ECEditorModule_MultiEditPropertyFactory_h

#define QT_QTPROPERTYBROWSER_IMPORT

#include <qtpropertybrowser.h>
#include "MultiEditPropertyManager.h"

namespace ECEditor
{
    //! MultiEditPropertyFactory is responsible to create all nessessary ui elements for each property.
    /*! When user set focus for attriubte factory's createEditor method is called and the widget that factory
     *  created will be released when it's focus is lost.
     *  /todo Some unfocus probles has been occurred when using QtTreePropertyBrowser when user right click the TreeWidget.
     *  \ingroup ECEditorModuleClient.
     */
    class MultiEditPropertyFact: public QtAbstractEditorFactory<MultiEditPropertyManager>
    {
        Q_OBJECT
    public:
        MultiEditPropertyFact(QObject *parent = 0);
        ~MultiEditPropertyFact();

    protected:
        //! Override from QtAbstractEditorFactory.
        virtual void connectPropertyManager(MultiEditPropertyManager *manager);

        //! Override from QtAbstractEditorFactory.
        virtual QWidget *createEditor(MultiEditPropertyManager *manager, QtProperty *proeprty, QWidget *parent);

        //! Override from QtAbstractEditorFactory.
        virtual void disconnectPropertyManager(MultiEditPropertyManager *manager);

    private slots:
        //! Called when user has picked one of the values.
        void DialogValueSelected(const QString &value);

        //! Remove dialog from the map when it's destroyed.
        void EditorDestroyed(QObject *object);
    signals:
        //! Value has been selected from a dialog window.
        void ValueSelected(QtProperty *property, const QString &value);

    private:
        QMap<const QtProperty *, QDialog *> createdEditors_;
        QMap<QDialog *,const QtProperty *> editorToProperty_;
    };
}

#endif