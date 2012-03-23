// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QWidget>
#include <QKeySequence>
#include <QMap>

class QTreeWidget;
class QTreeWidgetItem;

class Framework;

/// Key binding configuration window.
/** @todo Pending a re-implementation from QtScript. */
class KeyBindingsConfigWindow : public QWidget
{
    Q_OBJECT

public:
    explicit KeyBindingsConfigWindow(Framework *fw, QWidget *parent = 0);
    ~KeyBindingsConfigWindow();

public slots:
    void ApplyKeyConfig();
    void ButtonOK();
    void ButtonCancel();
    void ConfigListAdjustEditable(QTreeWidgetItem *item, int column);

private:
    void Clear();
    void PopulateBindingsList();
    void ExtractBindingsList();

    bool SpecialKeyPressChecker(int pressed_key);

    Framework *framework;

    QTreeWidget *configList;

    /// In the UI, the user edits values in this structure. When apply or OK is pressed, we update the real values to the input API.
    /// Edits are done here to allow Cancel to return without modifications having been done.
    QMap<QString, QKeySequence> editedActions;

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};
