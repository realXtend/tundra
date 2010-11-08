/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ArgumentType.cpp
 *  @brief  
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ArgumentType.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

#include "MemoryLeakCheck.h"

// QString
template<> QWidget *ArgumentType<QString>::CreateEditor(QWidget *parent)
{
    editor = new QLineEdit(parent);
    return editor;
}

template<> void ArgumentType<QString>::UpdateValueFromEditor()
{
    QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
    if (e)
        value = e->text();
}

template<> void ArgumentType<QString>::UpdateValueToEditor()
{
    QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
    if (e)
        e->setText(value);
}

template<> QString ArgumentType<QString>::ToString() const
{
    return value;
}

// QStringList
template<> QWidget *ArgumentType<QStringList>::CreateEditor(QWidget *parent)
{
    editor = new QTextEdit(parent);
    return editor;
}

template<> void ArgumentType<QStringList>::UpdateValueFromEditor()
{
    QTextEdit *e = dynamic_cast<QTextEdit *>(editor);
    if (e)
        value = e->toPlainText().split("\n", QString::SkipEmptyParts);
}

template<> void ArgumentType<QStringList>::UpdateValueToEditor()
{
    QTextEdit *e = dynamic_cast<QTextEdit *>(editor);
    if (e)
    {
        QString val;
        foreach(QString s, value)
            val.append(s + '\n');
        e->setPlainText(val);
    }
}

template<> QString ArgumentType<QStringList>::ToString() const
{
    QString str;
    foreach(QString item, value)
        str += item + "\n";

    return str;
}

// std::string
template<> QWidget *ArgumentType<std::string>::CreateEditor(QWidget *parent)
{
    editor = new QLineEdit(parent);
    return editor;
}

template<> void ArgumentType<std::string>::UpdateValueFromEditor()
{
    QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
    if (e)
        value = e->text().toStdString();
}

template<> void ArgumentType<std::string>::UpdateValueToEditor()
{
    QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
    if (e)
        e->setText(value.c_str());
}

template<> QString ArgumentType<std::string>::ToString() const
{
    return QString(value.c_str());
}

template<> QVariant ArgumentType<std::string>::ToQVariant() const
{
    return QVariant(value.c_str());
}

template<> void ArgumentType<std::string>::FromQVariant(const QVariant &var)
{
    value = var.toString().toStdString();
}

// Boolean
template<> QWidget *ArgumentType<bool>::CreateEditor(QWidget *parent)
{
    editor = new QCheckBox(parent);
    return editor;
}

template<> void ArgumentType<bool>::UpdateValueFromEditor()
{
    QCheckBox *e = dynamic_cast<QCheckBox *>(editor);
    if (e)
        value = e->isChecked();
}

template<> void ArgumentType<bool>::UpdateValueToEditor()
{
    QCheckBox *e = dynamic_cast<QCheckBox *>(editor);
    if (e)
        e->setChecked(value);
}

template<> QString ArgumentType<bool>::ToString() const
{
    return QString::number((int)value);
}

// Unsigned integer.
template<> QWidget *ArgumentType<unsigned int>::CreateEditor(QWidget *parent)
{
    editor = new QSpinBox(parent);
    return editor;
}

template<> void ArgumentType<unsigned int>::UpdateValueFromEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> void ArgumentType<unsigned int>::UpdateValueToEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        e->setValue(value);
}

template<> QString ArgumentType<unsigned int>::ToString() const
{
    return QString::number((unsigned int)value);
}

// Integer
template<> QWidget *ArgumentType<int>::CreateEditor(QWidget *parent)
{
    editor = new QSpinBox(parent);
    static_cast<QSpinBox *>(editor)->setRange(-std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    return editor;
}

template<> void ArgumentType<int>::UpdateValueFromEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        e->setValue(value);
}

template<> void ArgumentType<int>::UpdateValueToEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> QString ArgumentType<int>::ToString() const
{
    return QString::number((int)value);
}

// Float
template<> QWidget *ArgumentType<float>::CreateEditor(QWidget *parent)
{
    editor = new QDoubleSpinBox(parent);
    static_cast<QDoubleSpinBox *>(editor)->setRange(-std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    return editor;
}

template<> void ArgumentType<float>::UpdateValueFromEditor()
{
    QDoubleSpinBox *e = dynamic_cast<QDoubleSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> void ArgumentType<float>::UpdateValueToEditor()
{
    QDoubleSpinBox *e = dynamic_cast<QDoubleSpinBox *>(editor);
    if (e)
        e->setValue(value);
}

template<> QString ArgumentType<float>::ToString() const
{
    return QString::number((float)value);
}

// Double
template<> QWidget *ArgumentType<double>::CreateEditor(QWidget *parent)
{
    editor = new QDoubleSpinBox(parent);
    static_cast<QDoubleSpinBox *>(editor)->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    return editor;
}

template<> void ArgumentType<double>::UpdateValueFromEditor()
{
    QDoubleSpinBox *e = dynamic_cast<QDoubleSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> void ArgumentType<double>::UpdateValueToEditor()
{
    QDoubleSpinBox *e = dynamic_cast<QDoubleSpinBox *>(editor);
    if (e)
        e->setValue(value);
}

template<> QString ArgumentType<double>::ToString() const
{
    return QString::number((double)value);
}
