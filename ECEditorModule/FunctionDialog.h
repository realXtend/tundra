/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   FunctionDialog.h
 *  @brief  Dialog for invoking Qt slots (i.e. functions) of entities and components.
 */

#ifndef incl_ECEditorModule_FunctionDialog_h
#define incl_ECEditorModule_FunctionDialog_h

#include "ECEditorModuleApi.h"
#include "ForwardDefines.h"
#include "EntityAction.h"

#include <QDialog>
#include <QComboBox>

class QGridLayout;
class QLabel;

class IArgumentType;

/// Utility data structure for indentifying and handling of function signatures.
struct FunctionMetaData
{
    /// Function name in the simplest form
    QString function;

    /// Return type of the function.
    QString returnType;

    /// Signature of the function without return type and parameter names.
    QString signature;

    /// Full signature of the function including return type and parameter names.
    QString fullSignature;

    /// typename-name pairs of the parameters.
    QList<QPair<QString, QString> > parameters;
};

/// Combo box containing function meta data items.
class FunctionComboBox : public QComboBox
{
    Q_OBJECT

public:
    /// Constructs the combo box.
    /** @param parent Parent widget.
    */
    FunctionComboBox(QWidget *parent = 0);

    /// Adds new function to the combo box
    /** @param f Function.
    */
    void AddFunction(const FunctionMetaData &f);

    /// Adds list of functions to the combo box.
    /** @param funcs List of functions.
    */
    void AddFunctions(const QList<FunctionMetaData> &funcs);

    /// Returns meta data structure of the currently selected function.
    FunctionMetaData CurrentFunction() const;

    /// All available functions.
    QList<FunctionMetaData> functions;
};

/// Dialog for invoking Qt slots (i.e. functions) of entities and components.
/** Emits finished(0) when "Close" is clicked, finished(1) when "Close and Execute" is clicked,
    and finished(2), when "Execute" is cliked.

    Use Objects(), Function(), ReturnValueArgument() and Arguments() functions to retrieve
    necesary information for invoking Qt slots.
*/
class ECEDITOR_MODULE_API FunctionDialog : public QDialog
{
    Q_OBJECT

public:
    /// Constructs the dialog.
    /** Populates function combo box with union of all the functions of all the @objs.
        The dialog is destroyed when hide() or close() is called for it.
        @param objs List of objects.
        @param parent Parent widget.
        @param f Window flags.
    */
    FunctionDialog(const QList<boost::weak_ptr<QObject> > &objs, QWidget *parent = 0, Qt::WindowFlags f = 0);

    /// Destructor.
    ~FunctionDialog();

public slots:
    /// Returns list of entities for which the action is triggered.
    QList<boost::weak_ptr<QObject> > Objects() const;

    /// Returns name of the funtion in the most simplest form, f.ex. "setValue".
    QString Function() const;

    /// Return argument type of the current functions return value.
    IArgumentType *ReturnValueArgument() const;

    /// Returns list of arguments for the current function.
    /// Remember to call UpdateValueFromEditor() for each argument before using them.
    QList<IArgumentType *> Arguments() const;

    /// Sets return value text.
    /** @param text text to be shown.
    */
    void SetReturnValueText(const QString &text);

protected:
    /// QWidget override.
    void hideEvent(QHideEvent *);

private:
    /// Creates list of argument types for the current function.
    void CreateArgumentList();

    /// Returns Argument type object for spesific parameter type name.
    /** @param type Type name of the function parameter.
        @return Argument type, or 0 if invalid type name was given.
    */
    IArgumentType *CreateArgumentType(const QString &type);

    /// Function combo box
    FunctionComboBox *functionComboBox;

    /// "Execute" button.
    QPushButton *execButton;

    /// "Execute and Close" button.
    QPushButton *execAndCloseButton;

    /// Layout for dynamically created parameter editors.
    QGridLayout *editorLayout;

    /// Label for showing return values of slots.
    QLabel *returnValueLabel;

    /// List of objects.
    QList<boost::weak_ptr<QObject> > objects;

    /// For keeping track of created argument type objects.
    QList<IArgumentType *> allocatedArguments;

    /// Return value argument.
    IArgumentType *returnValueArgument;

private slots:
    /// Emits finished(2).
    void Execute();

    /// Creates editor widgets for the currently selected function's parameters.
    void UpdateEditors();
};

#endif
