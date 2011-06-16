/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_QML.cpp
 *  @brief  EC_QML is a component for showing QML-elements in a texture.
 *  @note   no notes
 */


#include "StableHeaders.h"
#include "EC_QML.h"
#include "IModule.h"
#include "Entity.h"
#include "LoggingFunctions.h"
#include "RenderServiceInterface.h"

#include "InputAPI.h"
#include "MemoryLeakCheck.h"

#include <QApplication>
#include <QMouseEvent>
#include <QEvent>

DEFINE_POCO_LOGGING_FUNCTIONS("EC_QML")

EC_QML::EC_QML(IModule *module) :
    IComponent(module->GetFramework()),
    renderSubmeshIndex(this, "Render Submesh", 0),
    interactive(this, "Interactive", false),
    qmlsource(this, "QML source", ""),
    renderinterval(this, "Render interval", 40),
    qml_ready(false)
{
    renderTimer_ = new QTimer();

    // Connect signals from IComponent
    connect(this, SIGNAL(ParentEntitySet()), SLOT(PrepareQML()), Qt::UniqueConnection);
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(AttributeChanged(IAttribute*, AttributeChange::Type)), Qt::UniqueConnection);
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(ServerHandleAttributeChange(IAttribute*, AttributeChange::Type)), Qt::UniqueConnection);
    QObject::connect(renderTimer_, SIGNAL(timeout()), this, SLOT(Render()));

    renderTimer_->setInterval(getrenderinterval());
    renderer_ = module->GetFramework()->GetService<Foundation::RenderServiceInterface>();

    //Create a new input context that EC_QML will use to fetch input.
    input_ = module->GetFramework()->Input()->RegisterInputContext("QMLInput", 100);

    // To be sure that Qt doesn't play tricks on us and miss a mouse release when we're in FPS mode,
    // grab the mouse movement input over Qt.
    input_->SetTakeMouseEventsOverQt(true);

    // Listen on mouse input signals.
    connect(input_.get(), SIGNAL(MouseEventReceived(MouseEvent *)), this, SLOT(HandleMouseInputEvent(MouseEvent *)));
}

EC_QML::~EC_QML()
{
    /// \todo Write your own EC_Component destructor here
    SAFE_DELETE_LATER(qmlview_);
    SAFE_DELETE_LATER(renderTimer_);
}

void EC_QML::PrepareQML()
{
    // Don't do anything if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;

    // Get parent and connect to the signals.
    Scene::Entity *parent = GetParentEntity();
    assert(parent);
    if (parent)
    {
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(ComponentRemoved(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
    }
    else
    {
        LogError("PrepareComponent: Could not get parent entity pointer!");
        return;
    }

    // Create EC_Mesh component.
    mesh_ = GetOrCreateMeshComponent();
    if (!mesh_)
    {
        // Wait for EC_Mesh to be added.
        connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), SLOT(ComponentAdded(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
        return;
    }

    EC_Placeable *placeable = GetOrCreatePlaceableComponent();
    if (!placeable)
    {
        // Wait for EC_Placeable to be added.
        connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), SLOT(ComponentAdded(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
        return;
    }

    // Get or create local EC_3DCanvas component
    canvas_ = GetOrCreateSceneCanvasComponent();
    if (!canvas_)
    {
        LogError("PrepareComponent: Could not get or create EC_3DCanvas component!");
        return;
    }
    else
    {
        qmlview_ = new QDeclarativeView();
        qmlview_->setSource(QUrl(getqmlsource()));
        QObject::connect(qmlview_, SIGNAL(statusChanged(QDeclarativeView::Status)), this, SLOT(QMLStatus(QDeclarativeView::Status)));

        canvas_->SetSubmesh(getrenderSubmeshIndex());
        canvas_->SetWidget(qmlview_);
    }
}

void EC_QML::QMLStatus(QDeclarativeView::Status qmlstatus)
{
    if (framework_->IsHeadless())
        return;
    if (qmlstatus == QDeclarativeView::Ready)
    {
        LogInfo("QDeclarativeView has loaded and created the QML component.");

        if (qmlview_->size().width() > 0 && qmlview_->size().height() > 0)
        {
            renderTimer_->start();
            qml_ready = true;
        }
        else
        {
            renderTimer_->stop();
            LogInfo("Unable to draw the QML component, because it has no size defined!");
            qml_ready = false;
        }
    }

    else if (qmlstatus == QDeclarativeView::Null)
    {
        renderTimer_->stop();
        LogInfo("QDeclarativeView has no source set.");
        qml_ready = false;
    }
    else if (qmlstatus == QDeclarativeView::Loading)
    {
        renderTimer_->stop();
        LogInfo("QDeclarativeView is loading network data.");
        qml_ready = false;
    }
    else if (qmlstatus == QDeclarativeView::Error)
    {
        renderTimer_->stop();
        LogInfo("One or more errors has occurred.");
        qml_ready = false;
    }
    else
    {
        renderTimer_->stop();
        qml_ready = false;
    }
}

void EC_QML::HandleMouseInputEvent(MouseEvent *mouse)
{
    if(mouse->eventType == MouseEvent::MousePressed && mouse->button == MouseEvent::LeftButton)
    {
        RaycastResult* result;
        if (renderer_)
        {
            result = renderer_->Raycast(mouse->X(), mouse->Y());

            if(result->entity_==GetParentEntity())
            {
                int xpos = (int)qmlview_->size().width() * result->u_;
                int ypos = (int)qmlview_->size().height() * result->v_;

                QPoint mousepoint(xpos, ypos);
                QMouseEvent event = QMouseEvent(QEvent::MouseButtonPress, qmlview_->mapFromScene(mousepoint), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(qmlview_->viewport(), &event);
            }
        }
    }

    if(mouse->eventType == MouseEvent::MouseReleased && mouse->button == MouseEvent::LeftButton)
    {
        RaycastResult* result;
        if (renderer_)
        {
            result = renderer_->Raycast(mouse->X(), mouse->Y());

            if(result->entity_==GetParentEntity())
            {
                int xpos = (int)qmlview_->size().width() * result->u_;
                int ypos = (int)qmlview_->size().height() * result->v_;

                QPoint mousepoint(xpos, ypos);
                QMouseEvent event = QMouseEvent(QEvent::MouseButtonRelease, qmlview_->mapFromScene(mousepoint), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(qmlview_->viewport(), &event);
            }
        }
    }

    if(mouse->eventType == MouseEvent::MouseDoubleClicked && mouse->button == MouseEvent::LeftButton)
    {
        RaycastResult* result;
        if (renderer_)
        {
            result = renderer_->Raycast(mouse->X(), mouse->Y());

            if(result->entity_==GetParentEntity())
            {
                int xpos = (int)qmlview_->size().width() * result->u_;
                int ypos = (int)qmlview_->size().height() * result->v_;

                QPoint mousepoint(xpos, ypos);
                QMouseEvent event = QMouseEvent(QEvent::MouseButtonDblClick, qmlview_->mapFromScene(mousepoint), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(qmlview_->viewport(), &event);
            }
        }
    }

    if(mouse->eventType == MouseEvent::MouseMove && mouse->IsLeftButtonDown())
    {
        RaycastResult* result;
        if (renderer_)
        {
            result = renderer_->Raycast(mouse->X(), mouse->Y());

            if(result->entity_==GetParentEntity())
            {
                int xpos = (int)qmlview_->size().width() * result->u_;
                int ypos = (int)qmlview_->size().height() * result->v_;

                QPoint mousepoint(xpos, ypos);
                QMouseEvent event = QMouseEvent(QEvent::MouseMove, qmlview_->mapFromScene(mousepoint), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(qmlview_->viewport(), &event);
            }
        }
    }
}

void EC_QML::AttributeChanged(IAttribute *attribute, AttributeChange::Type changeType)
{
    if (attribute == &qmlsource)
    {
        qml_ready = false;
        qmlview_->setSource(QUrl(getqmlsource()));
    }
    if (attribute == &renderSubmeshIndex)
    {
        canvas_->SetSubmesh(getrenderSubmeshIndex());
    }
    if (attribute == &renderinterval)
    {
        renderTimer_->setInterval(getrenderinterval());
    }
}


void EC_QML::Render()
{
    // Don't do anything if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;

    if (qml_ready)
    {
            canvas_->SetSubmesh(getrenderSubmeshIndex());
            canvas_->Update();
    }
}

void EC_QML::ServerHandleAttributeChange(IAttribute *attribute, AttributeChange::Type changeType)
{
    if (attribute == &qmlsource)
    {
        qml_ready = false;
        qmlview_->setSource(QUrl(getqmlsource()));
    }
    if (attribute == &renderSubmeshIndex)
    {
        canvas_->SetSubmesh(getrenderSubmeshIndex());
    }
    if (attribute == &renderinterval)
    {
        renderTimer_->setInterval(getrenderinterval());
    }
}


void EC_QML::ComponentAdded(IComponent *component, AttributeChange::Type change)
{
    //LogInfo("In ComponentAdded -slot");
    if(component->TypeName()==EC_Mesh::TypeNameStatic())
    {
        PrepareQML();
    }
    else if (component->TypeName()==EC_Placeable::TypeNameStatic())
    {
    }
}

EC_Mesh* EC_QML::GetOrCreateMeshComponent()
{

    if (GetParentEntity())
    {
            IComponent *iComponent =  GetParentEntity()->GetOrCreateComponent("EC_Mesh", AttributeChange::LocalOnly, false).get();
            EC_Mesh *mesh = dynamic_cast<EC_Mesh*>(iComponent);
            return mesh;

    }
    else
        LogError("Couldn't get parent entity, returning 0");
    return 0;

}

EC_3DCanvas* EC_QML::GetOrCreateSceneCanvasComponent()
{
    IComponent *iComponent = GetParentEntity()->GetOrCreateComponent("EC_3DCanvas", AttributeChange::LocalOnly, false).get();
    EC_3DCanvas *canvas = dynamic_cast<EC_3DCanvas*>(iComponent);

    /// \!TODO some error handling would be nice..
    return canvas;
}

EC_Placeable *EC_QML::GetOrCreatePlaceableComponent()
{
    if (!GetParentEntity())
        return 0;
    IComponent *iComponent = GetParentEntity()->GetOrCreateComponent("EC_Placeable", AttributeChange::LocalOnly, false).get();
    EC_Placeable *placeable = dynamic_cast<EC_Placeable*>(iComponent);
    return placeable;
}

void EC_QML::ComponentRemoved(IComponent *component, AttributeChange::Type change)
{

}
