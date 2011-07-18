/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreScriptHighlighter.h
 *  @brief  Syntax highlighter for Ogre scripts.
 */

#pragma once

#include <QSyntaxHighlighter>

/// Syntax highlighter for Ogre scripts.
class OgreScriptHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    /** @param type "OgreMaterial" or "OgreScript" valid. */
    OgreScriptHighlighter(const QString &type, QObject *parent = 0) : QSyntaxHighlighter(parent), scriptType(type) {}

protected:
    void highlightBlock(const QString &text);

private:
    QString scriptType;
};
