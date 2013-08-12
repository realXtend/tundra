/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   FunctionDialog.cpp
    @brief  Dialog for invoking Qt slots (i.e. functions) of entities and components. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "FunctionDialog.h"
#include "ArgumentType.h"
#include "DoxygenDocReader.h"
#include "FunctionInvoker.h"
#include "InvokeItem.h"
#include "Entity.h"
#include "IAsset.h"
#include "LoggingFunctions.h"

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
void FunctionComboBox::AddFunction(const FunctionMetadata &f)
{
    addItem(f.function);
    functions.append(f);
    model()->sort(0);
    for(int i = 0; i < count(); ++i)
    id setItemText(, );
}
*/

void FunctionComboBox::SetFunctions(const std::set<FunctionMetadata> &funcs)
{
    QList<FunctionMetadata> fmdsAsList;
    foreach(const FunctionMetadata &f, funcs)
    {
        addItem(f.signature);
        fmdsAsList << f;
    }

    model()->sort(0);
//    functions.append(funcs);
    functions = fmdsAsList;
    qSort(functions);
    for(int i = 0; i < count() && i < functions.size(); ++i)
        if (itemText(i) == functions[i].signature)
        {
            setItemText(i, functions[i].fullSignature);
            //QFont font = QApplication::font();
            //font.setBold(true);
            //setItemData(i, font, Qt::FontRole);
        }
}

void FunctionComboBox::SetCurrentFunction(const QString &function, const QStringList &paramTypeNames)
{
    FunctionMetadata matchingFmd;
    foreach(FunctionMetadata f, functions)
        if (f.function == function && f.parameters.size() == paramTypeNames.size())
        {
            int matchingTypes = 0;
            int idx = 0;
            foreach(FunctionMetadata::Parameter p, f.parameters)
            {
                if (p.first == paramTypeNames[idx])
                    ++matchingTypes;
                ++idx;
            }

            if (matchingTypes == f.parameters.size())
            {
                matchingFmd = f;
                break;
            }
        }

    if (!matchingFmd.fullSignature.isEmpty())
        for(int i = 0; i < count() && i < functions.size(); ++i)
            if (itemText(i) == matchingFmd.fullSignature)
                setCurrentIndex(i);
}

FunctionMetadata FunctionComboBox::CurrentFunction() const
{
    foreach(FunctionMetadata f, functions)
        if (f.fullSignature == currentText())
            return f;

    return FunctionMetadata();
}

void FunctionComboBox::Clear()
{
    functions.clear();
    clear();
}

// FunctionDialog

FunctionDialog::FunctionDialog(const QObjectWeakPtrList &objs, QWidget *parent) :
    QDialog(parent, 0),
    objects(objs)
{
    setWindowFlags(Qt::Tool);
    Initialize();
}

FunctionDialog::FunctionDialog(const QObjectWeakPtrList &objs, const InvokeItem &invokeItem, QWidget *parent) :
    QDialog(parent, 0),
    objects(objs)
{
    setWindowFlags(Qt::Tool);
    Initialize();

    QStringList paramTypeNames;
    foreach(QVariant var, invokeItem.parameters)
        paramTypeNames << QString(var.typeName());

    functionComboBox->SetCurrentFunction(invokeItem.name, paramTypeNames);

    UpdateEditors();

    for(int i = 0; i < invokeItem.parameters.size() && i < currentArguments.size(); ++i)
    {
        currentArguments[i]->FromQVariant(invokeItem.parameters[i]);
        currentArguments[i]->UpdateValueToEditor();
    }
}

FunctionDialog::~FunctionDialog()
{
    qDeleteAll(currentArguments);
}

QObjectWeakPtrList FunctionDialog::Objects() const
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

void FunctionDialog::Initialize()
{
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
    doxygenView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    doxygenView->setMinimumSize(300, 50);
//    doxygenView->setMaximumSize(600, 200);
    doxygenView->hide();

    mainLayout->addWidget(targetsLabel);
    mainLayout->addWidget(doxygenView);
    mainLayout->addWidget(functionComboBox);

    //QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    //mainLayout->insertSpacerItem(-1, spacer);

    publicCheckBox = new QCheckBox(tr("Public"));
    publicCheckBox->setChecked(true);
    protectedAndPrivateCheckBox = new QCheckBox(tr("Protected and private"));

    slotsCheckBox = new QCheckBox(tr("Slots"));
    slotsCheckBox->setChecked(true);
    signalsCheckBox = new QCheckBox(tr("Signals"));

    connect(publicCheckBox, SIGNAL(toggled(bool)), SLOT(GenerateTargetLabelAndFunctions()));
    connect(protectedAndPrivateCheckBox, SIGNAL(toggled(bool)), SLOT(GenerateTargetLabelAndFunctions()));
    connect(slotsCheckBox, SIGNAL(toggled(bool)), SLOT(GenerateTargetLabelAndFunctions()));
    connect(signalsCheckBox, SIGNAL(toggled(bool)), SLOT(GenerateTargetLabelAndFunctions()));

    QHBoxLayout *typeLayout = new QHBoxLayout;
    typeLayout->addWidget(publicCheckBox);
    typeLayout->addWidget(protectedAndPrivateCheckBox);
    QSpacerItem *typeSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    typeLayout->addSpacerItem(typeSpacer);
    mainLayout->insertLayout(-1, typeLayout);

    QHBoxLayout *accessLayout = new QHBoxLayout;
    accessLayout->addWidget(slotsCheckBox);
    accessLayout->addWidget(signalsCheckBox);
    QSpacerItem *accessSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    accessLayout->addSpacerItem(accessSpacer);
    mainLayout->insertLayout(-1, accessLayout);

    editorLayout = new QGridLayout;
    mainLayout->insertLayout(-1, editorLayout);

    QLabel *returnValueLabel = new QLabel(tr("Return value(s):"));
    returnValueEdit = new QTextEdit;

    mainLayout->addWidget(returnValueLabel);
    mainLayout->addWidget(returnValueEdit);

//    QSpacerItem *spacer2 = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
//    mainLayout->insertSpacerItem(-1, spacer2);

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

    FunctionMetadata fmd = functionComboBox->CurrentFunction();
    if (fmd.function.isEmpty())
        return;

    if (objects[0].expired())
        return;

    // Create and show doxygen documentation for the function.
    QObject *obj = objects[0].lock().get();
    //QString doxyFuncName = QString(obj->metaObject()->className()) + "::" + fmd.function;
    QString doxyFuncName = fmd.className + "::" + fmd.function;
    QUrl styleSheetPath;
    QString documentation;
    DoxygenDocReader::GetSymbolDocumentation(doxyFuncName, &documentation, &styleSheetPath);
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
    currentArguments.clear();
    currentArguments = FunctionInvoker::CreateArgumentList(obj, fmd.signature);
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

void FunctionDialog::Populate(const QMetaObject *mo, std::set<FunctionMetadata> &fmds)
{
    if (!mo)
        return;

    // Create filter from user's check box selection to decide which methods we will show.
    QList<QMetaMethod::Access> acceptedAccessTypes;
    if (publicCheckBox->isChecked())
        acceptedAccessTypes << QMetaMethod::Public;
    if (protectedAndPrivateCheckBox->isChecked())
        acceptedAccessTypes << QMetaMethod::Protected << QMetaMethod::Private;

    QList<QMetaMethod::MethodType> acceptedMethodTypes;
    if (slotsCheckBox->isChecked())
        acceptedMethodTypes << QMetaMethod::Slot;
    if (signalsCheckBox->isChecked())
        acceptedMethodTypes << QMetaMethod::Signal;

    QList<QMetaMethod::Access>::const_iterator accessIter;
    QList<QMetaMethod::MethodType>::const_iterator methodIter;

    for(int i = mo->methodOffset(); i < mo->methodCount(); ++i)
    {
        const QMetaMethod &mm = mo->method(i);

        // Filter according to users selection.
        accessIter = qFind(acceptedAccessTypes, mm.access());
        methodIter = qFind(acceptedMethodTypes, mm.methodType());
        if ((accessIter == acceptedAccessTypes.end()) || (methodIter == acceptedMethodTypes.end()))
            continue;

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

        // Construct FunctionMetadata struct.
        FunctionMetadata f;
        f.className = mo->className();
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

        fmds.insert(f);
    }
}

void FunctionDialog::GenerateTargetLabelAndFunctions()
{
    // Generate functions for the function combo box.
    std::set<FunctionMetadata> fmds;

    QString targetText;
    assert(objects.size());
    if (objects.size() == 1)
        targetText.append(tr("Target: "));
    else
        targetText.append(tr("Targets: "));

    // Clear previous content of function combo box
    functionComboBox->Clear();

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
        const QMetaObject *superClassMo = mo->superClass();
        while(superClassMo != 0)
        {
            Populate(superClassMo, fmds);
            superClassMo = superClassMo->superClass();
        }

        targetText.append(mo->className());

        // Decorate the target text according to the object type.
        {
            Entity *e = dynamic_cast<Entity *>(obj);
            IComponent *c = dynamic_cast<IComponent *>(obj);
            IAsset *a = dynamic_cast<IAsset *>(obj);
            if (e)
                targetText.append('(' + QString::number((uint)e->Id()) + ')');
            else if (c)
                targetText.append('(' + c->Name() + ')');
            else if (a)
                targetText.append('(' + a->Name() + ')');
        }

        if (i < objects.size() - 1)
            targetText.append(", ");

        Populate(mo, fmds);
    }

    targetsLabel->setText(targetText);
    functionComboBox->SetFunctions(fmds);

    // If no functions, disable exec buttons.
    functionComboBox->setDisabled(functionComboBox->count() == 0);
}
