/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneInteract.h
 *  @brief  Transforms generic mouse and keyboard input events to 
 *          input-related Entity Action for scene entities.
 */

#ifndef incl_Scene_sceneInteracth
#define incl_Scene_sceneInteracth

#include "FrameworkFwd.h"
#include "SceneFwd.h"
#include "InputFwd.h"

#include <QObject>

/// Transforms generic mouse and keyboard input events to input-related entity action for scene entities and Qt signals. 
/**
<table class="header"><tr><td>
<h2>SceneInteract</h2>

Transforms generic mouse and keyboard input events to input-related entity action for scene entities and Qt signals. 
Performs a raycast to the mouse position each frame and executes entity actions depending on the result.

Owned by SceneAPI.

<b>Local entity actions executed to the hit entity:</b>
<ul>
<li>"MouseHoverIn" - Executed when mouse hover enters the entity.
<div>No parameters.</div>
<li>"MouseHover" - Executed when mouse hovers on the entity.
<div>No parameters</div>
<li>"MouseHoverOut" - Executed when mouse hover leaves the entity.
<div></div>
<li>"MousePress" - Executed when mouse is clicked on the entity.
<div>String parameters: (int)"Qt::MouseButton", (float,float,float)"x,y,z", (int)"submesh index"</div>
<li>
<div></div>
<li>
<div></div>
</ul>

<b>Qt signals emitted by SceneInteract object:</b>
<ul>
<li>EntityClicked(Entity*, Qt::MouseButton, RaycastResult*) - Emitted when mouse is clicked on the entity.
<div>Parameters: hit entity, clicked mouse button, raycast result or the hit.</div>
</ul>

</td></tr></table>
*/
class SceneInteract : public QObject
{
    Q_OBJECT

public:
    /// Constructor. This object does not have a parent. It is stored in a QWeakPointer in SceneAPI and released in its dtor.
    SceneInteract();

    /// Destructor.
    ~SceneInteract() {}

    /// Initialize this object. Must be done for this object to work. Called by SceneAPI
    /** @param Framework Framework pointer. */
    void Initialize(Framework *framework);

    /// PostInitialize this object. Must be done after modules have been loaded. Called by SceneAPI.
    void PostInitialize();

signals:
    /// Emitted when scene was clicked and raycast hit an entity.
    /** @param entity Hit entity.
        @param Clicked mouse button
        @param result Raycast result data object.
    */
    void EntityClicked(Entity *entity, Qt::MouseButton button, RaycastResult *result);

    /// Emitted when scene was clicked and raycast hit an entity.
    /** @param entity Hit entity.
        @param Possible mouse button held down during the move.
        @param result Raycast result data object.
    */
    void EntityMouseMove(Entity *entity, Qt::MouseButton button, RaycastResult *result);

    /// Emitted when scene was clicked and raycast hit an entity.
    /** @param entity Hit entity.
        @param Released mouse button.
        @param result Raycast result data object.
    */
    void EntityClickReleased(Entity *entity, Qt::MouseButton button, RaycastResult *result);

private:
    /// Performs raycast to last known mouse cursor position.
    /** @return Result of the raycast. */
    RaycastResult* Raycast();

    Framework *framework_; ///< Framework.
    InputContextPtr input_; ///< Input context.
    RendererServiceWeakPtr renderer_; ///< Renderer pointer.
    int lastX_; ///< Last known mouse cursor's x position.
    int lastY_; ///< Last known mouse cursor's y position.
    bool itemUnderMouse_; ///< Was there widget under mouse in last known position.
    EntityWeakPtr lastHitEntity_; ///< Last entity raycast has hit.

private slots:
    /// Executes "MouseHover" action each frame is raycast has hit and entity.
    void Update();

    /// Handles key events from input service.
    /** @param e Key event. */
    void HandleKeyEvent(KeyEvent *e);

    /// Handles mouse events from input service.
    /** @param e Mouse event. */
    void HandleMouseEvent(MouseEvent *e);
};

#endif
