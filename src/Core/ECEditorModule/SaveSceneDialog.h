// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "ECEditorModuleApi.h"
#include "FrameworkFwd.h"
#include "CoreTypes.h"

#include <QDialog>

class QLineEdit;
class QCheckBox;

/// Dialog for adding new Entity into a Scene.
class ECEDITOR_MODULE_API SaveSceneDialog : public QDialog
{
    Q_OBJECT

public:
    SaveSceneDialog(Framework *fw, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~SaveSceneDialog();

protected:
    void showEvent(QShowEvent *event);

public slots:
    /// Returns the currently selected target file for the save operation.
    QString File() const;

    /// Returns if local Entities should be saved.
    bool SaveLocalEntities() const;

    /// Returns if local Entities should be saved.
    bool SaveTemporaryEntities() const;

    /// Show file picker dialog.
    void PickFile();
    
    /// Set file to save to.
    /** @note xml boolean is only used if there is no extension in @c file. */
    void SetFile(QString file, bool xml = true);

signals:
    void Selected(QString file, bool xml, bool temporary, bool local);

private slots:
    void OnFilePicked(int result);
    void OnAccepted();

    void CenterToMainWindow();
    void CheckTempAndSync();

private:
    Framework *framework_;

    QLineEdit *editFile_;
    QCheckBox *checkBoxSaveLocal_;
    QCheckBox *checkBoxSaveTemp_;
};
