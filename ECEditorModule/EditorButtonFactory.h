// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include <QWidget>

class QPushButton;

class EditorButtonFactory : public QWidget
{
public:
    EditorButtonFactory(QWidget *parent = 0);
    ~EditorButtonFactory();

    /// Takes ownership of the created button.
    QPushButton* AddButton(const QString &objectName, const QString &text);

private:
    QList<QPushButton*> buttons;
};
