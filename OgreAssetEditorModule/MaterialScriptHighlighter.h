/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   MaterialScriptHighlighter.h
 *  @brief  Syntax highlighter for Ogre material scripts.
 */

#pragma once

#include <QSyntaxHighlighter>

/// Syntax highlighter for Ogre material scripts.
class MaterialScriptHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    /** param type "OgreMaterial" or "OgreScript" valid. */
    MaterialScriptHighlighter(const QString &type, QObject *parent = 0) : QSyntaxHighlighter(parent), scriptType(type) {}

protected:
    void highlightBlock(const QString &text);
private:
    QString scriptType;
};
