// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ActionNode.h"

namespace CoreUi
{
    ActionNode::ActionNode(const QString& name, const QString &icon) : MenuNode(name, icon)
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
