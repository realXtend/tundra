/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   PythonScriptInstance.h
 *  @brief  Python script instance used with EC_Script.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "PythonScriptInstance.h"
#include "Entity.h"
#include "Scene.h"

#include <PythonQt.h>
#include <QFile>

#include "MemoryLeakCheck.h"

PythonScriptInstance::PythonScriptInstance(const QString &filename, Entity *entity) :
    filename_(filename),
    evaluated_(false)
{
    context_ = PythonQt::self()->createUniqueModule();

    // Add parent entity and scene to the context
    PythonQt::self()->addObject(context_, "me", entity);
    PythonQt::self()->addObject(context_, "scene", entity->ParentScene());
}

void PythonScriptInstance::Load()
{
    ///\todo implement
//    PythonQt::self()->importModule("sys").evalScript("sys.reload(" + filename_ + ")");
//    context_.evalScript("import sys\nsys.reload('" + moduleName_ + "')");
//    context_.evalScript("import " + moduleName_ + "\nreload(" + moduleName_ + ")");
//    context_.evalScript("import test\nreload(test)");
//    context_.evalScript("import sys\nprint sys.modules");
}

void PythonScriptInstance::Unload()
{
    ///\todo implement
}

void PythonScriptInstance::Run()
{
    if (QFile::exists(filename_) && !context_.isNull())
    {
        context_.evalFile(filename_);
        evaluated_ = true;
    }
}

