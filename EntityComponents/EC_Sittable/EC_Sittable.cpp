#include "StableHeaders.h"
#include "EC_Sittable.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "TundraLogicModule.h"
#include "Client.h"
#include "SceneManager.h"
#include "SceneInteract.h"
#include "RenderServiceInterface.h"
#include "FrameAPI.h"

#include <QLine>
#include <QPoint>
#include <QTimer>

// Utility functions
QPointF RotateAroundOrigin(const QPointF & point, const QPointF & origin, float angle)
{
    QPointF result;
    result.setX(origin.x() + (cos(angle * DEGTORAD) * (point.x() - origin.x()) - sin(angle * DEGTORAD) * (point.y() - origin.y())));
    result.setY(origin.y() + (sin(angle * DEGTORAD) * (point.x() - origin.x()) + cos(angle * DEGTORAD) * (point.y() - origin.y())));
    return result;
}

namespace TundraLogic
{
    class Client;
}

EC_Sittable::EC_Sittable(IModule *module):
    IComponent(module->GetFramework()),
    canCheck_(false),
    canUpdate_(false),
    sitPosition(this, "Sit position", Vector3df()),
    sitOrientation(this, "Sit orientation", Vector3df()),
    autoWalk(this, "Auto walk", false),
    occupied(this, "Occupied", false)
{
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(InitComponent()));
}

void EC_Sittable::InitComponent()
{
    if (!ViewEnabled())
        return;
    if (GetFramework()->IsHeadless())
        return;

    Scene::Entity *parentEntity = GetParentEntity();
    if (!parentEntity)
        return;

    placeable_ = parentEntity->GetComponent<EC_Placeable>();
    mesh_ = parentEntity->GetComponent<EC_Mesh>();

    if (placeable_ && mesh_)
    {
        connect(placeable_.get(), SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(HandleAttributeChange(IAttribute*, AttributeChange::Type)));
        CalculateSitPosAndRot();
    }

    SceneInteractWeakPtr sceneInteract = GetFramework()->Scene()->GetSceneIteract();
    if (!sceneInteract.isNull())
    {
        connect(sceneInteract.data(), SIGNAL(EntityMousePressed(Scene::Entity*, Qt::MouseButton, RaycastResult*)), 
                SLOT(EntityClicked(Scene::Entity*, Qt::MouseButton, RaycastResult*)));
    }
    
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(HandleAttributeChange(IAttribute*, AttributeChange::Type)));
    connect(framework_->Frame(), SIGNAL(Updated(float)), SLOT(Update(float)));
}

EC_Sittable::~EC_Sittable()
{
    disconnect(this, SIGNAL(ParentEntitySet()), this, SLOT(InitComponent()));
}

void EC_Sittable::HandleAttributeChange(IAttribute *attr, AttributeChange::Type change)
{
    if (GetFramework()->IsHeadless())
        return;

    if (attr == &placeable_.get()->transform)
        CalculateSitPosAndRot();

    if (attr == &autoWalk)
        if (!autoWalk.Get())
            StopMovement();
}

void EC_Sittable::CalculateSitPosAndRot()
{
    std::vector<boost::shared_ptr<EC_Sittable> > compVec = GetParentEntity()->GetComponents<EC_Sittable>();

    Vector3df worldSize = mesh_.get()->GetWorldSize();
    Vector3df position = placeable_.get()->gettransform().position;
    float rotZ = placeable_.get()->gettransform().rotation.z;
    float offset = 0.7f * 0.2f; // 0.7 is Avatar's width, 0.2 a magic number

    int sittablePos = 0;
    for (std::vector<boost::shared_ptr<EC_Sittable> >::iterator i = compVec.begin(); i != compVec.end(); ++i)
    {
        if ((*i).get() == this)
            break;
        sittablePos++;
    }

    Vector3df sitpos(0,0,position.z + worldSize.z/2);
    Vector3df sitort(0,0,0);
    if (worldSize.x > worldSize.y)
    {
        QPointF x1 = RotateAroundOrigin(QPointF(position.x - worldSize.x/2, position.y + worldSize.y/2 - 0.2), QPointF(position.x, position.y), rotZ);
        QPointF x2 = RotateAroundOrigin(QPointF(position.x + worldSize.x/2, position.y + worldSize.y/2 - 0.2), QPointF(position.x, position.y), rotZ);
        QLineF line(x1, x2);
        float seats = floor(line.length() / 0.7);
        sitpos.x = line.pointAt(sittablePos / seats + offset).x();
        sitpos.y = line.pointAt(sittablePos / seats + offset).y();
        sitort.z = rotZ + 90;
    }
    else
    {
        QPointF y1 = RotateAroundOrigin(QPointF(position.y - worldSize.y/2, position.x + worldSize.x/2 - 0.2), QPointF(position.x, position.y), rotZ);
        QPointF y2 = RotateAroundOrigin(QPointF(position.y + worldSize.y/2, position.x + worldSize.x/2 - 0.2), QPointF(position.x, position.y), rotZ);
        QLineF line(y1, y2);
        float seats = floor(line.length() / 0.7);
        sitpos.x = line.pointAt(sittablePos / seats + offset).x();
        sitpos.y = line.pointAt(sittablePos / seats + offset).y();
        sitort.z = rotZ + 180;
    }

    sitPosition.Set(sitpos, AttributeChange::Default);
    sitOrientation.Set(sitort, AttributeChange::Default);
    EnableChecking();
}

void EC_Sittable::GoTowardsTarget()
{
    if (!canUpdate_)
        return;

    Scene::Entity *avatar = GetOwnAvatar();
    if (avatar)
    {
        boost::shared_ptr<EC_Placeable> avatarPlaceable = avatar->GetComponent<EC_Placeable>();
        if (avatarPlaceable)
        {
            Vector3df avatarPos = avatarPlaceable_.get()->gettransform().position;
            Vector3df targetPos = sitPosition.Get();
            float avrotZ = avatarPlaceable_.get()->gettransform().rotation.z;
            Vector3df relTargetPos = targetPos - avatarPos;
            float distance = sqrt(relTargetPos.x*relTargetPos.x + relTargetPos.y*relTargetPos.y);

            // Since euler angles are in the range -180 to 180, we use acos to get an angle that will be always from 0 to 180
            float angle = acos(relTargetPos.x/distance) * 180 / 3.1415;
            // If the target is in the third or fourth quadrant, get the negative angle
            if (relTargetPos.y < 0)
                angle *= -1;

            if (distance > 2 && distance < 20)
            {
                avatar->Exec(2, "Move", "forward");

                if (avrotZ > angle + 5)
                    avatar->Exec(2, "Rotate", "right");
                else
                    avatar->Exec(2, "StopRotate", "right");

                if (avrotZ < angle - 5)
                    avatar->Exec(2, "Rotate", "left");
                else
                    avatar->Exec(2, "StopRotate", "left");
            }
            else
                StopMovement();
        }
    }
}

Scene::Entity * EC_Sittable::GetOwnAvatar()
{
    Scene::ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
        return 0;
    int connectionID;
    TundraLogic::TundraLogicModule *tundra = GetFramework()->GetModule<TundraLogic::TundraLogicModule>();
    if (!tundra)
        return 0;

    connectionID = tundra->GetClient().get()->GetConnectionID();
    Scene::Entity *ent = scene.get()->GetEntityByNameRaw("Avatar" + QString::number(connectionID));
    if (ent)
        return ent;
    else
        return 0;
}

void EC_Sittable::EntityClicked(Scene::Entity* entity, Qt::MouseButton button, RaycastResult* result)
{
    if (!autoWalk.Get())
        return;
    if (!GetParentEntity())
        return;
    if (!result)
        return;
    if (button != Qt::LeftButton)
        return;
    if (occupied.Get())
        return;

    if (entity == GetParentEntity())
    {
        Scene::Entity *avatar = GetOwnAvatar();
        if (avatar)
        {
            boost::shared_ptr<EC_Placeable> avatarPlaceable = avatar->GetComponent<EC_Placeable>();
            if (avatarPlaceable)
                canUpdate_ = true;
        }
    }
}

void EC_Sittable::Update(float frametime)
{
    if (canCheck_)
        CheckIfOccupied();

    if (!autoWalk.Get() || !canUpdate_)
        return;

    GoTowardsTarget();
}

void EC_Sittable::CheckIfOccupied()
{
    QList<Scene::Entity*> avatars = GetFramework()->Scene()->GetDefaultScene()->GetEntitiesWithComponentRaw("EC_Avatar");
    if (!avatars.isEmpty())
    {
        for (QList<Scene::Entity*>::iterator i = avatars.begin(); i != avatars.end(); ++i)
        {
            boost::shared_ptr<EC_Placeable> avplaceable = (*i)->GetComponent<EC_Placeable>();
            if (avplaceable)
            {
                if (sitPosition.Get().getDistanceFrom(avplaceable.get()->gettransform().position) < 0.7f)
                {
                    occupied.Set(true, AttributeChange::Default);
                    break;
                }
                else
                    if (occupied.Get())
                        occupied.Set(false, AttributeChange::Default);
            }
        }
        canCheck_ = false;
        QTimer::singleShot(100, this, SLOT(EnableChecking()));

    }
}

void EC_Sittable::EnableChecking()
{
    canCheck_ = true;
}

void EC_Sittable::StopMovement()
{
    canUpdate_ = false;

    Scene::Entity *avatar = GetOwnAvatar();
    if(avatar)
    {
        avatar->Exec(2, "Stop", "all");
        avatar->Exec(2, "StopRotate", "left");
        avatar->Exec(2, "StopRotate", "right");
    }
}
