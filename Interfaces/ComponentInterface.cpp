// For conditions of distribution and use, see copyright notice in license.txt

#include "Framework.h"
#include "ComponentInterface.h"

namespace Foundation
{

ComponentInterface::ComponentInterface(Foundation::Framework *framework) : framework_(framework), parent_entity_(0)
{
}

ComponentInterface::ComponentInterface(const ComponentInterface &rhs) : QObject(), framework_(rhs.framework_), parent_entity_(rhs.parent_entity_)
{
}

ComponentInterface::~ComponentInterface()
{
}

}
