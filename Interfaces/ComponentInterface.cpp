// For conditions of distribution and use, see copyright notice in license.txt

#include "Framework.h"
#include "ComponentInterface.h"

namespace Foundation
{

ComponentInterface::ComponentInterface(const Foundation::Framework *framework) : framework_(framework)
{
}

ComponentInterface::ComponentInterface(const ComponentInterface &rhs) : QObject(), framework_(rhs.framework_)
{
}
ComponentInterface::~ComponentInterface()
{
}

}
