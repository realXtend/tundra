// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QWidget>

class QPushButton;

/// @cond PRIVATE
class EditorButtonFactory : public QWidget
{
    Q_OBJECT

public:
    EditorButtonFactory(QWidget *parent = 0);
    ~EditorButtonFactory();

    /// Takes ownership of the created button.
    QPushButton* AddButton(const QString &objectName, const QString &text);
    QList<QPushButton*> GetButtons() { return buttons; };

private:
    QList<QPushButton*> buttons;
};
/// @endcond
