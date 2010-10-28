/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ArgumentType.h
 *  @brief  
 */

#ifndef incl_ECEditorModule_ArgumentType_h
#define incl_ECEditorModule_ArgumentType_h

#include "ECEditorModuleApi.h"
#include "CoreStringUtils.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

/// Pure virtual base class for different argument types.
class ECEDITOR_MODULE_API IArgumentType
{
public:
    /// Default constructor.
    IArgumentType() {}

    /// Destroyes the object.
    virtual ~IArgumentType() {}

    /// Creates new editor for the argument type.
    /** @param parent Parent widget.
    */
    virtual QWidget *CreateEditor(QWidget *parent = 0) = 0;

    /// Reads value from the dedicated editor widget created by CreateEditor().
    virtual void UpdateValueFromEditor() = 0;

    /// Sets new value.
    /** @param val Value.
    */
    virtual void SetValue(void *val) = 0;

    /// Return arguments value as QGenericArgument.
    virtual QGenericArgument Value() = 0;

    /// Return arguments value as QGenericReturnArgument.
    virtual QGenericReturnArgument ReturnValue() = 0;

    /// Returns the arguments value a string.
    virtual QString ToString() const = 0;

private:
    Q_DISABLE_COPY(IArgumentType);
};

/// Template implementation of IArgumentType.
template<typename T>
class ECEDITOR_MODULE_API ArgumentType : public IArgumentType
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
    void SetValue(void *val) { value = *reinterpret_cast<T*>(val); }

    /// IArgumentType override.
    QGenericArgument Value() { return QGenericArgument(typeName.c_str(), static_cast<void*>(&value)); }

    /// IArgumentType override.
    QGenericReturnArgument ReturnValue() { return QGenericReturnArgument(typeName.c_str(), static_cast<void*>(&value)); }

    /// IArgumentType override.
    virtual QString ToString() const { return QString(); }

private:
    /// Type name
    std::string typeName;

    /// Value.
    T value;

    /// Editor widget dedicated for this argument type.
    QWidget *editor;
};

/// Void argument type. Used for return values only.
class ECEDITOR_MODULE_API VoidArgumentType : public IArgumentType
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
    void SetValue(void *val) {}

    /// IArgumentType override. Returns empty QGenericArgument.
    QGenericArgument Value() { return QGenericArgument(); }

    /// IArgumentType override. Returns empty QGenericReturnArgument.
    QGenericReturnArgument ReturnValue() { return QGenericReturnArgument(); }

    /// IArgumentType override. Returns "void".
    QString ToString() const { return typeName; }

private:
    /// Type name
    QString typeName;
};

// QString
template<> QWidget *ArgumentType<QString>::CreateEditor(QWidget *parent);
template<> void ArgumentType<QString>::UpdateValueFromEditor();
template<> QString ArgumentType<QString>::ToString() const;

// Boolean
template<> QWidget *ArgumentType<bool>::CreateEditor(QWidget *parent);
template<> void ArgumentType<bool>::UpdateValueFromEditor();
template<> QString ArgumentType<bool>::ToString() const;

// Integer
template<> QWidget *ArgumentType<int>::CreateEditor(QWidget *parent);
template<> void ArgumentType<int>::UpdateValueFromEditor();
template<> QString ArgumentType<int>::ToString() const;

// Unsigned int and its most common typedefs 
/*
template<>
QWidget *ArgumentType<unsigned int>::CreateEditor(QWidget *parent)
{
    return new QSpinBox(parent);
}

template<>
QString ArgumentType<unsigned int>::ToString() const
{
    return QString::number((unsigned int)value);
}

template<>
QWidget *ArgumentType<uint>::CreateEditor(QWidget *parent)
{
    return new QSpinBox(parent);
}

template<>
QString ArgumentType<uint>::ToString() const
{
    return QString::number((unsigned int)value);
}

template<>
QWidget *ArgumentType<size_t>::CreateEditor(QWidget *parent)
{
    return new QSpinBox(parent);
}

template<>
QString ArgumentType<size_t>::ToString() const
{
    return QString::number((unsigned int)value);
}

template<>
QWidget *ArgumentType<size_t>::CreateEditor(QWidget *parent)
{
    return new QSpinBox(parent);
}

template<>
QString ArgumentType<size_t>::ToString() const
{
    return QString::number((unsigned int)value);
}

template<>
QWidget *ArgumentType<entity_id_t>::CreateEditor(QWidget *parent)
{
    return new QSpinBox(parent);
}

template<>
QString ArgumentType<entity_id_t>::ToString() const
{
    return QString::number((unsigned int)value);
}
*/

// Float
template<> QWidget *ArgumentType<float>::CreateEditor(QWidget *parent);
template<> void ArgumentType<float>::UpdateValueFromEditor();
template<> QString ArgumentType<float>::ToString() const;

// Double
template<> QWidget *ArgumentType<double>::CreateEditor(QWidget *parent);
template<> void ArgumentType<double>::UpdateValueFromEditor();
template<> QString ArgumentType<double>::ToString() const;

#endif
