// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "FrameworkFwd.h"
#include "InputFwd.h"
#include "SceneFwd.h"
#include "OgreModuleFwd.h"

#include <string>
#include <vector>

class EC_Mesh;
class EC_Placeable;
class EC_Camera;
class EC_WebView;
class EC_WidgetCanvas;
class EC_Script;
class EC_DynamicComponent;

namespace PythonScript
{
    class PythonEngine;
    typedef boost::shared_ptr<PythonEngine> PythonEnginePtr;
}

namespace OgreRenderer 
{
    class OgreRenderingModule;
    class Renderer;
}

class ScriptAsset;
typedef boost::shared_ptr<ScriptAsset> ScriptAssetPtr;

typedef boost::shared_ptr<std::vector<void(*)(char*)> > StdFunctionVectorPtr;