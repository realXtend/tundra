// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_LineEditFactory_h
#define incl_ECEditorModule_LineEditFactory_h

#define QT_QTPROPERTYBROWSER_IMPORT

#include "SceneFwd.h"
#include "IAttribute.h"
#include "AttributeMetadata.h"

#include <qteditorfactory.h>

#include <QtGroupPropertyManager>
#include <QMap>
#include <QList>
#include <QPointer>

class QLineEdit;
class QPushButton;
class FunctionInvoker;
class EditorButtonFactory;
typedef QMap<QtProperty *, QWidget *> PropertyToWidgetMap;
typedef QMap<QWidget *, QtProperty *> WidgetToPropertyMap;
typedef QMap<QtProperty*, AttributeMetadata::ButtonInfoList> PropertyToButtonsMap;
typedef QMap<QtProperty*, QList<ComponentWeakPtr> > PropertyToComponentList;

class LineEditWithButtons : public QWidget
{
    Q_OBJECT

public:
    LineEditWithButtons(const QString &text, QWidget *parent = 0);
    virtual ~LineEditWithButtons();

    QLineEdit *LineEditor() const { return lineEditor_; }

signals:
    void EditingFinished();

private:
    QLayout *layout_;
    QLineEdit *lineEditor_;
};

/// \todo add support to RegExp.
class LineEditPropertyFactory: public QtAbstractEditorFactory<QtStringPropertyManager>
{
    Q_OBJECT

public:
    LineEditPropertyFactory(QObject *parent = 0);
    ~LineEditPropertyFactory();

    /// Override the old component with a new list of components.
    void SetComponents(QtProperty *property, QList<ComponentWeakPtr> components) {components2_[property] = components;}

    /// Give all buttons that need to get created while creatEditor method is called.
    void AddButtons(AttributeMetadata::ButtonInfoList buttons);

    QPointer<EditorButtonFactory> buttonFactory;

public slots:
    void ComponentAdded(QtProperty *property, IComponent *comp);
    void ComponentRemoved(QtProperty *property, IComponent *comp);

signals:
    void EditorCreated(QtProperty *prop, QObject *factory);

protected:
    virtual void connectPropertyManager(QtStringPropertyManager *manager);
    virtual QWidget *createEditor(QtStringPropertyManager *manager, QtProperty *property, QWidget *parent);
    virtual void disconnectPropertyManager(QtStringPropertyManager *manager);

private slots:
    void OnEditingFinished();
    void OnButtonClicked();
    void OnEditorDestroyed(QObject *object);

private:
    PropertyToWidgetMap propertyToWidget_;
    WidgetToPropertyMap widgetToProperty_;
    PropertyToComponentList components2_;
    QList<ComponentWeakPtr> components_;
    AttributeMetadata::ButtonInfoList buttons_; ///< @todo replace this with QMap<QtProperty*, ButtonInfoList> so that create buttons can vary based on QtProperty.
    FunctionInvoker *invoker_;
};

#endif
