// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_MultiEditPropertyFactory_h
#define incl_ECEditorModule_MultiEditPropertyFactory_h

#define QT_QTPROPERTYBROWSER_IMPORT

#include <qtpropertybrowser.h>
#include <QPointer>

#include "MultiEditPropertyManager.h"

class EditorButtonFactory;

class QPushButton;

class MultiEditButton : public QWidget
{
    Q_OBJECT

public:
    explicit MultiEditButton(QWidget *parent = 0);
    QPushButton *button;
};

/// MultiEditPropertyFactory is responsible to create all nessessary ui elements for each property.
/** When user set focus for attribute, factory's createEditor method is called and the widget that factory
    created will be released when it's focus is lost.
    @todo Some unfocus probles has been occurred when using QtTreePropertyBrowser when user right click the TreeWidget.
    @ingroup ECEditorModuleClient.
*/
class MultiEditPropertyFactory: public QtAbstractEditorFactory<MultiEditPropertyManager>
{
    Q_OBJECT

public:
    MultiEditPropertyFactory(QObject *parent = 0);
    ~MultiEditPropertyFactory();

    QPointer<EditorButtonFactory> buttonFactory;

protected:
    /// QtAbstractEditorFactory override.
    virtual void connectPropertyManager(MultiEditPropertyManager *manager);

    /// QtAbstractEditorFactory override.
    virtual QWidget *createEditor(MultiEditPropertyManager *manager, QtProperty *proeprty, QWidget *parent);

    /// QtAbstractEditorFactory override.
    virtual void disconnectPropertyManager(MultiEditPropertyManager *manager);

private slots:
    /// Called when user has picked one of the values.
    void DialogValueSelected(const QString &value);

    /// Remove dialog from the map when it's destroyed.
    void EditorDestroyed(QObject *object);

signals:
    void EditorCreated(QtProperty *prop, QObject *factory);

    /// Value has been selected from a dialog window.
    void ValueSelected(QtProperty *property, const QString &value);

private:
    QMap<const QtProperty *, QDialog *> createdEditors_;
    QMap<QDialog *,const QtProperty *> editorToProperty_;
};

#endif
