// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "BindingWidget.h"

#include <QMap>
#include <QVariant>
#include <QMessageBox>

#include <QDebug>

namespace CoreUi
{
    BindingWidget::BindingWidget(QObject *settings_object) :
        QWidget()
    {
        setupUi(this);
        helpWidget->hide();

        connect(settings_object, SIGNAL(SaveSettingsClicked()), SLOT(ExportSettings()));
        connect(restoreDefaultsPushButton, SIGNAL(clicked()), SLOT(RestoreBindingsToDefault()));
        connect(helpPushButton, SIGNAL(clicked()), SLOT(ToggleHelp()));

        // Loop trough all of our line edits and set a input validator
        KeySequenceValidator *key_sequence_validator = new KeySequenceValidator(this);
        Foundation::KeyBindings *bindings = new Foundation::KeyBindings();
        foreach (QString line_edit_identifier, bindings->GetConfigKeys())
        {
            QLineEdit *line_edit = GetLineEditForName(line_edit_identifier);
            if (line_edit)
                line_edit->setValidator(key_sequence_validator);
        }
        SAFE_DELETE(bindings);
    }

    // Public

    void BindingWidget::UpdateContent(Foundation::KeyBindings *bindings)
    {
        Foundation::KeyBindingList bindings_list = bindings->GetBindings();
        Foundation::KeyBindingList::const_iterator iter = bindings_list.begin();
        Foundation::KeyBindingList::const_iterator end = bindings_list.end();

        // Generate internal format QMap from KeyBindings data
        QMap<QString, QList<QVariant> > name_to_seq_list_map;
        while (iter != end)
        {
            Foundation::Binding binding = (*iter);
            QString event_name = bindings->NameForEvent(binding.event_ids.enter_id);
            if (!event_name.isEmpty())
            {
                if (name_to_seq_list_map.contains(event_name))
                    name_to_seq_list_map[event_name].append(binding.sequence);
                else
                {
                    QList<QVariant> seq_list;
                    seq_list.append(binding.sequence);
                    name_to_seq_list_map[event_name] = seq_list;
                }
            }
            iter++;
        }

        // Update ui with internal format list
        foreach (QString identifier, name_to_seq_list_map.keys())
        {
            QList<QVariant> value_list = name_to_seq_list_map[identifier];
            QLineEdit *line_edit = GetLineEditForName(identifier);
            if (!line_edit)
                continue;

            QString text = "";
            foreach (QVariant value, value_list)
                text += value.toString() + ", ";
            line_edit->setText(text.left(text.lastIndexOf(",")));
        }
    }

    // Private

    void BindingWidget::ExportSettings()
    {
        // Create KeyBindings from internal format map
        Foundation::KeyBindings *bindings = new Foundation::KeyBindings();
        
        // Generate internal format map
        QMap<QString, QList<QVariant> > name_to_seq_list_map;
        foreach (QString identifier, bindings->GetConfigKeys())
        {
            QLineEdit *line_edit = GetLineEditForName(identifier);
            if (!line_edit)
                continue;

            QList<QVariant> seq_value_list;
            QStringList line_edit_values = line_edit->text().split(", ");
            foreach (QString sequence_string, line_edit_values)
                seq_value_list.append(QKeySequence(sequence_string));
            name_to_seq_list_map[identifier] = seq_value_list;
        }

        foreach (QString identifier, name_to_seq_list_map.keys())
        {
            QList<QVariant> seq_list = name_to_seq_list_map[identifier];
            std::pair<int,int> id_pair = bindings->EventPairForName(identifier);
            if (id_pair.first == 0)
                continue;

            foreach (QVariant seq, seq_list)
                bindings->BindKey(QKeySequence(seq.toString()), id_pair);
        }

        emit KeyBindingsUpdated(bindings);
    }

    void BindingWidget::RestoreBindingsToDefault()
    {
        int selection = QMessageBox::question(this, "Restoring default bindings", "Really want to restore? This will remove your\ncurrently stored bindings permanently.", QMessageBox::Yes, QMessageBox::No);
        if (selection == QMessageBox::Yes)
            emit RestoreDefaultBindings();
    }

    void BindingWidget::ToggleHelp()
    {
        if (helpWidget->isVisible())
            helpWidget->hide();
        else
            helpWidget->show();
    }

    QLineEdit *BindingWidget::GetLineEditForName(QString name)
    {
        if (name == "toggle.fly")
            return toggleFlyLineEdit;
        else if (name == "toggle.camera")
            return toggleCameraModeLineEdit;
        else if (name == "toggle.console")
            return toggleConsoleLineEdit;
        else if (name == "zoom.in")
            return zoomInLineEdit;
        else if (name == "zoom.out")
            return zoomOutLineEdit;
        else if (name == "move.forward")
            return forwardLineEdit;
        else if (name == "move.back")
            return backwardLineEdit;
        else if (name == "move.left")
            return strafeLeftLineEdit;
        else if (name == "move.right")
            return strafeRightLineEdit;
        else if (name == "move.up")
            return upJumpLineEdit;
        else if (name == "move.down")
            return downLineEdit;
        else if (name == "rotate.left")
            return turnLeftLineEdit;
        else if (name == "rotate.right")
            return turnRightLineEdit;
        else if (name == "naali.delete")
            return deleteLineEdit;
        else if (name == "naali.undo")
            return undoLineEdit;
        else if (name == "naali.object.link")
            return objectLinkLineEdit;
        else if (name == "naali.object.unlink")
            return objectUnlinkLineEdit;
        else if (name == "naali.toggle.ether")
            return toggleEtherLineEdit;
        else if (name == "naali.toggle.worldchat")
            return focusChatLineEdit;
        else if (name == "python.run")
            return runLineEdit;
        else if (name == "python.restart")
            return restartLineEdit;
        else if (name == "python.duplicate.drag")
            return duplicateDragLineEdit;
        else if (name == "python.object.toggle.move")
            return toggleObjectMoveLineEdit;
        else if (name == "python.object.toggle.scale")
            return toggleObjectScaleLineEdit;
        return 0;
    }

    // =========================================================================
    // KeySequenceValidator for line edits

    KeySequenceValidator::KeySequenceValidator(QObject *parent) :
        QValidator(parent)
    {
    }

    QValidator::State KeySequenceValidator::validate(QString &input, int &pos) const
    {
        if (pos == 0)
            return QValidator::Acceptable;

        QString new_char, is_space = "", seq_string;
        new_char = input.right(1);
        if (pos >= 2)
            is_space = QString(input.at(pos-2));
        if (new_char == " ")
            return QValidator::Acceptable;
        seq_string = QKeySequence(new_char).toString();
        if (pos != 1 && (is_space != " " && is_space != "+"))
            seq_string = seq_string.toLower();
        input = input.left(pos-1) + seq_string;
        return QValidator::Acceptable;
    }
}