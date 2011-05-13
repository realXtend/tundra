// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "OgreRenderingModule.h"
#include "OgreMaterialAsset.h"
#include "FrameAPI.h"
#include "Entity.h"
#include "EC_Material.h"
#include "LoggingFunctions.h"

EC_Material::EC_Material(Framework *fw) :
    IComponent(fw),
    parameters(this, "Parameters", QVariantList()),
    inputMat(this, "Input Material", ""),
    outputMat(this, "Output Material", "")
{
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(OnAttributeUpdated(IAttribute*)));
}

EC_Material::~EC_Material()
{
}

void EC_Material::OnAttributeUpdated(IAttribute* attribute)
{
}

void EC_Material::ProcessMaterial()
{
}
