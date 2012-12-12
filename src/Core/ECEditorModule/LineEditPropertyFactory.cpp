// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "LineEditPropertyFactory.h"
#include "FunctionInvoker.h"
#include "EditorButtonFactory.h"

#include "Entity.h"

#include <QLineEdit>
#include <QPushButton>
#include <QLayout>

#include "MemoryLeakCheck.h"

LineEditWithButtons::LineEditWithButtons(const QString &text, QWidget *parent):
    QWidget(parent)
{
    layout_ = new QHBoxLayout(this);
    layout_->setContentsMargins(0,0,0,0);
    layout_->setSpacing(0);

    lineEditor_ = new QLineEdit(this);
    lineEditor_->setText(text);
    layout_->addWidget(lineEditor_);
    setFocusProxy(lineEditor_);
    connect(lineEditor_, SIGNAL(editingFinished()), this, SIGNAL(EditingFinished()));
}

LineEditWithButtons::~LineEditWithButtons()
{
}

LineEditPropertyFactory::LineEditPropertyFactory(QObject *parent):
    QtAbstractEditorFactory<QtStringPropertyManager>(parent)
{
}

LineEditPropertyFactory::~LineEditPropertyFactory()
{
}

void LineEditPropertyFactory::AddButtons(AttributeMetadata::ButtonInfoList buttons)
{
    buttons_ = buttons;
}

void LineEditPropertyFactory::ComponentAdded(QtProperty * /*property*/, IComponent *comp)
{
    assert(comp);
    if(!comp)
        return;

    QList<ComponentWeakPtr>::iterator iter = components_.begin();
    for(; iter != components_.end(); ++iter)
        if ((*iter).lock().get() == comp)
            return;

    components_.push_back(ComponentWeakPtr(comp->ParentEntity()->GetComponent(comp->TypeName(), comp->Name())));
}

void LineEditPropertyFactory::ComponentRemoved(QtProperty * /*property*/, IComponent *comp)
{
    assert(comp);
    if(!comp)
        return;

    QList<ComponentWeakPtr>::iterator iter = components_.begin();
    for(; iter != components_.end(); ++iter)
        if ((*iter).lock().get() == comp)
        {
            components_.erase(iter);
            return;
        }
}

void LineEditPropertyFactory::connectPropertyManager(QtStringPropertyManager * /*manager*/)
{
}

QWidget *LineEditPropertyFactory::createEditor(QtStringPropertyManager *manager, QtProperty *property, QWidget *parent)
{
    LineEditWithButtons *editor = new LineEditWithButtons(manager->value(property), parent);
    buttonFactory = new EditorButtonFactory(parent);
    editor->layout()->addWidget(buttonFactory);

    for(uint i = 0; i < (uint)buttons_.size(); ++i)
    {
        QPushButton *button = buttonFactory->AddButton(buttons_[i].objectName, buttons_[i].text);
        connect(button, SIGNAL(clicked(bool)), SLOT(OnButtonClicked()));
    }

    propertyToWidget_[property] = editor;
    widgetToProperty_[editor] = property;

    connect(editor, SIGNAL(EditingFinished()), SLOT(OnEditingFinished()));
    connect(editor, SIGNAL(destroyed(QObject*)), SLOT(OnEditorDestroyed(QObject*)));

    emit EditorCreated(property, this);

    return editor;
}

void LineEditPropertyFactory::disconnectPropertyManager(QtStringPropertyManager * /*manager*/)
{
}

void LineEditPropertyFactory::OnButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button)
        return;

    QString function;
    for(uint i = 0; i < (uint)buttons_.size(); ++i)
    {
        if (buttons_[i].objectName == button->objectName())
        {
            function = buttons_[i].method;
            break;
        }
    }

    if(!function.isNull())
    {
        foreach(const ComponentWeakPtr &comp_ptr, components_)
        {
            ComponentPtr comp = comp_ptr.lock();
            if (comp)
            {
                QVariantList list;
                list.push_back(QVariant(button->objectName()));
                FunctionInvoker::Invoke(comp.get(), function, list);
            }
        }
    }
}

void LineEditPropertyFactory::OnEditingFinished()
{
    LineEditWithButtons *edit = qobject_cast<LineEditWithButtons*>(sender());
    if (!edit)
        return;

    WidgetToPropertyMap::ConstIterator iter = widgetToProperty_.find(edit);
    if (iter != widgetToProperty_.end())
    {
        QtProperty *property = iter.value();
        if (!property)
            return;

        QtStringPropertyManager *stringManager = qobject_cast<QtStringPropertyManager*>(property->propertyManager());
        if (stringManager)
            stringManager->setValue(property, edit->LineEditor()->text());
    }
}

void LineEditPropertyFactory::OnEditorDestroyed(QObject *object)
{
    QWidget *editor = qobject_cast<QWidget*>(object);
    WidgetToPropertyMap::ConstIterator iter = widgetToProperty_.find(editor);
    if (iter != widgetToProperty_.end())
    {
        propertyToWidget_.remove(iter.value());
        widgetToProperty_.remove(iter.key());
    }
}
