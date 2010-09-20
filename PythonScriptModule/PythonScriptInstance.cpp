/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   PythonScriptModule.h
 *  @brief  Python script instance used with EC_Script.
 */

#include "StableHeaders.h"
#include "PythonScriptInstance.h"

#include <PythonQt.h>

#include <QFile>

PythonScriptInstance::PythonScriptInstance(const QString &filename) : filename_(filename)
{
    context_ = PythonQt::self()->createUniqueModule();
}

void PythonScriptInstance::Reload()
{
}

void PythonScriptInstance::Unload()
{
}

void PythonScriptInstance::Run()
{
    if (QFile::exists(filename_) && !context_.isNull())
        context_.evalFile(filename_);
}

void PythonScriptInstance::Stop()
{
}
