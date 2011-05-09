// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EditorButtonFactory.h"

#include "MemoryLeakCheck.h"

EditorButtonFactory::EditorButtonFactory(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    setLayout(layout);
}

EditorButtonFactory::~EditorButtonFactory()
{
    qDeleteAll(buttons);
}

QPushButton* EditorButtonFactory::AddButton(const QString &objectName, const QString &text)
{
    QPushButton *button = new QPushButton(text, this);
    button->setObjectName(objectName);
    button->setMaximumWidth(20);
    layout()->addWidget(button);
    buttons.push_back(button);
    return button;
}

