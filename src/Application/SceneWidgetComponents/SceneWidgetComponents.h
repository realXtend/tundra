// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "SceneWidgetComponentsApi.h"
#include "IModule.h"

#include "FrameworkFwd.h"
#include "InputFwd.h"
#include "SceneFwd.h"

class SCENEWIDGET_MODULE_API SceneWidgetComponents : public IModule
{

Q_OBJECT

public:
    /// Constructor
    SceneWidgetComponents();

    /// Deconstructor. 
    virtual ~SceneWidgetComponents();

    /// IModule override.
    virtual void Initialize();

private slots:
    void OnMouseEvent(MouseEvent *mEvent);
};
