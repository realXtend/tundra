// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtInputModule_KeyBindingsConfigWindow_h
#define incl_QtInputModule_KeyBindingsConfigWindow_h

#include <QWidget>
#include <QKeySequence>
#include <map>
#include <string>

#include "InputApi.h"

class QTreeWidget;
class QTreeWidgetItem;

namespace Foundation
{
class Framework;
}

class INPUT_API KeyBindingsConfigWindow : public QWidget
{
    Q_OBJECT

public:
    explicit KeyBindingsConfigWindow(Foundation::Framework *framework_)
    :framework(framework_),
    configList(0)
    {
        ShowWindow();
    }

    // Creates and shows the configuration window.
    void ShowWindow();

    // Closes and deletes the configuration window.
    void CloseWindow();

public slots:
    void ApplyKeyConfig();
    void ButtonOK();
    void ButtonCancel();
    void ConfigListAdjustEditable(QTreeWidgetItem *item, int column);

private:
    void PopulateBindingsList();
    void ExtractBindingsList();

    bool SpecialKeyPressChecker(int pressed_key);

    Foundation::Framework *framework;

    QTreeWidget *configList;

    /// In the UI, the user edits values in this structure. When apply or OK is pressed, we update the real values to the input service.
    /// Edits are done here to allow Cancel to return without modifications having been done.
    std::map<std::string, QKeySequence> editedActions;

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif