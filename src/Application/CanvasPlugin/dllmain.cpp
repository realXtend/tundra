// For conditions of distribution and use, see copyright notice in LICENSE

#include "CoreDefines.h"
#include "Framework.h"
#include "SceneAPI.h"
#include "IComponentFactory.h"
#include "EC_GraphicsViewCanvas.h"

extern "C"
{

DLLEXPORT void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw);
    fw->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_GraphicsViewCanvas>));
}

}
