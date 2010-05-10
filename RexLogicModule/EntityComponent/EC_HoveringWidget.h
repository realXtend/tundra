/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_HoveringWidget.h
 *  @brief  EC_HoveringWidget shows a hovering Widget attached to an entity.
 *  @note   The entity must have EC_OgrePlaceable component available in advance.
*/

#ifndef incl_EC_HoveringWidget_h
#define incl_EC_HoveringWidget_h

#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "Vector3D.h"
#include "RexLogicModuleApi.h"

#include "HoveringNameController.h"
#include "HoveringButtonsController.h"

#include <QFont>
#include <QColor>
#include <QLinearGradient>
#include <QSizeF>


//Needs refactor
namespace Ogre
{
    class SceneNode;
    class BillboardSet;
    class Billboard;
    class MaterialPtr;
}

namespace OgreRenderer
{
    class Renderer;
}

class QPushButton;

QT_BEGIN_NAMESPACE
class QTimeLine;
QT_END_NAMESPACE
class QWidget;
namespace RexLogic
{


    class REXLOGIC_MODULE_API EC_HoveringWidget : public Foundation::ComponentInterface
    {
        Q_OBJECT
        DECLARE_EC(EC_HoveringWidget);

    private:
        /// Constuctor.
        /// @param module Owner module.
        explicit EC_HoveringWidget(Foundation::ModuleInterface *module);

    public:
        /// Destructor.
        ~EC_HoveringWidget();

        /// Sets postion for the chat bubble.
        /// @param position Position.
        /// @note The position is relative to the entity to which the hovering text is attached.
        void SetPosition(const Vector3df &position);

        /// Sets the font used for the hovering text.
        /// @param font Font.
        void SetFont(const QFont &font);

        /// Sets the color of the chat bubble text.
        /// @param color Color.
        void SetTextColor(const QColor &color);

        Ogre::Billboard* const GetButtonsBillboard(){return buttonsbillboard_;}
        Ogre::BillboardSet* const GetButtonsBillboardSet(){return buttonsbillboardSet_;}
<<<<<<< HEAD
        QSizeF GetButtonsBillboardScreenSpaceSize(){ return bb_buttons_size_view; }
=======
>>>>>>> 5aa6bd5698a8734b6fe9ce7cb684b848eb04a506


    public slots:
        void InitializeBillboards();

        void SetDisabled(bool val){disabled_ = val;}

        bool IsDisabled(){return disabled_;}

        void SetButtonsDisabled(bool val);

        bool IsButtonsDisabled(){return buttons_disabled_;}

        void ShowButtons(bool val);
        /// Shows the hovering text.
        void Show();

        /// Shows the hovering text with animation.
        void AnimatedShow();
        void EntityClicked(int msec_to_show = 5000);

        void WidgetClicked(Real x, Real y);

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

        void AddButton(QPushButton &button);

        void SetCameraDistance(Real dist);
        Real GetCameraDistance(){return cam_distance_;}

        void AdjustWidgetinfo();

        void ScaleWidget(Ogre::BillboardSet& bset, Ogre::Billboard& b, QSizeF& size,bool next_to_nametag=false);

    private slots:
        /// Updates the animation
        /// @param step
        void UpdateAnimationStep(int step);

        /// Finishes the animation.
        void AnimationFinished();

        /// Redraws the hovering text with the current text, font and color.
        void Redraw();

    private:

        /// Returns pixmap with widget rendered to it
        QPixmap GetPixmap(QWidget& w, QRect dimensions);

        /// Renderer pointer.
        boost::weak_ptr<OgreRenderer::Renderer> renderer_;

        /// Ogre billboard set.
        Ogre::BillboardSet *namebillboardSet_;

        Ogre::BillboardSet *buttonsbillboardSet_;

        /// Ogre billboard.
        Ogre::Billboard *namebillboard_;

        Ogre::Billboard *buttonsbillboard_;

        /// Name of the material used for the billboard set.
        std::string namematerialName_;

        std::string buttonsmaterialName_;


        // Visibility animation timeline.
        QTimeLine *visibility_animation_timeline_;

        // Timed visibility timer
        QTimer *visibility_timer_;

        bool disabled_;

        bool buttons_disabled_;

        bool buttons_visible_;

        HoveringNameController* namewidget_;

        HoveringButtonsController *buttonswidget_;

        QSizeF bb_name_size_view;
        QSizeF bb_buttons_size_view;

        Real cam_distance_;

        Real bb_rel_posy;
    };
}
#endif
