// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ActionNode.h"

namespace CoreUi
{
    ActionNode::ActionNode(const QString& name) :
            MenuNode(name)
    {
        connect(GetMenuButton(), SIGNAL( clicked() ), SLOT( NodeClicked() ));
    }

    void ActionNode::NodeClicked()
    {
        emit ActionButtonClicked(id_);
    }
}