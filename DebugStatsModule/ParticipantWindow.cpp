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
//#include "EntityComponent/EC_OpenSimPresence.h"
#include "EC_OpenSimPresence.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpacerItem>

#include "MemoryLeakCheck.h"

namespace DebugStats
{

ParticipantWindow::ParticipantWindow(Foundation::Framework *fw, QWidget *parent) :
    QWidget(parent), framework_(fw)
{
    setWindowTitle(tr("Particpants"));
    setAttribute(Qt::WA_DeleteOnClose);
    // Base layout for this widget
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    resize(250, 300);

    PopulateUserList();
}

ParticipantWindow::~ParticipantWindow()
{
//    qDeleteAll(entries_);
}

void ParticipantWindow::AddUserEntry(EC_OpenSimPresence *presence)
{
    if (!entries_.contains(presence->agentId))
    {
        // Create widget with horizontal layout for each user
        QWidget *entryWidget= new QWidget(this);
        entryWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        entryWidget->setLayout(new QHBoxLayout(entryWidget));

        // Add name label by default for each entry
        QLabel *label = new QLabel(presence->GetFullName().c_str(), entryWidget);
        entryWidget->layout()->addWidget(label);

        QVBoxLayout *l = static_cast<QVBoxLayout *>(layout());
        l->insertWidget(l->children().size() + 1, entryWidget);

        entries_[presence->agentId] = entryWidget;
    }
}

void ParticipantWindow::RemoveUserEntry(EC_OpenSimPresence *presence)
{
    if (entries_.contains(presence->agentId))
    {
        QWidget *entryWidget = entries_.take(presence->agentId);
        entryWidget->deleteLater();
        entryWidget = 0;
        /*
        QListIterator<QObject *> it(entryLayout->children());
        while(it.hasNext())
        {
            QWidget *w = dynamic_cast<QWidget *>(it.next());
            if (w)
                entryLayout->removeWidget(w);
        }
        layout()->removeItem(entryLayout);
        entryLayout->deleteLater();
        entryLayout = 0;
        */
        entries_.remove(presence->agentId);
    }
}

void ParticipantWindow::AddInfoField(const RexUUID &user_id, QWidget *widget)
{
    QMap<RexUUID, QWidget *>::iterator it = entries_.find(user_id);
    if (it != entries_.end())
        (*it)->layout()->addWidget(widget);
}

void ParticipantWindow::PopulateUserList()
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = **iter;
        boost::shared_ptr<EC_OpenSimPresence> ec_presence = entity.GetComponent<EC_OpenSimPresence>();
        if (ec_presence)
            AddUserEntry(ec_presence.get());
    }

    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    static_cast<QVBoxLayout*>(layout())->addSpacerItem(spacer);
}

}
