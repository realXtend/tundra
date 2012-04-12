/**
    For conditions of distribution and use, see copyright notice in LICENSE

    Implements key bindings configuration UI for Input API. */

function printObj(obj) {
  for (i in obj) {
    if (i == "locale") // This is a hack to avoid calling QLocale::toString in iteration, which would throw an overload resolution error inside Qt.
      print(i + ": locale"); 
    else
      print(i + ": " + obj[i]);
  }
}

// Saves widget position to config and destroys the widget.
function OnScriptDestroyed()
{
    if (!framework.IsHeadless())
    {
        if (settingsWidget)
        {
            SaveWindowPositionToSettings();
            /*settingsWidget.*/Clear();
            settingsWidget.deleteLater();
            settingsWidget = null;
        }
    }
}

// Key bindings widget usable only in headful mode.
if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var configFile = "tundra";
    var configUiSection = "ui";
    var configWinPos = "key bindings config windows pos"
    var settingsWidget = null;

    var configList = null;
    // In the UI, the user edits values in this structure. When apply or OK is pressed,
    // we update the real values to the input API. Edits are done here to allow Cancel
    // to return without modifications having been done.
    var editedActions = {};

    // Add menu entry to Settings menu
    var settingsMenu = findChild(ui.MainWindow().menuBar(), "SettingsMenu");
    if (!settingsMenu)
    {
        settingsMenu = ui.MainWindow().menuBar().addMenu("&Settings");
        settingsMenu.objectName = "SettingsMenu";
    }
    var keyBindings = settingsMenu.addAction("Key Bindings");
    keyBindings.triggered.connect(ShowKeyBindings);

    // Shows renderer settings or hides it if it's already visible.
    function ShowKeyBindings()
    {
        if (settingsWidget)
        {
            settingsWidget.visible = !settingsWidget.visible;
            return;
        }

        CreateKeyBindingsWindow();
        settingsWidget.visible = true;

        LoadWindowPositionFromSettings();
    }

    // Creates the widget, but does not show it.
    function CreateKeyBindingsWindow()
    {
        settingsWidget = new FilterWidget(ui.MainWindow());
        settingsWidget.setWindowFlags(Qt.Tool);
        settingsWidget.setLayout(new QVBoxLayout());
        settingsWidget.layout().setContentsMargins(0,0,0,0)
        var child = ui.LoadFromFile(application.installationDirectory + "data/ui/KeyBindingsConfig.ui", false);
        child.setParent(settingsWidget);
        settingsWidget.layout().addWidget(child, 0, 0);

        settingsWidget.setWindowTitle("Key Bindings");

        // TODO: ideally we would do the following, but for some reason destroyed is not emitted ever.
/*
        settingsWidget.setAttribute(Qt.WA_DeleteOnClose);
        // Connect to destroyed signal so that we can set the pointer to null when the window is closed and destroyed.
        settingsWidget.destroyed.connect(SetPointerToNull);
        function SetPointerToNull()
        {
            settingsWidget = null;
        }
*/

        configList = findChild(settingsWidget, "configList");
//        configList.installEventFilter(settingsWidget);
        configList["itemClicked(QTreeWidgetItem*, int)"].connect(ConfigListAdjustEditable);
        configList["itemDoubleClicked(QTreeWidgetItem*, int)"].connect(ConfigListAdjustEditable);

        var applyButton = findChild(settingsWidget, "pushButtonApply");
        applyButton.clicked.connect(ApplyKeyConfig);

        PopulateBindingsList();
    }

    // Loads window position from config.
    function LoadWindowPositionFromSettings()
    {
        if (settingsWidget && config.HasValue(configFile, configUiSection, configWinPos))
        {
            var pos = config.Get(configFile, configUiSection, configWinPos);
            ui.MainWindow().EnsurePositionWithinDesktop(settingsWidget, pos);
        }
    }

    // Saves window position to config.
    function SaveWindowPositionToSettings()
    {
        if (settingsWidget)
            config.Set(configFile, configUiSection, configWinPos, settingsWidget.pos);
    }

    
    function ApplyKeyConfig()
    {
        ExtractBindingsList();
        // TODO
    /*
        var conflictingSequences = {};
        for(std::map<std::string, QKeySequence>::const_iterator it = editedActions.begin(); it != editedActions.end(); ++it)
        {
            std::map<std::string, QKeySequence>::const_iterator jt = it;
            ++jt;
            for(;jt != editedActions.end(); ++jt)
                if (it != jt && it.second == jt.second && it.second == jt.second && !conflictingSequences.contains(it.second.toString()))
                {
                    conflictingSequences.append(it.second.toString());
                    break;
                }
        }

        if (conflictingSequences.length > 0)
        {
            if (!QMessageBox.question(this, tr("Bindings Warning?"), 
                tr("Same shortcut '%1' has been set on multiple actions! Do you wish to continue?").arg(conflictingSequences.join(",")), 
                tr("&Yes"), tr("&No"),
                QString::null, 0, 1))
            {
                input.SetKeyBindings(editedActions);
                input.SaveKeyBindingsToFile();
            }
        }
        */
    }

    // Read more from http://www.qtcentre.org/threads/26689-QTableWidget-one-column-editable
    function ConfigListAdjustEditable(item, column)
    {
        print("before" + item.flags());
        if (column == 1)
            //item.setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable);
            setFlagsForQTreeWidgetItem(item, Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable);
        else
            //item.setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled);
            setFlagsForQTreeWidgetItem(item, Qt.ItemIsSelectable | Qt.ItemIsEnabled);
        print("after " + item.flags());
    }

    function ExtractBindingsList()
    {
        for(var i = 0; i < configList.topLevelItemCount; ++i)
        {
            var item = configList.topLevelItem(i);
            var actionName = item.text(0);
            var shortcut = QKeySequence.fromString(item.text(1), QKeySequence.NativeText);
            editedActions[actionName] = shortcut;
        }
    }

    function Clear()
    {
        for(var i = 0; i < configList.topLevelItemCount; ++i)
        {
            var item = configList.topLevelItem(i);
            delete item;
        }
        configList.clear();
    }

    function PopulateBindingsList()
    {
        if (!configList)
            return;

        Clear();

        var keyActions = input.KeyBindings();
        for(var action in keyActions)
        {
            var binding = keyActions[action];
            var item = new QTreeWidgetItem([action], 0);
            item.setText(1, binding); // binding is in QKeySequence.NativeText format
            setFlagsForQTreeWidgetItem(item, Qt.ItemIsSelectable);// | Qt.ItemIsEnabled | Qt.ItemIsEditable);
            //item.setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable);
            print("item.flags() " + item.flags());
            configList.addTopLevelItem(item);
        }
        configList.resizeColumnToContents(0);
    }

    function ButtonCancel()
    {
        input.LoadKeyBindingsFromFile();
    }

    // Filter object needed for overriding eventFilter
    function FilterWidget(parent)
    {
        QWidget.call(this, parent);
    }
    FilterWidget.prototype = new QWidget();
    
    FilterWidget.prototype.keyPressEvent = function(event)
    {
        var currentItem = configList.currentItem();
        print("keyPress");
        if (currentItem)
        {
            print("currentItem");
            if (event.isAutoRepeat())
            {
                print("1");
                return true;
            }
            else if (event.key() == Qt.Key_Backspace || event.key() == Qt.Key_Return || event.key() == Qt.Key_Enter )
            {
                //return QWidget.eventFilter.call(this, obj, event);
                print("2");
                return false;
            }
            else if (event.modifiers() != Qt.NoModifier && !event.isAutoRepeat())
            {
                print("3");
                var text = new QKeySequence(event.modifiers()).toString();
                print(text);
                var insertText = text.substr(0, text.length-1);
                print(insertText);
                currentItem.setText(1, insertText);
                event.accept();
                return true;
            }
            else if (event.key() == Qt.Key_Up || event.key() == Qt.Key_Down || event.key() == Qt.Key_Tab ||
                event.key() == Qt.Key_PageUp || event.key() == Qt.Key_PageDown)
            {
                print("4");
                event.accept();
                return true;
            }
        }
        print("5");
        return false;
    }

    FilterWidget.prototype.keyReleaseEvent = function(event)
    {
        print("keyRelease");
        var currentItem = configList.currentItem();
        if (currentItem)
        {
            print(currentItem.text(0) + " " + currentItem.text(1));
            if (event.isAutoRepeat())
            {
                print("1");
                return true;
            }
            else if (event.key() == Qt.Key_Backspace || event.key() == Qt.Key_Return ||
                event.key() == Qt.Key_Enter || IsSpecialKey(event.key()))
            {
                print("2");
                return false;
                //return QWidget.prototype.eventFilter.call(this, obj, event);
            }
            else if (event.modifiers() == Qt.NoModifier || event.key() != Qt.Key_unknown)
            {
                var itemText = currentItem.text(1);
                if (itemText.length() < 2)
                {
                    print("3");
                    currentItem.setText(1, "");
                    currentItem.setText(1, QKeySequence(event.key()).toString());
                    event.accept();
                    return true;
                }
                else 
                {
                    var items = itemText.split("+");
                    if (items.length > 0)
                    {
                        if (items[items.length-1].length < 2)
                        {
                            print("4");
                            currentItem.setText(1, new QKeySequence(event.key()).toString());
                            event.accept();
                            return true;
                        }
                        else
                        {
                            var alreadyThere = false;
                            for(var i = 0; i < items.length; ++i)
                                if (items[i] == new QKeySequence(event.key()).toString())
                                {
                                    alreadyThere = true;
                                    break;
                                }

                            if (alreadyThere)
                            {
                                print("5");
                                currentItem.setText(1, new QKeySequence(event.key()).toString());
                                event.accept();
                                return true;
                            }
                            else
                            {
                                print("6");
                                currentItem.setText(1, itemText + "+" + new QKeySequence(event.key()).toString());
                                event.accept();
                                return true;
                            }
                        }
                    }
                }
            }
        }

        print("7");
        return false;
    }

    FilterWidget.prototype.eventFilter = function(obj, event)
    {
        var currentItem = configList.currentItem();
        print(currentItem);
        if (currentItem && event.type() == QEvent.KeyPress)
        {
            print("eventFilter QEvent.KeyPress");
            if (event.isAutoRepeat())
            {
                return true;
            }
            else if (event.key() == Qt.Key_Backspace || event.key() == Qt.Key_Return || event.key() == Qt.Key_Enter )
            {
                //return QWidget.eventFilter.call(this, obj, event);
                return false;
            }
            else if (event.modifiers() != Qt.NoModifier && !event.isAutoRepeat())
            {
                var text = new QKeySequence(event.modifiers()).toString();
                //print(text);
                var insertText = text.substr(0, text.length-1);
                //print(insertText);
                currentItem.setText(1, insertText);
                event.accept();
                return true;
            }
            else if (event.key() == Qt.Key_Up || event.key() == Qt.Key_Down || event.key() == Qt.Key_Tab ||
                event.key() == Qt.Key_PageUp || event.key() == Qt.Key_PageDown)
            {
                event.accept();
                return true;
            }
        }
        else if (currentItem && event.type() == QEvent.KeyRelease)
        {
            print("eventFilter QEvent.KeyRelease");
            if (event.isAutoRepeat())
            {
                return true;
            }
            else if (event.key() == Qt.Key_Backspace || event.key() == Qt.Key_Return ||
                event.key() == Qt.Key_Enter || IsSpecialKey(event.key()))
            {
                return false;
                //return QWidget.prototype.eventFilter.call(this, obj, event);
            }
            else if (event.modifiers() == Qt.NoModifier || event.key() != Qt.Key_unknown)
            {
                var itemText = currentItem.text(1);
                if (itemText.length() < 2)
                {
                    currentItem.setText(1, "");
                    currentItem.setText(1, QKeySequence(event.key()).toString());
                    event.accept();
                    return true;
                }
                else 
                {
                    var items = itemText.split("+");
                    if (items.length > 0)
                    {
                        if (items[items.length-1].length < 2)
                        {
                            currentItem.setText(1, new QKeySequence(event.key()).toString());
                            event.accept();
                            return true;
                        }
                        else
                        {
                            var alreadyThere = false;
                            for(var i = 0; i < items.length; ++i)
                                if (items[i] == new QKeySequence(event.key()).toString())
                                {
                                    alreadyThere = true;
                                    break;
                                }

                            if (alreadyThere)
                            {
                                currentItem.setText(1, new QKeySequence(event.key()).toString());
                                event.accept();
                                return true;
                            }
                            else
                            {
                                currentItem.setText(1, itemText + "+" + new QKeySequence(event.key()).toString());
                                event.accept();
                                return true;
                            }
                        }
                    }
                }
            }
        }

        return false;
    }

    function IsSpecialKey(key)
    {
        switch(key)
        {
        case Qt.Key_Control:
        case Qt.Key_Alt:
        case Qt.Key_Shift:
        case Qt.Key_Meta:
        case Qt.Key_AltGr:
            return true;
        default:
            return false;
        }
    }
}
