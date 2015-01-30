/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <QtWidgets>
#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{

    // highlighting rules are applied layer after layer, here two layers.
    // E.g. a keyword rule is applied first, then the line comment rule.
    // In this way keywords in commented lines no not get highlighted.
    // This solution is not completely general, but it's ok as long as
    // higher layers cover more than lower layers, pretty much like layers of paint.

    // first level rules
    QList<HighlightingRule> ruleList;

    HighlightingRule rule;
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
//    QStringList keywordPatterns;
//    keywordPatterns << "\\blayer\\b" << "\\bconnection\\b" << "\\bdatabase\\b"
//                    << "\\bfile\\b" << "\\bpattern\\b" << "\\bobserver\\b"
//                    << "\\bconversion\\b" << "\\bparameterset\\b" << "\\bsteps\\b"
//                    << "\\brepresentation\\b" << "\\bgroup\\b";
    QStringList keywords;
    keywords << "layer" << "connection" << "database"
             << "file" << "pattern" << "observer"
             << "conversion" << "parameterset" << "steps"
             << "representation" << "group";
    keywords.replaceInStrings(QRegExp("(.*)"),"\\b\\1\\b");

    rule.pattern = QRegExp(keywords.join("|"));
    rule.format = keywordFormat;
    ruleList.append(rule);


//    foreach (const QString &pattern, keywordPatterns) {
//        rule.pattern = QRegExp(pattern);
//        rule.format = keywordFormat;
//        ruleList.append(rule);
//    }



//    classFormat.setFontWeight(QFont::Bold);
//    classFormat.setForeground(Qt::darkMagenta);
//    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
//    rule.format = classFormat;
//    highlightingRules.append(rule);


    loadLibraryFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp(".+(\\.so)");
    rule.format = loadLibraryFormat;
    ruleList.append(rule);


    queryFormat.setFontItalic(true);
    rule.pattern = QRegExp("^query[^\\n]*");
    rule.format = queryFormat;
    ruleList.append(rule);

//    multiLineCommentFormat.setForeground(Qt::red);

    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    ruleList.append(rule);

    highlightingRules.append(ruleList);


    // second level rules
    ruleList.clear();

    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("^#(?!eN)[^\\n]*");
    rule.format = singleLineCommentFormat;
    ruleList.append(rule);

    eN_directiveFormat.setForeground(Qt::green);
    rule.pattern = QRegExp("#eN[^\\n]*");
    rule.format = eN_directiveFormat;
    ruleList.append(rule);

    highlightingRules.append(ruleList);


//    functionFormat.setFontItalic(true);
//    functionFormat.setForeground(Qt::blue);
//    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
//    rule.format = functionFormat;
//    highlightingRules.append(rule);

//    commentStartExpression = QRegExp("/\\*");
//    commentEndExpression = QRegExp("\\*/");


}


void Highlighter::highlightBlock(const QString &text)
{
    foreach (const QList<HighlightingRule> &rules, highlightingRules) {
    foreach (const HighlightingRule &rule, rules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }}
//    setCurrentBlockState(0);

//    int startIndex = 0;
//    if (previousBlockState() != 1)
//        startIndex = commentStartExpression.indexIn(text);

//    while (startIndex >= 0) {
//        int endIndex = commentEndExpression.indexIn(text, startIndex);
//        int commentLength;
//        if (endIndex == -1) {
//            setCurrentBlockState(1);
//            commentLength = text.length() - startIndex;
//        } else {
//            commentLength = endIndex - startIndex
//                            + commentEndExpression.matchedLength();
//        }
//        setFormat(startIndex, commentLength, multiLineCommentFormat);
//        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
//    }
}

