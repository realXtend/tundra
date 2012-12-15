// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifdef ANDROID

#include "Framework.h"

#include <vector>

class Framework;

class StaticPluginRegistry
{
public:
    void AddPluginMainFunction(void (*pluginMain)(Framework*))
    {
        pluginMainFunctions.push_back(pluginMain);
    }
    
    void RunPluginMainFunctions(Framework* fw)
    {
        for (std::vector<void (*)(Framework*)>::iterator i = pluginMainFunctions.begin(); i != pluginMainFunctions.end(); ++i)
            (*i)(fw);
    }

    std::vector<void (*)(Framework*)> pluginMainFunctions;
};

class StaticPluginHelper
{
public:
    StaticPluginHelper(void (*pluginMain)(Framework*))
    {
        Framework::StaticPluginRegistryInstance()->AddPluginMainFunction(pluginMain);
    }
};

#define DEFINE_STATIC_PLUGIN_MAIN(module) \
void module ## Main(Framework* fw)

#define REGISTER_STATIC_PLUGIN(module) \
extern "C" \
{ \
extern void module ## Main(Framework* fw);\
StaticPluginHelper module ## Helper(&module ## Main); \
}

#endif
