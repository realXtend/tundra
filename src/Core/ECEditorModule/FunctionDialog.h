/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   FunctionDialog.h
    @brief  Dialog for invoking Qt slots (i.e. functions) of entities and components. */

#pragma once

#include "ECEditorModuleApi.h"
#include "EntityAction.h"

#include <QDialog>
#include <QComboBox>

#include <set>

class QGridLayout;
class QLabel;
class QTextEdit;
class QCheckBox;

class IArgumentType;
struct InvokeItem;

/// Utility data structure for indentifying and handling of function signatures.
/** @cond PRIVATE */
struct FunctionMetadata
{
    /// Less than operator. Needed for qSort().
    bool operator <(const FunctionMetadata &rhs) const { return signature < rhs.signature; }
    QString className; ///< Class name.
    QString function; ///< Function name in the simplest form.
    QString returnType; ///< Return type of the function.
    QString signature; ///< Signature of the function without return type and parameter names.
    QString fullSignature; ///< Full signature of the function including return type and parameter names.
    typedef QPair<QString, QString> Parameter; /// <Type name - name pair
    QList<Parameter> parameters; ///< Parameters of the function.
};

/// Combo box containing function meta data items.
class FunctionComboBox : public QComboBox
{
    Q_OBJECT

public:
    /// Constructs the combo box.
    /** @param parent Parent widget. */
    FunctionComboBox(QWidget *parent = 0);

    /// Adds new function to the combo box
    /** @param f Function. */
//    void AddFunction(const FunctionMetadata &f);

    /// Sets list of functions to the combo box. Overrides existing functions.
    /** @param funcs List of functions. */
    void SetFunctions(const std::set<FunctionMetadata> &funcs);

    /// Set currently active.
    /** @param function Function name.
        @param paramTypeNames List of parameter type names. */
    void SetCurrentFunction(const QString &function, const QStringList &paramTypeNames);

    /// Returns meta data structure of the currently selected function.
    FunctionMetadata CurrentFunction() const;

    /// Clears functions list and items at the combo box.
    void Clear();

    /// All available functions.
    QList<FunctionMetadata> functions;
};
/** @endcond */

typedef weak_ptr<QObject> QObjectWeakPtr;
typedef QList<QObjectWeakPtr> QObjectWeakPtrList;

/// Dialog for invoking Qt slots (i.e. functions) of entities and components.
/** Emits finished(0) when "Close" is clicked, finished(1) when "Close and Execute" is clicked,
    and finished(2), when "Execute" is cliked. The dialog is destroyed when hide() or close() is called for it.

    Use Objects(), Function() and Arguments() functions to retrieve necesary information for invoking Qt slots. */
class ECEDITOR_MODULE_API FunctionDialog : public QDialog
{
    Q_OBJECT

public:
    /// Constructs the dialog.and populates function combo box with union of all the functions of all the objects @c objs.
    /** @param objs List of objects.
        @param parent Parent widget. */
    FunctionDialog(const QObjectWeakPtrList &objs, QWidget *parent = 0);

    /// Constructs the dialog and uses information of @c invokeItem to fill the currently active function and parameter editors.
    /** @param objs List of objects.
        @param invokeItem Invoke history item 
        @param parent Parent widget. */
    FunctionDialog(const QObjectWeakPtrList &objs, const InvokeItem &invokeItem, QWidget *parent = 0);

    /// Destructor.
    ~FunctionDialog();

    /// Returns list of entities for which the action is triggered.
    QObjectWeakPtrList Objects() const;

    /// Returns name of the function in the most simplest form, f.ex. "setValue".
    QString Function() const;

    /// Returns list of arguments for the current function.
    /// Remember to call UpdateValueFromEditor() for each argument before using them.
    QList<IArgumentType *> Arguments() const;

    /// Sets return value text.
    /** @param text Text to be shown. */
    void SetReturnValueText(const QString &text);

    /// Append text in the return value text field. The given @c text always starts from a new line.
    /** @param text Text to be added. */
    void AppendReturnValueText(const QString &text);

private:
    /// QWidget override.
    void hideEvent(QHideEvent *);

    void Initialize();
    void Populate(const QMetaObject *mo, std::set<FunctionMetadata> &fmds);

    QLabel *targetsLabel; ///< Label showing the target objects.
    FunctionComboBox *functionComboBox; ///< Function combo box
    QTextEdit *doxygenView; ///< View for doxygen documentation.
    QGridLayout *editorLayout; ///< Layout for dynamically created parameter editors.
    QTextEdit *returnValueEdit; /// Text edit field for showing return values of functions.
    QCheckBox *publicCheckBox; ///< "Public" function filter check box.
    QCheckBox *protectedAndPrivateCheckBox; ///< "Protected and private" function filter check box.
    QCheckBox *slotsCheckBox; ///< "Slots" function filter check box.
    QCheckBox *signalsCheckBox; ///< "Signals" function filter check box.
    QObjectWeakPtrList objects; ///< List of objects.
    QList<IArgumentType *> currentArguments; ///< Argument types for currently active function in the combo box.

private slots:
    /// Emits finished(2).
    void Execute();

    /// Creates editor widgets for the currently selected function's parameters.
    void UpdateEditors();

    /// Generates the targel label and list of available functions according to current object selection and function filter.
    void GenerateTargetLabelAndFunctions();
};
