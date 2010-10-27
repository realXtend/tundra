/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ArgumentType.cpp
 *  @brief  
 */

#include "StableHeaders.h"
#include "ArgumentType.h"

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

template<> QString ArgumentType<QString>::ToString() const
{
    return value;
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

template<> QString ArgumentType<bool>::ToString() const
{
    return QString::number((int)value);
}

// Integer
template<> QWidget *ArgumentType<int>::CreateEditor(QWidget *parent)
{
    editor = new QSpinBox(parent);
    return editor;
}

template<> void ArgumentType<int>::UpdateValueFromEditor()
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
    return editor;
}

template<> void ArgumentType<float>::UpdateValueFromEditor()
{
    QDoubleSpinBox *e = dynamic_cast<QDoubleSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> QString ArgumentType<float>::ToString() const
{
    return QString::number((float)value);
}

// Double
template<> QWidget *ArgumentType<double>::CreateEditor(QWidget *parent)
{
    editor = new QDoubleSpinBox(parent);
    return editor;
}

template<> void ArgumentType<double>::UpdateValueFromEditor()
{
    QDoubleSpinBox *e = dynamic_cast<QDoubleSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> QString ArgumentType<double>::ToString() const
{
    return QString::number((double)value);
}
