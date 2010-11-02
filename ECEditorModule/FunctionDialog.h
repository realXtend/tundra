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
#include <QMetaMethod>

class QGridLayout;
class QLabel;
class QTextEdit;

class IArgumentType;
class FunctionInvoker;

/// Utility data structure for indentifying and handling of function signatures.
struct FunctionMetaData
{
    QString function; ///< Function name in the simplest form
    QString returnType; ///< Return type of the function.
    QString signature; ///< Signature of the function without return type and parameter names.
    QString fullSignature; ///< Full signature of the function including return type and parameter names.
    QList<QPair<QString, QString> > parameters; ///< Typename-name pairs of the parameters.
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

    /// Returns list of entities for which the action is triggered.
    QList<boost::weak_ptr<QObject> > Objects() const;

    /// Returns name of the funtion in the most simplest form, f.ex. "setValue".
    QString Function() const;

    /// Returns list of arguments for the current function.
    /// Remember to call UpdateValueFromEditor() for each argument before using them.
    QList<IArgumentType *> Arguments() const;

    /// Sets return value text.
    /** @param text Text to be shown.
    */
    void SetReturnValueText(const QString &text);

    /// Append text in the return value text field. The given @c text always starts from a new line.
    /** @param text Text to be added.
    */
    void AppendReturnValueText(const QString &text);

protected:
    /// QWidget override.
    void hideEvent(QHideEvent *);

private:
    /// Generates the targel label and list of available functions according to current object selection and function filter.
    void GenerateTargetLabelAndFunctions();

    /// Function invoker object.
    FunctionInvoker *invoker;

    /// Label showing the target objects.
    QLabel *targetsLabel;

    /// Function combo box
    FunctionComboBox *functionComboBox;

    /// View for doxygen documentation.
    QTextEdit *doxygenView;

    /// Layout for dynamically created parameter editors.
    QGridLayout *editorLayout;

    /// Text edit field for showing return values of functions.
    QTextEdit *returnValueEdit;

    /// List of objects.
    QList<boost::weak_ptr<QObject> > objects;

    /// Argument types for currently active function in the combo box.
    QList<IArgumentType *> currentArguments;

    /// Filter consists access level and type of a method 
    typedef QPair<QMetaMethod::Access, QMetaMethod::MethodType> FunctionFilter;

    /// Filter used for controlling which functions are visible.
    FunctionFilter functionFilter;

private slots:
    /// Emits finished(2).
    void Execute();

    /// Creates editor widgets for the currently selected function's parameters.
    void UpdateEditors();
};

#endif
