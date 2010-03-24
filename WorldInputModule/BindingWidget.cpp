// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "BindingWidget.h"
#include "KeyDataManager.h"

#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QMessageBox>

namespace Input
{
    BindingWidget::BindingWidget(KeyDataManager *key_data_manager, QObject *settings_object) :
        QWidget(),
        key_data_manager_(key_data_manager)
    {
        setupUi(this);

        UpdateContent(key_data_manager_->GetCurrentKeyMap());
        connect(settings_object, SIGNAL(SaveSettingsClicked()), SLOT(ExportSettings()));
        connect(restoreDefaultsPushButton, SIGNAL(clicked()), SLOT(RestoreDefaultBindings()));
    }

    // Public

    void BindingWidget::UpdateContent(QMap<QString, QList<QVariant> > key_map)
    {
        foreach (QString identifier, key_map.keys())
        {
            QList<QVariant> value_list = key_map[identifier];
            QLineEdit *line_edit = GetLineEditForName(identifier);
            if (!line_edit)
                continue;

            QString text = "";
            foreach (QVariant value, value_list)
                text += value.toString() + ", ";
            text = text.left(text.lastIndexOf(","));
            line_edit->setText(text);
        }
    }

    // Private

    void BindingWidget::ExportSettings()
    {
        QMap<QString, QList<QVariant> > new_key_map;
        foreach(QString identifier, key_data_manager_->GetConfigKeys())
        {
            QLineEdit *line_edit = GetLineEditForName(identifier);
            if (!line_edit)
                continue;

            QList<QVariant> seq_value_list;
            QStringList line_edit_values = line_edit->text().split(", ");
            foreach (QString key_sequence, line_edit_values)
                seq_value_list.append(key_sequence);
            new_key_map[identifier] = seq_value_list;

            qDebug() << "Got keys for " << identifier << " = " << line_edit_values;
        }

        key_data_manager_->WriteCustomConfig(new_key_map);
    }

    void BindingWidget::RestoreDefaultBindings()
    {
        int selection = QMessageBox::question(this, "Restoring default bindings", "Really want to restore? This will remove your\ncurrently stored bindings permanently.", QMessageBox::Yes, QMessageBox::No);
        if (selection == QMessageBox::Yes)
        {
            UpdateContent(key_data_manager_->GetDefaultKeyMap());
            key_data_manager_->RevertEverythingToDefault();
        }
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
}