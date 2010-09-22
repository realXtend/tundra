/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   PythonScriptModule.h
 *  @brief  Python script instance used with EC_Script.
 */

#include "StableHeaders.h"
#include "PythonScriptInstance.h"

#include "Entity.h"

#include <PythonQt.h>

#include <QFile>

PythonScriptInstance::PythonScriptInstance(const QString &filename, Scene::Entity *entity) :
    filename_(filename)
{
    //context_ = PythonQt::self()->createUniqueModule();
    moduleName_ = filename_.mid(3, filename_.length() - 4);
    context_ = PythonQt::self()->createModuleFromScript(moduleName_);
    PythonQt::self()->addObject(context_, "me", entity);
}

void PythonScriptInstance::Reload()
{
//    PythonQt::self()->importModule("sys").evalScript("sys.reload(" + filename_ + ")");
//    context_.evalScript("import sys\nsys.reload('" + moduleName_ + "')");
//    context_.evalScript("import " + moduleName_ + "\nreload(" + moduleName_ + ")");
//    context_.evalScript("import test\nreload(test)");
//    context_.evalScript("import sys\nprint sys.modules");
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
