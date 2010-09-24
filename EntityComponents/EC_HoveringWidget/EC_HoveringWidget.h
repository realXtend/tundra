/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_HoveringWidget.h
 *  @brief  EC_HoveringWidget shows a hovering widget attached to an entity.
 *  @note   The entity must have EC_OgrePlaceable component available in advance.
 */

#ifndef incl_EC_HoveringWidget_h
#define incl_EC_HoveringWidget_h

#include "IComponent.h"
#include "Declare_EC.h"
#include "Vector3D.h"

#include <QSizeF>
#include <QPixmap>

namespace Ogre
{
    class SceneNode;
    class BillboardSet;
    class Billboard;
}

namespace OgreRenderer
{
    class Renderer;
}

QT_BEGIN_NAMESPACE
class QPushButton;
class QTimeLine;
class QWidget;
class QGraphicsProxyWidget;
class QTimer;
QT_END_NAMESPACE

class HoveringButtonsController;
class HoveringNameController;
class DetachedWidgetController;

/// Shows a hovering widget attached to an entity.
class EC_HoveringWidget : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_HoveringWidget);

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_HoveringWidget(IModule *module);

public:
    /// Destructor.
    ~EC_HoveringWidget();

    //! @return billboard that shows buttons
    Ogre::Billboard* const GetButtonsBillboard() const { return buttonsbillboard_; }

    //! @return billboardset that owns the buttons billboard
    Ogre::BillboardSet* const GetButtonsBillboardSet()const { return buttonsbillboardSet_; }

    //! @return get size of the button billboard in screenspace
    QSizeF GetButtonsBillboardScreenSpaceSize() const { return bb_buttons_size_view; }

public slots:
    //!called when widget is hovered
    void HoveredOver();

    //!Initializes billboards, must be called before use
    void InitializeBillboards();

    //Disables/enables widget
    void SetDisabled(bool val);

    //! @ return true if widget is disabled
    bool IsDisabled() const { return disabled_;}

    //! set buttons disabled, will still show the text
    void SetButtonsDisabled(bool val);

    //! @return true if buttons ase disabled
    bool IsButtonsDisabled(){return buttons_disabled_;}

    //! show/hide buttons
    void ShowButtons(bool val);

    /// Shows the widget
    void Show();

    /// Shows the widget with animation (not currently working properly)
    void AnimatedShow();

    //! notifies that entity was clicked
    void EntityClicked(int msec_to_show = 5000);

    void Detach();
    void Attach();

    //! called when the widget is clicked in 3D scene
    //! @param x coordinate (clamped in 0-1) 
    //! @param y coordinate (clamped in 0-1)
    void WidgetClicked(float x, float y);

    /// Hides the hovering text
    void Hide();

    /// Hides the hovering text with animation.
    void AnimatedHide();

    /// Returns if the hovering text is visible or not.
    /// @true If the hovering text is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;

    /// Sets the text to be shown.
    /// @param text Text to be shown.
    void SetText(const QString &text);

    // Add button to the widget
    //! @button button to add
    void AddButton(QPushButton &button);

    //! set widgets distance from the camera
    void SetCameraDistance(float dist);

    //! get widgets distance from the camera
    float GetCameraDistance() const {return cam_distance_;}

    //! Adjust what to show, might disable button or set the widget invisible.
    void AdjustWidgetinfo();

    //! scale widget to have static screenspace size
    //@param bset billboardset, used to get worldtransforms
    //@param b billboard that we scale
    //@param size size in screenspace
    //@param next_to_nametag, if true, will try to place the billboard next to the namebillboard
    void ScaleWidget(Ogre::BillboardSet& bset, Ogre::Billboard& b, const QSizeF& size,bool next_to_nametag=false);

private slots:
    /// Updates the animation
    /// @param step
    void UpdateAnimationStep(int step);

    /// Finishes the animation.
    void AnimationFinished();

    /// Redraws the hovering text with the current text, font and color.
    void Redraw();

    /// Check for name width
    int CheckNameTagWidth() const;

    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();

private:
    /// Returns pixmap with widget rendered to it
    QPixmap GetPixmap(QWidget &w, const QRect &dimensions);

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;

    /// Ogre billboard set.
    Ogre::BillboardSet *namebillboardSet_;

    /// Ogre billboard set.
    Ogre::BillboardSet *buttonsbillboardSet_;

    /// Ogre billboard.
    Ogre::Billboard *namebillboard_;

    /// Ogre billboard.
    Ogre::Billboard *buttonsbillboard_;

    /// Name of the material used for the name billboard set.
    std::string namematerialName_;

    /// Name of the material used for the buttons billboard set.
    std::string buttonsmaterialName_;

    /// Name of the 1. texture used for hovering widget.
    std::string hoveringTexture1Name_;

    /// Name of the 2. texture used for hovering widget.
    std::string hoveringTexture2Name_;

    // Visibility animation timeline.
    QTimeLine *visibility_animation_timeline_;

    // Timed visibility timer
    QTimer *visibility_timer_;

    QTimer *hovering_timer_;

    QGraphicsProxyWidget *proxy_;

    qreal hovering_time_;

    bool disabled_;

    bool buttons_disabled_;

    bool buttons_visible_;

    bool detached_;

    HoveringNameController* namewidget_;

    DetachedWidgetController* detachedwidget_;

    HoveringButtonsController *buttonswidget_;

    QSizeF bb_name_size_view;
    QSizeF bb_buttons_size_view;

    QPointF name_scr_pos_;

    float cam_distance_;

    float bb_rel_posy;
};

#endif
