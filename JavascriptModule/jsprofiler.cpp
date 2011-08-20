/*
* Copyright (C) 2007 Benjamin C Meyer
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of the contributors may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY <copyright holder> ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _WINDOWS

#include "jsprofiler.h"
#include <sys/time.h>
#include <time.h>

namespace JsProfiler {


inline suseconds_t getTimeOfDay()
{
    timeval t;
    gettimeofday(&t, NULL);
    return t.tv_usec;
}

Profiler::Profiler(QScriptEngine *engine) : QScriptEngineAgent(engine), first(true)
{
}

static unsigned int count;

Profiler::~Profiler()
{
    QString fileName = QString("callgrind.%1.out").arg(count++);
    QTextStream stdOut(stdout);
    stdOut << "===" << fileName << "===" << endl;

    QFile file(fileName);
    file.open(QFile::ReadWrite);
    QTextStream out(&file);
    out << "creator: " << QCoreApplication::applicationName() << endl;
    out << "positions: line" << endl;
    out << "events: microseconds" << endl;
    out << endl;
    QHashIterator<int, Object> i(objects);
    while (i.hasNext()) {
        i.next();
        Object *object = &objects[i.key()];
        for (int j = 0; j < object->functions.count(); ++j) {
            out << "fl=" << (object->fileName.isEmpty() ? "native" : object->fileName) << endl;
            out << "fn=" << (object->functions[j].functionName.isEmpty()
                             ? (QString("native_") + object->functions[j].startLine)
                             : object->functions[j].functionName) << endl;
            int curLine = -1;
            for (int k = 0; k < objects[i.key()].functions[j].actions.count(); ++k) {
                const Action *action = &(objects[i.key()].functions.at(j).actions[k]);
                if (action->called != 0) {
                    out << "cfl=" << objects[action->callObject].fileName << endl;
                    out << "cfn=" << action->callFunction << endl;
                    out << "calls=" << action->called << " " << action->callFunctionLine /*action->line*/ << endl;
                }
                if (curLine == -1) {
                    out << action->line;
                } else {
                    if (action->line == curLine)
                        out << "*";
                    if (action->line > curLine)
                        out << "+" << action->line - curLine;
                    if (action->line < curLine)
                        out << "-" << curLine - action->line;
                }
                out << " " << action->cost << endl;
                curLine = action->line;
            }
            out << endl;
        }
    }
}

Function *Profiler::determineCurrentFunction(qint64 scriptId)
{
    if (currentFunctionLine.isEmpty())
        return 0;
    if (scriptIdCache == scriptId && currentFunctionCache
        && currentFunctionCache->startLine == currentFunctionLine.top())
        return currentFunctionCache;

    Object *object = &objects[scriptId];
    QList<Function>::iterator i;
    i = qBinaryFind(object->functions.begin(), object->functions.end(), Function(currentFunctionLine.top()));
    if (i == object->functions.end())
        return 0;
    int fn = (i - object->functions.begin());
    currentFunctionCache = &objects[scriptId].functions[fn];
    scriptIdCache = scriptId;
    return &objects[scriptId].functions[fn];
}

QString Profiler::functionName(QScriptContext *ctx, qint64 scriptId) const
{
    QScriptContextInfo info(ctx);
    if (!info.functionName().isEmpty())
        return info.functionName();

    if (info.functionType() == QScriptContextInfo::NativeFunction && scriptId != -1)
        return "global";

    if (info.functionType() == QScriptContextInfo::NativeFunction)
        return "native";

    // wish this could be nicer
    QString lineName =objects[scriptId].code[(info.functionStartLineNumber()) - 1].simplified();
    QString strip = "= function(";
    if (lineName.contains(strip))
        lineName = lineName.mid(0, lineName.indexOf(strip));
    return lineName;

    //return QString("anonymous_%1").arg(info.functionStartLineNumber());
}

void Profiler::functionEntry(qint64 scriptId)
{
    positionDone();
    QScriptContext *ctx = engine()->currentContext();
    QScriptContextInfo info(ctx);
    //qDebug() << "push ->" << scriptId << info.functionName() << info.functionStartLineNumber();
    Function *f = determineCurrentFunction(currentScriptId.isEmpty() ? -1 : currentScriptId.top());
    QString infoFunctionName = info.functionName();
    if (f) {
        Action action;
        action.line = currentLine.top();
        action.callObject = scriptId;
        action.callFunction = functionName(ctx, scriptId);
        action.callFunctionLine = info.functionStartLineNumber();

        Action *a = 0;
        QList<Action>::iterator i = qBinaryFind(f->actions.begin(), f->actions.end(), action);
        if (i != f->actions.end()) {
            a = &f->actions[i - f->actions.begin()];
        } else {
            f->actions.append(action);
            a = &f->actions[f->actions.count() - 1];
        }
        currentFunction.push(a);
    }

    currentLine.push(0);
    currentFunctionLine.push(info.functionStartLineNumber());
    currentScriptId.push(scriptId);
    f = determineCurrentFunction(scriptId);
    if (!f) {
        Function fn;
        fn.startLine = info.functionStartLineNumber();
        fn.functionName = functionName(ctx, scriptId);
        QList<Function>::iterator i = qLowerBound(objects[scriptId].functions.begin(), objects[scriptId].functions.end(), fn);
        objects[scriptId].functions.insert(i, fn);
    }
}

void Profiler::functionExit(qint64 /*scriptId*/, const QScriptValue &)
{
    positionDone();

    currentFunctionLine.pop();
    currentLine.pop();
    currentScriptId.pop();

    if (!currentFunction.isEmpty())
        currentFunction.pop();
    if (!currentFunction.isEmpty()) {
        useconds_t current = getTimeOfDay();
        currentFunction.top()->cost += current - funTime;
        funTime = current;
        currentFunction.top()->called++;
    }
}

void Profiler::positionChange(qint64 scriptId, int lineNumber, int /*columnNumber*/)
{
    positionDone();
    if (first) {
        first = false;
        instTime = getTimeOfDay();
    }
    //qDebug() << "step" << scriptId << lineNumber << objects[scriptId].code[lineNumber-1];
    Function *f = determineCurrentFunction(scriptId);
    if (!f) {
        qWarning() << "bug";
        return;
    }
    QList<Action>::iterator i = qBinaryFind(f->actions.begin(), f->actions.end(), Action(lineNumber));
    if (i == f->actions.end())
        f->actions.append(Action(lineNumber));
    currentLine.pop();
    currentLine.push(lineNumber);
}

void Profiler::positionDone()
{
    Function *f = determineCurrentFunction(currentScriptId.isEmpty() ? -1 : currentScriptId.top());
    if (!f || currentLine.isEmpty())
        return;
    QList<Action>::iterator i = qBinaryFind(f->actions.begin(), f->actions.end(), Action(currentLine.top()));
    if (i == f->actions.end())
        return;

    suseconds_t current = getTimeOfDay();
    Action *action = &f->actions[i - f->actions.begin()];
    action->cost += current - instTime;
    instTime = current;
}

void Profiler::scriptLoad(qint64 id, const QString &program, const QString &fileName, int /*baseLineNumber*/ )
{
    Object obj;
    obj.code = program.split('\n');
    obj.fileName = fileName;
    objects[id] = obj;
}

}

#endif
