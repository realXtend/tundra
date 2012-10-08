/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   Color.h
    @brief  A 4-component color value, component values are floating-points [0.0, 1.0]. */

#pragma once

#ifdef MATH_QT_INTEROP
#include <QColor>
#include <QString>
#include <QMetaType>
#endif

#ifdef MATH_OGRE_INTEROP
#include <OgreColourValue.h>
#endif

class float4;

/// A 4-component color value, component values are floating-points [0.0, 1.0].
/** @todo Expose to QtScript by using QScriptBindings instead of manual exposing. */
class Color
{
public:
    float r; ///< Red component
    float g; ///< Green component
    float b; ///< Blue component
    float a; ///< Alpha component

    /// The default ctor initializes values to 0.f, 0.f, 0.f, 1.f (opaque black).
    Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f)
    {
    }

    Color(float nr, float ng, float nb) : r(nr), g(ng), b(nb), a(1.0f)
    {
    }

    Color(float nr, float ng, float nb, float na) : r(nr), g(ng), b(nb), a(na)
    {
    }

    bool operator == (const Color& rhs) const
    {
        return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a; /**< @todo Use epsilon! */
    }

    bool operator != (const Color& rhs) const
    {
        return !(*this == rhs);
    }

    /// Parses a string to a new Color.
    /** Accepted formats are: "r,g,b,a" or "(r,g,b,a)" or "(r;g;b;a)" or "r g b" or "r,g,b" or "(r,g,b)" or "(r;g;b)" or "r g b" .
        @sa SerializeToString */
    static Color FromString(const char *str);

    /// Implicit conversion to float4.
    operator float4() const;

    /// Returns Color as a float4.
    float4 ToFloat4() const;

#ifdef MATH_QT_INTEROP
    /// Constucts Color from QColor
    Color(const QColor &other) { r = other.redF(); g = other.greenF(); b = other.blueF(); a = other.alphaF(); }

    /// Implicit conversion to Color.
    operator QColor() const;

    /// Returns Color as QColor.
    QColor ToQColor() const { return *this; }

    /// Implicit conversion to string.
    /** @see ToString*/
    operator QString() const;

    /// Returns "Color(r,g,b,a)".
    QString ToString() const { return (QString)*this; }

    /// For QtScript-compatibility.
    QString toString() const { return (QString)*this; }

    /// Returns "r g b a".
    /** This is the preferred format for the Color if it has to be serialized to a string for machine transfer.
        @sa FromString */
    QString SerializeToString() const;

    static Color FromString(const QString &str) { return FromString(str.simplified().toStdString().c_str()); } /**< @overload*/
#endif

#ifdef MATH_OGRE_INTEROP
    /// Constructs Color from Ogre::ColourValue.
    Color(const Ogre::ColourValue &other) { r = other.r; g = other.g; b = other.b; a = other.a; }

    /// Implicit conversion to Ogre::ColourValue.
    operator Ogre::ColourValue() const { return Ogre::ColourValue(r, g, b, a); }
#endif
};

#ifdef MATH_QT_INTEROP
Q_DECLARE_METATYPE(Color)
Q_DECLARE_METATYPE(Color*)
#endif
