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
    explicit MaterialScriptHighlighter(QObject *parent = 0) : QSyntaxHighlighter(parent) {}

protected:
    void highlightBlock(const QString &text);
};
