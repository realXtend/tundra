/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ParticipantWindow.cpp
 *  @brief  ParticipantWindow shows list of users in the world.
 */

#include "StableHeaders.h"

#include "DebugOperatorNew.h"
#include "ParticipantWindow.h"

#include "Framework.h"
#include "SceneManager.h"
#include "EntityComponent/EC_OpenSimPresence.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "MemoryLeakCheck.h"

namespace DebugStats
{

ParticipantWindow::ParticipantWindow(Foundation::Framework *fw, QWidget *parent) :
    QWidget(parent), framework_(fw)
{
    // Base layout for this widget
    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // Vertical layout for username labels
    usernameLayout_ = new QVBoxLayout(this);
    layout->addLayout(usernameLayout_, 0, 0);

    resize(300,400);

    PopulateUsernameList();
}

ParticipantWindow::~ParticipantWindow()
{
}

void ParticipantWindow::PopulateUsernameList()
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = **iter;
        boost::shared_ptr<RexLogic::EC_OpenSimPresence> ec_presence = entity.GetComponent<RexLogic::EC_OpenSimPresence>();
        if (ec_presence)
        {
            QLabel *label = new QLabel(ec_presence->GetFullName().c_str(), this);
            usernameLayout_->addWidget(label);
        }
    }
}

void ParticipantWindow::AddUser(RexLogic::EC_OpenSimPresence *presence)
{
    if (!users_.contains(presence->agentId))
    {
        QLabel *label = new QLabel(presence->GetFullName().c_str(), this);
        usernameLayout_->addWidget(label);
        users_[presence->agentId] = label;
    }
}

void ParticipantWindow::RemoveUser(RexLogic::EC_OpenSimPresence *presence)
{
    if (users_.contains(presence->agentId))
    {
        QLabel *label = users_.take(presence->agentId);
        SAFE_DELETE(label);
    }
}

}
