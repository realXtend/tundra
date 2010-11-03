/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   FunctionDialog.cpp
 *  @brief  Dialog for invoking Qt slots (i.e. functions) of entities and components.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "FunctionDialog.h"
#include "ArgumentType.h"
#include "DoxygenDocReader.h"
#include "FunctionInvoker.h"

#include "Entity.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("FunctionDialog");

//#include <QWebView>

#include "MemoryLeakCheck.h"

// FunctionComboBox

FunctionComboBox::FunctionComboBox(QWidget *parent) : QComboBox(parent)
{
    //setInsertPolicy(QComboBox::InsertAlphabetically);
    setMinimumContentsLength(50);
    setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
}

/*
void FunctionComboBox::AddFunction(const FunctionMetaData &f)
{
    addItem(f.function);
    functions.append(f);
    model()->sort(0);
    for(int i = 0; i < count(); ++i)
    id setItemText(, );
}
*/

void FunctionComboBox::AddFunctions(const QList<FunctionMetaData> &funcs)
{
    foreach(FunctionMetaData f, funcs)
        addItem(f.function);

    model()->sort(0);
//    functions.append(funcs);
    functions = funcs;
    qSort(functions);
    for(int i = 0; i < count() && i < functions.size(); ++i)
        setItemText(i, functions[i].fullSignature);
}

FunctionMetaData FunctionComboBox::CurrentFunction() const
{
    foreach(FunctionMetaData f, functions)
        if (f.fullSignature == currentText())
            return f;

    return FunctionMetaData();
}

// FunctionDialog

FunctionDialog::FunctionDialog(const QList<boost::weak_ptr<QObject> > &objs, QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f),
    objects(objs),
    invoker(new FunctionInvoker)
{
    // Set up the UI
    setAttribute(Qt::WA_DeleteOnClose);
    resize(500, 200);

    if (graphicsProxyWidget())
        graphicsProxyWidget()->setWindowTitle(tr("Trigger Function"));

    setWindowTitle(tr("Trigger Function"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5,5,5,5);
    mainLayout->setSpacing(6);

    targetsLabel = new QLabel;
    targetsLabel->setWordWrap(true);

    functionComboBox = new FunctionComboBox;
    connect(functionComboBox, SIGNAL(currentIndexChanged(int)), SLOT(UpdateEditors()));

    doxygenView = new QTextEdit/*QWebView*/;
//    doxygenView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    doxygenView->setMinimumSize(300, 50);
//    doxygenView->setMaximumSize(600, 200);
    doxygenView->hide();

    mainLayout->addWidget(targetsLabel);
    mainLayout->addWidget(doxygenView);
    mainLayout->addWidget(functionComboBox);

    editorLayout = new QGridLayout;
    mainLayout->insertLayout(-1, editorLayout);

    QLabel *returnValueLabel = new QLabel(tr("Return value(s):"));
    returnValueEdit = new QTextEdit;

    mainLayout->addWidget(returnValueLabel);
    mainLayout->addWidget(returnValueEdit);

    QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    mainLayout->insertSpacerItem(-1, spacer);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    QSpacerItem *buttonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonsLayout->addSpacerItem(buttonSpacer);
    mainLayout->insertLayout(-1, buttonsLayout);

    QPushButton *execButton = new QPushButton(tr("Execute"));
    QPushButton *execAndCloseButton = new QPushButton(tr("Execute and Close"));
    QPushButton *closeButton = new QPushButton(tr("Close"));

    connect(execButton, SIGNAL(clicked()), SLOT(Execute()));
    connect(execAndCloseButton, SIGNAL(clicked()), SLOT(accept()));
    connect(closeButton, SIGNAL(clicked()), SLOT(reject()));

    buttonsLayout->addWidget(execButton);
    buttonsLayout->addWidget(execAndCloseButton);
    buttonsLayout->addWidget(closeButton);

    GenerateTargetLabelAndFunctions();

    UpdateEditors();
}

FunctionDialog::~FunctionDialog()
{
    qDeleteAll(currentArguments);
    SAFE_DELETE(invoker);
}

QList<boost::weak_ptr<QObject> > FunctionDialog::Objects() const
{
    return objects;
}

QString FunctionDialog::Function() const
{
    return functionComboBox->CurrentFunction().function;
}

QList<IArgumentType *> FunctionDialog::Arguments() const
{
    return currentArguments;
}

void FunctionDialog::SetReturnValueText(const QString &text)
{
    returnValueEdit->setText(text);
}

void FunctionDialog::AppendReturnValueText(const QString &text)
{
    QString newText = returnValueEdit->toPlainText();
    if (!newText.isEmpty())
        newText.append('\n');
    newText.append(text);

    returnValueEdit->setText(newText);
}

void FunctionDialog::hideEvent(QHideEvent *)
{
    close();
}

void FunctionDialog::GenerateTargetLabelAndFunctions()
{
    // Generate functions for the function combo box.
    QList<FunctionMetaData> fmds;
    QSet<QString> functions;

    QString targetText;
    assert(objects.size());
    if (objects.size() == 1)
        targetText.append(tr("Target: "));
    else
        targetText.append(tr("Targets: "));

    int objectsTotal = objects.size();
    for(int i = 0; i < objectsTotal; ++i)
    {
        QObject *obj = objects[i].lock().get();
        assert(obj);
        if (!obj)
        {
            --objectsTotal;
            continue;
        }

        const QMetaObject *mo = obj->metaObject();
        targetText.append(mo->className());

        {
            Scene::Entity *e = dynamic_cast<Scene::Entity *>(obj);
            IComponent *c = dynamic_cast<IComponent *>(obj);
            if (e)
                targetText.append('(' + QString::number((uint)e->GetId()) + ')');
            else if (c)
                targetText.append('(' + c->Name() + ')');
        }

        if (i < objects.size() - 1)
            targetText.append(", ");

        for(int i = mo->methodOffset(); i < mo->methodCount(); ++i)
        {
            const QMetaMethod &mm = mo->method(i);
            //if ((mm.methodType() == QMetaMethod::Slot) && (mm.access() == QMetaMethod::Public))
            // if (mm matches with current filter)
            {
                // Craft full signature with return type and parameter names.
                QString fullSig = mm.signature();
                QList<QByteArray> pNames = mm.parameterNames();
                QList<QByteArray> pTypes = mm.parameterTypes();
                if (pTypes.size())
                {
                    // Insert as many parameter names as we pNames in total; function signatures might be missing some param names.
                    int idx, searchStart = 0;
                    for(int pIdx = 0; pIdx < pNames.size(); ++pIdx)
                    {
                        idx = fullSig.indexOf(',', searchStart);
                        QString param = ' ' + QString(pNames[pIdx]);
                        if (idx == -1)
                        {
                            // No more commas found. Assume that we can insert the last param name now.
                            if (pIdx < pNames.size())
                                fullSig.insert(fullSig.size() - 1, param);
                            break;
                        }

                        // Insert parameter name.
                        fullSig.insert(idx, param);
                        searchStart = idx + param.size() + 1;
                    }
                }

                // Prepend full signature with return type.
                QString returnType = mm.typeName();
                if (returnType.isEmpty())
                    returnType = "void";
                fullSig.prepend(returnType + ' ');

                // Construct FunctionMetaData struct.
                FunctionMetaData f;
                int start = fullSig.indexOf(' ');
                int end = fullSig.indexOf('(');
                f.function = fullSig.mid(start + 1, end - start - 1);
                f.returnType = returnType;
                f.fullSignature = fullSig;
                f.signature = mm.signature();

                for(int k = 0; k < pTypes.size(); ++k)
                    if (k < pNames.size())
                        f.parameters.push_back(qMakePair(QString(pTypes[k]), QString(pNames[k])));
                    else 
                        // Protection agains missing parameter names in the signature: insert just an empty string.
                        f.parameters.push_back(qMakePair(QString(pTypes[k]), QString()));

                fmds.push_back(f);
            }
        }
    }

    targetsLabel->setText(targetText);
    functionComboBox ->AddFunctions(fmds);

    ///\If no functions, disable exec buttons.
    /*
    if (functionComboBox->count() == 0)
    {
    }
    */
}

void FunctionDialog::Execute()
{
    emit finished(2);
}

void FunctionDialog::UpdateEditors()
{
    QPoint orgPos = pos();

    // delete widgets from gridlayout
    QLayoutItem *child;
    while((child = editorLayout->takeAt(0)) != 0)
    {
        QWidget *w = child->widget();
        SAFE_DELETE(child);
        SAFE_DELETE(w);
    }

    FunctionMetaData fmd = functionComboBox->CurrentFunction();
    if (fmd.function.isEmpty())
        return;

    if (objects[0].expired())
        return;

//    SAFE_DELETE(returnValueArgument);
//    returnValueArgument = CreateArgumentType(fmd.returnType);

    // Create and show doxygen documentation for the function.
    QString doxyFuncName = QString(objects[0].lock()->metaObject()->className()) + "::" + fmd.function;
    QUrl styleSheetPath;
    QString documentation;
    bool success = DoxygenDocReader::GetSymbolDocumentation(doxyFuncName, &documentation, &styleSheetPath);
    if (documentation.length() != 0)
    {
        doxygenView->setHtml(documentation);//, styleSheetPath);
        doxygenView->show();
    }
    else
    {
        LogDebug("Failed to find documentation!");
        doxygenView->hide();
    }

    qDeleteAll(currentArguments);
    currentArguments = invoker->CreateArgumentList(objects[0].lock().get(), fmd.function);
    if (currentArguments.empty() || (currentArguments.size() != fmd.parameters.size()))
        return;

    for(int idx = 0; idx < currentArguments.size(); ++idx)
    {
        QLabel *label = new QLabel(fmd.parameters[idx].first + ' ' + fmd.parameters[idx].second);
        label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        QWidget *editor = currentArguments[idx]->CreateEditor();

        // Layout takes ownership of label and editor.
        editorLayout->addWidget(label, idx, 0);
        editorLayout->addWidget(editor, idx, 1);
    }

    move(orgPos);
}

