/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   ArgumentType.h
    @brief  Pure virtual base class for different argument types and template implementations for supported argument types. */

#pragma once

#include "TundraCoreApi.h"
#include "CoreStringUtils.h"
#include "Math/float3.h"
#include "Math/Quat.h"
#include "Math/MathFunc.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTextEdit>

/// Pure virtual base class for different argument types.
class TUNDRACORE_API IArgumentType
{
public:
    /// Constructor.
    /** @param name Type name. */
    IArgumentType(const char * argTypeName) : typeName(argTypeName) {}

    /// Destroys the object.
    virtual ~IArgumentType() {}

    /// Creates new editor for the argument type.
    /** @param parent Parent widget. */
    virtual QWidget *CreateEditor(QWidget *parent = 0) = 0;

    /// Reads value from the dedicated editor widget created by CreateEditor().
    virtual void UpdateValueFromEditor() = 0;

    /// Sets value to the editor, if editor exists.
    virtual void UpdateValueToEditor() = 0;

    /// Sets new value.
    /** @param val New value. */
    virtual void SetValue(void *val) = 0;

    /// Returns the argument's value as QGenericArgument.
    virtual QGenericArgument Value() = 0;

    /// Returns the argument's value as QGenericReturnArgument.
    virtual QGenericReturnArgument ReturnValue() = 0;

    /// Returns the argument's value a string.
    virtual QString ToString() const = 0;

    /// Returns the argument's value as a QVariant.
    virtual QVariant ToQVariant() const = 0;

    /// Sets value from QVariant.
    /** @param var New value as QVariant. */
    virtual void FromQVariant(const QVariant &var) = 0;

    /// Sets value from string.
    virtual void FromString(const QString &str) = 0;

protected:
    const std::string typeName; ///< Type name

private:
    Q_DISABLE_COPY(IArgumentType);
};

/// Template implementation of IArgumentType.
template<typename T>
class TUNDRACORE_API ArgumentType : public IArgumentType
{
public:
    /// Constructor.
    /** @param name Type name. */
    ArgumentType(const char *name) : IArgumentType(name), editor(0) {}

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
        value = var.value<T>();
    }

    /// IArgumentType override.
    virtual void FromString(const QString &) { }

private:
    T value; ///< Value.
    QWidget *editor; ///< Editor widget dedicated for this argument type.
};

/// Void argument type. Used for return values only.
class TUNDRACORE_API VoidArgumentType : public IArgumentType
{
public:
    /// Default constructor.
    /** @param name Type name. */
    VoidArgumentType() : IArgumentType("void") {}

    /// IArgumentType override. Returns 0.
    QWidget *CreateEditor(QWidget * /*parent*/ = 0) { return 0; }

    /// IArgumentType override. Does nothing.
    void UpdateValueFromEditor() {}

    /// IArgumentType override. Does nothing.
    void UpdateValueToEditor() {}

    /// IArgumentType override. Does nothing.
    void SetValue(void * /*val*/) {}

    /// IArgumentType override. Returns empty QGenericArgument.
    QGenericArgument Value() { return QGenericArgument(); }

    /// IArgumentType override. Returns empty QGenericReturnArgument.
    QGenericReturnArgument ReturnValue() { return QGenericReturnArgument(); }

    /// IArgumentType override. Returns "void".
    QString ToString() const { return typeName.c_str(); }

    /// IArgumentType override. Returns default ctor QVariant.
    QVariant ToQVariant() const { return QVariant(); }

    /// IArgumentType override. Does nothing.
    void FromQVariant(const QVariant &) {}

    /// IArgumentType override. Does nothing.
    void FromString(const QString &) {}
};

// QString
template<> inline QWidget *ArgumentType<QString>::CreateEditor(QWidget *parent)
{
    editor = new QLineEdit(parent);
    return editor;
}

template<> inline void ArgumentType<QString>::UpdateValueFromEditor()
{
    QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
    if (e)
        value = e->text();
}

template<> inline void ArgumentType<QString>::UpdateValueToEditor()
{
    QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
    if (e)
        e->setText(value);
}

template<> inline QString ArgumentType<QString>::ToString() const
{
    return value;
}

template<> inline void ArgumentType<QString>::FromString(const QString &str)
{
    value = str;
}

// QStringList
template<> inline QWidget *ArgumentType<QStringList>::CreateEditor(QWidget *parent)
{
    editor = new QTextEdit(parent);
    return editor;
}

template<> inline void ArgumentType<QStringList>::UpdateValueFromEditor()
{
    QTextEdit *e = dynamic_cast<QTextEdit *>(editor);
    if (e)
        value = e->toPlainText().split("\n", QString::SkipEmptyParts);
}

template<> inline void ArgumentType<QStringList>::UpdateValueToEditor()
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

template<> inline QString ArgumentType<QStringList>::ToString() const
{
    QString str;
    foreach(QString item, value)
        str += item + "\n";

    return str;
}

template<> inline void ArgumentType<QStringList>::FromString(const QString &str)
{
    value = str.split(",");
}

// std::string
template<> inline QWidget *ArgumentType<std::string>::CreateEditor(QWidget *parent)
{
    editor = new QLineEdit(parent);
    return editor;
}

template<> inline void ArgumentType<std::string>::UpdateValueFromEditor()
{
    QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
    if (e)
        value = e->text().toStdString();
}

template<> inline void ArgumentType<std::string>::UpdateValueToEditor()
{
    QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
    if (e)
        e->setText(value.c_str());
}

template<> inline QString ArgumentType<std::string>::ToString() const
{
    return QString(value.c_str());
}

template<> inline void ArgumentType<std::string>::FromString(const QString &str)
{
    value = str.toStdString();
}

template<> inline QVariant ArgumentType<std::string>::ToQVariant() const
{
    return QVariant(value.c_str());
}

template<> inline void ArgumentType<std::string>::FromQVariant(const QVariant &var)
{
    value = var.toString().toStdString();
}

// Boolean
template<> inline QWidget *ArgumentType<bool>::CreateEditor(QWidget *parent)
{
    editor = new QCheckBox(parent);
    return editor;
}

template<> inline void ArgumentType<bool>::UpdateValueFromEditor()
{
    QCheckBox *e = dynamic_cast<QCheckBox *>(editor);
    if (e)
        value = e->isChecked();
}

template<> inline void ArgumentType<bool>::UpdateValueToEditor()
{
    QCheckBox *e = dynamic_cast<QCheckBox *>(editor);
    if (e)
        e->setChecked(value);
}

template<> inline QString ArgumentType<bool>::ToString() const
{
    return BoolToString(value);
}

template<> inline void ArgumentType<bool>::FromString(const QString &str)
{
    value = ParseBool(str);
}

// Unsigned integer
template<> inline QWidget *ArgumentType<unsigned int>::CreateEditor(QWidget *parent)
{
    editor = new QSpinBox(parent);
    static_cast<QSpinBox *>(editor)->setRange(0, std::numeric_limits<unsigned int>::max());
    return editor;
}

template<> inline void ArgumentType<unsigned int>::UpdateValueFromEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> inline void ArgumentType<unsigned int>::UpdateValueToEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        e->setValue(value);
}

template<> inline QString ArgumentType<unsigned int>::ToString() const
{
    return QString::number(value);
}

template<> inline void ArgumentType<unsigned int>::FromString(const QString &str)
{
    value = str.toUInt();
}

// Unsigned short
template<> inline QWidget *ArgumentType<unsigned short>::CreateEditor(QWidget *parent)
{
    editor = new QSpinBox(parent);
    static_cast<QSpinBox *>(editor)->setRange(0, std::numeric_limits<unsigned short>::max());
    return editor;
}

template<> inline void ArgumentType<unsigned short>::UpdateValueFromEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> inline void ArgumentType<unsigned short>::UpdateValueToEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        e->setValue(value);
}

template<> inline QString ArgumentType<unsigned short>::ToString() const
{
    return QString::number(value);
}

template<> inline void ArgumentType<unsigned short>::FromString(const QString &str)
{
    value = str.toUShort();
}

// Short
template<> inline QWidget *ArgumentType<short>::CreateEditor(QWidget *parent)
{
    editor = new QSpinBox(parent);
    static_cast<QSpinBox *>(editor)->setRange(-std::numeric_limits<short>::max(), std::numeric_limits<short>::max());
    return editor;
}

template<> inline void ArgumentType<short>::UpdateValueFromEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> inline void ArgumentType<short>::UpdateValueToEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        e->setValue(value);
}

template<> inline QString ArgumentType<short>::ToString() const
{
    return QString::number(value);
}

template<> inline void ArgumentType<short>::FromString(const QString &str)
{
    value = str.toShort();
}

// Integer
template<> inline QWidget *ArgumentType<int>::CreateEditor(QWidget *parent)
{
    editor = new QSpinBox(parent);
    static_cast<QSpinBox *>(editor)->setRange(-std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    return editor;
}

template<> inline void ArgumentType<int>::UpdateValueFromEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> inline void ArgumentType<int>::UpdateValueToEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        e->setValue(value);
}

template<> inline QString ArgumentType<int>::ToString() const
{
    return QString::number(value);
}

template<> inline void ArgumentType<int>::FromString(const QString &str)
{
    value = str.toInt();
}

// Unsigned long
template<> inline QWidget *ArgumentType<unsigned long>::CreateEditor(QWidget *parent)
{
    editor = new QSpinBox(parent);
    static_cast<QSpinBox *>(editor)->setRange(0, std::numeric_limits<unsigned long>::max());
    return editor;
}

template<> inline void ArgumentType<unsigned long>::UpdateValueFromEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> inline void ArgumentType<unsigned long>::UpdateValueToEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        e->setValue(value);
}

template<> inline QString ArgumentType<unsigned long>::ToString() const
{
    return QString::number(value);
}

template<> inline void ArgumentType<unsigned long>::FromString(const QString &str)
{
    value = str.toULong();
}

// Long
template<> inline QWidget *ArgumentType<long>::CreateEditor(QWidget *parent)
{
    editor = new QSpinBox(parent);
    static_cast<QSpinBox *>(editor)->setRange(-std::numeric_limits<long>::max(), std::numeric_limits<long>::max());
    return editor;
}

template<> inline void ArgumentType<long>::UpdateValueFromEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> inline void ArgumentType<long>::UpdateValueToEditor()
{
    QSpinBox *e = dynamic_cast<QSpinBox *>(editor);
    if (e)
        e->setValue(value);
}

template<> inline QString ArgumentType<long>::ToString() const
{
    return QString::number(value);
}

template<> inline void ArgumentType<long>::FromString(const QString &str)
{
    value = str.toLong();
}

// Float
template<> inline QWidget *ArgumentType<float>::CreateEditor(QWidget *parent)
{
    editor = new QDoubleSpinBox(parent);
    static_cast<QDoubleSpinBox *>(editor)->setRange(-std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    return editor;
}

template<> inline void ArgumentType<float>::UpdateValueFromEditor()
{
    QDoubleSpinBox *e = dynamic_cast<QDoubleSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> inline void ArgumentType<float>::UpdateValueToEditor()
{
    QDoubleSpinBox *e = dynamic_cast<QDoubleSpinBox *>(editor);
    if (e)
        e->setValue(value);
}

template<> inline QString ArgumentType<float>::ToString() const
{
    return QString::number(value);
}

template<> inline void ArgumentType<float>::FromString(const QString &str)
{
    value = str.toFloat();
}

// Double
template<> inline QWidget *ArgumentType<double>::CreateEditor(QWidget *parent)
{
    editor = new QDoubleSpinBox(parent);
    static_cast<QDoubleSpinBox *>(editor)->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    return editor;
}

template<> inline void ArgumentType<double>::UpdateValueFromEditor()
{
    QDoubleSpinBox *e = dynamic_cast<QDoubleSpinBox *>(editor);
    if (e)
        value = e->value();
}

template<> inline void ArgumentType<double>::UpdateValueToEditor()
{
    QDoubleSpinBox *e = dynamic_cast<QDoubleSpinBox *>(editor);
    if (e)
        e->setValue(value);
}

template<> inline QString ArgumentType<double>::ToString() const
{
    return QString::number((double)value);
}

template<> inline void ArgumentType<double>::FromString(const QString &str)
{
    value = str.toDouble();
}

// float3
/// QLineEdit is used for editing float3.
template<> inline QWidget *ArgumentType<float3>::CreateEditor(QWidget *parent)
{
    editor = new QLineEdit(parent);
    return editor;
}

template<> inline void ArgumentType<float3>::UpdateValueFromEditor()
{
    QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
    if (e)
        value = float3::FromString(e->text());
}

template<> inline void ArgumentType<float3>::UpdateValueToEditor()
{
    QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
    if (e)
        e->setText(value);
}

/// Returns "x y z".
template<> inline QString ArgumentType<float3>::ToString() const
{
    return value.SerializeToString().c_str();
}

template<> inline void ArgumentType<float3>::FromString(const QString &str)
{
    value = float3::FromString(str);
}

// Quat
/// QLineEdit is used for editing Quat.
template<> inline QWidget *ArgumentType<Quat>::CreateEditor(QWidget *parent)
{
    editor = new QLineEdit(parent);
    return editor;
}

/// The value is created from the text of the line edit by interpreting it as euler angles.
template<> inline void ArgumentType<Quat>::UpdateValueFromEditor()
{
    QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
    if (e)
    {
        float3 eulerRad = DegToRad(float3::FromString(e->text()));
        value = Quat::FromEulerZYX(eulerRad.x, eulerRad.y, eulerRad.z);
    }
}

template<> inline void ArgumentType<Quat>::UpdateValueToEditor()
{
    QLineEdit *e = dynamic_cast<QLineEdit *>(editor);
    if (e)
        e->setText(value.ToEulerZYX().SerializeToString().c_str());
}

/// Returns "x y z w".
template<> inline QString ArgumentType<Quat>::ToString() const
{
    return value.SerializeToString().c_str();
}

template<> inline void ArgumentType<Quat>::FromString(const QString &str)
{
    value = Quat::FromString(str);
}
