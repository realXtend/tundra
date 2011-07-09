/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ArgumentType.h
 *  @brief  
 */

#pragma once

#include "CoreStringUtils.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTextEdit>

/// Pure virtual base class for different argument types.
class IArgumentType
{
public:
    /// Default constructor.
    IArgumentType() {}

    /// Destroys the object.
    virtual ~IArgumentType() {}

    /// Creates new editor for the argument type.
    /** @param parent Parent widget.
    */
    virtual QWidget *CreateEditor(QWidget *parent = 0) = 0;

    /// Reads value from the dedicated editor widget created by CreateEditor().
    virtual void UpdateValueFromEditor() = 0;

    /// Sets value to the editor, if editor exists.
    virtual void UpdateValueToEditor() = 0;

    /// Sets new value.
    /** @param val New value.
    */
    virtual void SetValue(void *val) = 0;

    /// Return arguments value as QGenericArgument.
    virtual QGenericArgument Value() = 0;

    /// Return arguments value as QGenericReturnArgument.
    virtual QGenericReturnArgument ReturnValue() = 0;

    /// Returns the arguments value a string.
    virtual QString ToString() const = 0;

    /// Returns the arguments value as a QVariant.
    virtual QVariant ToQVariant() const = 0;

    /// Sets value from QVariant.
    /** @param var New value as QVariant.
    */
    virtual void FromQVariant(const QVariant &var) = 0;

    virtual void FromString(const QString &str) = 0;
private:
    Q_DISABLE_COPY(IArgumentType);
};

/// Template implementation of IArgumentType.
template<typename T>
class ArgumentType : public IArgumentType
{
public:
    /// Constructor.
    /** @param name Type name.
    */
    ArgumentType(const char *name) : typeName(name), editor(0) {}

    /// IArgumentType override.
    virtual QWidget *CreateEditor(QWidget *parent = 0) { return 0; }

    /// IArgumentType override.
    virtual void UpdateValueFromEditor() {}

    /// IArgumentType override.
    virtual void UpdateValueToEditor() {}

    /// IArgumentType override.
    void SetValue(void *val) { value = *reinterpret_cast<T*>(val); }

    /// IArgumentType override.
    QGenericArgument Value() { return QGenericArgument(typeName.c_str(), static_cast<void*>(&value)); }

    /// IArgumentType override.
    QGenericReturnArgument ReturnValue() { return QGenericReturnArgument(typeName.c_str(), static_cast<void*>(&value)); }

    /// IArgumentType override.
    virtual QString ToString() const { return QString(); }

    /// IArgumentType override.
    virtual QVariant ToQVariant() const { return QVariant::fromValue<T>(value); }

    /// IArgumentType override.
    virtual void FromQVariant(const QVariant &var)
    {
        //assert(var.canConvert<T>());
        value = var.value<T>();
    }

    virtual void FromString(const QString &str)
    {
    }

private:
    std::string typeName; ///< Type name
    T value; ///< Value.
    QWidget *editor; ///< Editor widget dedicated for this argument type.
};

/// Void argument type. Used for return values only.
class VoidArgumentType : public IArgumentType
{
public:
    /// Default constructor.
    /** @param name Type name.
    */
    VoidArgumentType() : typeName("void") {}

    /// IArgumentType override. Returns 0.
    QWidget *CreateEditor(QWidget *parent = 0) { return 0; }

    /// IArgumentType override. Does nothing.
    void UpdateValueFromEditor() {}

    /// IArgumentType override. Does nothing.
    void UpdateValueToEditor() {}

    /// IArgumentType override. Does nothing.
    void SetValue(void *val) {}

    /// IArgumentType override. Returns empty QGenericArgument.
    QGenericArgument Value() { return QGenericArgument(); }

    /// IArgumentType override. Returns empty QGenericReturnArgument.
    QGenericReturnArgument ReturnValue() { return QGenericReturnArgument(); }

    /// IArgumentType override. Returns "void".
    QString ToString() const { return typeName; }

    /// IArgumentType override. Returns "void".
    QVariant ToQVariant() const { return QVariant(); }

    /// IArgumentType override. Does nothing.
    void FromQVariant(const QVariant &) {}

    /// IArgumentType override. Does nothing.
    void FromString(const QString &) {}

private:
    QString typeName; ///< Type name
};

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

template<> void ArgumentType<QString>::FromString(const QString &str)
{
    value = str;
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

template<> void ArgumentType<QStringList>::FromString(const QString &str)
{
    value = str.split(",");
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

template<> void ArgumentType<std::string>::FromString(const QString &str)
{
    value = str.toStdString();
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

template<> void ArgumentType<bool>::FromString(const QString &str)
{
    value = ParseBool(str);
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

template<> void ArgumentType<unsigned int>::FromString(const QString &str)
{
    value = str.toInt();
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

template<> void ArgumentType<int>::FromString(const QString &str)
{
    value = str.toInt();
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

template<> void ArgumentType<float>::FromString(const QString &str)
{
    value = str.toFloat();
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

template<> void ArgumentType<double>::FromString(const QString &str)
{
    value = str.toDouble();
}
