// For conditions of distribution and use, see copyright notice in license.txt

#include "Framework.h"
#include "SceneAPI.h"
#include "IComponentFactory.h"

#include "EC_WidgetCanvas.h"
#include "EC_WebView.h"
#include "EC_SlideShow.h"

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        fw->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_WidgetCanvas>));
        fw->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_WebView>));
        fw->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_SlideShow>));
    }
}
