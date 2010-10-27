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

#include "Entity.h"

#include "MemoryLeakCheck.h"


// FunctionComboBox

FunctionComboBox::FunctionComboBox(QWidget *parent) : QComboBox(parent)
{
}

void FunctionComboBox::AddFunction(const FunctionMetaData &f)
{
    addItem(f.fullSignature);
    functions.append(f);
}

void FunctionComboBox::AddFunctions(const QList<FunctionMetaData> &funcs)
{
    foreach(FunctionMetaData f, funcs)
        addItem(f.fullSignature);

    functions.append(funcs);
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
    returnValueArgument(0)
{
    setAttribute(Qt::WA_DeleteOnClose);
    if (graphicsProxyWidget())
        graphicsProxyWidget()->setWindowTitle(tr("Trigger Function"));

    setWindowTitle(tr("Trigger Function"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5,5,5,5);
    mainLayout->setSpacing(6);

    QList<FunctionMetaData> fmds;
    QSet<QString> functions;

    QString targetText;
    assert(objects.size());
    if (objects.size() == 1)
        targetText.append(tr("Target: "));
    else
        targetText.append(tr("Targets: "));

    for(int i = 0; i < objects.size(); ++i)
    {
        QObject *obj = objects[i].lock().get();
        assert(obj);
        if (!obj)
            continue;

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
            targetText.append(" ,");

        QStringList methods;

        for(int i = mo->methodOffset(); i < mo->methodCount(); ++i)
        {
            const QMetaMethod &mm = mo->method(i);
            if ((mm.methodType() == QMetaMethod::Slot) && (mm.access() == QMetaMethod::Public))
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
                f.returnType = returnType;
                f.fullSignature = fullSig;
                f.signature = mm.signature();
                int start = fullSig.indexOf(' ');
                int end = fullSig.indexOf('(');
                f.function = fullSig.mid(start + 1, end - start - 1);

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

    QLabel *targetsLabel = new QLabel(targetText);

    functionComboBox = new FunctionComboBox;
    connect(functionComboBox, SIGNAL(currentIndexChanged(int)), SLOT(UpdateEditors()));
    functionComboBox ->AddFunctions(fmds);

    QLabel *functionLabel = new QLabel(tr("Function: "));

    mainLayout->addWidget(targetsLabel);
    mainLayout->addWidget(functionComboBox);

    editorLayout = new QGridLayout;
    mainLayout->insertLayout(-1, editorLayout);

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

    UpdateEditors();
}

FunctionDialog::~FunctionDialog()
{
    qDeleteAll(allocatedArguments);
}

QList<boost::weak_ptr<QObject> > FunctionDialog::Objects() const
{
    return objects;
}

QString FunctionDialog::Function() const
{
    return functionComboBox->CurrentFunction().function;
}

IArgumentType *FunctionDialog::ReturnValueArgument() const
{
    return returnValueArgument;
}

QList<IArgumentType *> FunctionDialog::Arguments() const
{
    return allocatedArguments;
}

void FunctionDialog::hideEvent(QHideEvent *)
{
    close();
}

void FunctionDialog::CreateArgumentList()
{
    QList<IArgumentType *> args;
    QPair<QString, QString> pair;
    foreach(pair, functionComboBox->CurrentFunction().parameters)
    {
        IArgumentType *arg = CreateArgumentType(pair.first);
        if (arg)
            args.append(arg);
    }

    qDeleteAll(allocatedArguments);
    allocatedArguments = args;
}

IArgumentType *FunctionDialog::CreateArgumentType(const QString &type)
{
    IArgumentType *arg = 0;
    if (type == "void")
        arg = new VoidArgumentType;
    else if (type == "QString")
        arg = new ArgumentType<QString>("QString");
    else if (type == "bool")
        arg = new ArgumentType<bool>("bool");
    else if (type == "int")
        arg = new ArgumentType<int>("int");
    else if (type == "float")
        arg = new ArgumentType<float>("float");
    else if (type == "double")
        arg = new ArgumentType<double>("double");
    else
        std::cerr << "Invalid argument type: " << type.toStdString() << std::endl;

    return arg;
}

void FunctionDialog::Execute()
{
    emit finished(2);
}

void FunctionDialog::UpdateEditors()
{
    if (functionComboBox->CurrentFunction().function.isEmpty())
        return;

    // delete widgets from gridlayout
    QLayoutItem *child;
    while((child = editorLayout->takeAt(0)) != 0)
    {
        QWidget *w = child->widget();
        SAFE_DELETE(child);
        SAFE_DELETE(w);
    }

    SAFE_DELETE(returnValueArgument);
    returnValueArgument = CreateArgumentType(functionComboBox->CurrentFunction().returnType);

    CreateArgumentList();
    if (allocatedArguments.empty())
        return;

    int idx = 0;
    foreach(IArgumentType *arg, allocatedArguments)
    {
        QLabel *label = new QLabel(arg->Value().name());
        label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        QWidget *editor = arg->CreateEditor();

        // Layout takes ownership of label and editor.
        editorLayout->addWidget(label, idx, 0);
        editorLayout->addWidget(editor, idx, 1);

        ++idx;
    }
}

