// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ActionNode.h"

namespace CoreUi
{
    ActionNode::ActionNode(const QString& name, QIcon icon, UiDefines::MenuNodeStyleMap map) :
        MenuNode(name, icon, map)
    {
    }

    ActionNode::~ActionNode()
    {
    }

    void ActionNode::NodeClicked()
    {
        emit ActionButtonClicked(id_);
    }
}