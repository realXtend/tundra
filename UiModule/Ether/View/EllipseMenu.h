// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EllipseMenu_h
#define incl_UiModule_EllipseMenu_h

#include <QSharedPointer>
#include <QVector>
#include <QPointF>
#include <QGraphicsWidget>
#include <QParallelAnimationGroup>
#include <QTimer>

#include "InfoCard.h"

namespace Ether
{
    namespace View
    {
        //! Class for animated ellipse menu.
        //! note that this class is NOT responsible of deleting the QGraphicsobjects it animates
        class EllipseMenu : public QObject
        {

        Q_OBJECT
        Q_PROPERTY(qreal scale_factor READ GetScaleFactor WRITE SetScaleFactor)

        public:
            //! Ellipse menu opens either down or up
            enum TYPE { OPENS_UP, OPENS_DOWN };

            //! Type of EllipseMenu is either OPENS_UP or OPENS_DOWN
            EllipseMenu(TYPE type);
            virtual ~EllipseMenu();

            //! Initializes the menu with given bounds, items that menu has, maximum size of the biggest item, scalefactor (how much scale down when item is more far away from being first) and number of objects visible in menu
            void Initialize(const QRectF& bounds, const QVector<InfoCard *>& items, const QRectF& max_size, qreal scalef, int visible_objects=5, qreal opacity_factor=0);

            //! Set new focus item to center
            void SetFocusToCard(InfoCard *card);

            //! Changes bounding rect to new bounds
            void RectChanged(const QRectF& bounds);

            InfoCard *GetHighlighted();

            //! Returns the position of current index 0 (selected) item
            QPointF GetHightlightScenePos() { if (positions_.count() > 0) return positions_.at(0); else return QPointF(); }

			void moveRight();

            void moveLeft();

            void SetScaleFactor(qreal sfactor){ scale_factor_ = sfactor; }

            qreal GetScaleFactor(){return scale_factor_;}

            QVector<InfoCard *> GetObjects() { return objects_; }

        signals:
            void ItemHighlighted(View::InfoCard*);

        private:
            void MoveObjectToPosition(int index, const QPointF& point);
            void ScaleObjectBasedOnPriority(int index,int priority);
            void ZOrderObjectBasedOnPriority(int index, int priority, bool animated);
            void ChangeObjectOpacityBasedOnPriority(int index,int priority);
            void AssignObjectsToStates();

            //! Differs from AssignObjectsToStates() function so that this
            //! version does not animate some stages (like z buffer)
            void InitializeObjectsToStates();
            void InitializeObjectValues();

            InfoCard* highlighted_;

            void ConstructPriorityList();
            void CalculateLimits();

            void InitializeAnimations();

            void HideCardAtIndex(int index);
            void ShowCardAtIndex(int index);

            //! Calculate positions for objects
            void CalculatePositions(QVector<QPointF> &positions);

            //! Relies that order of objects does not change(this is not the order of the menu)
            QVector<InfoCard *> objects_;

            //! Calculated positions
            QVector<QPointF> positions_;

            //! List to determine the place in the menu
            QVector<int> priority_list_;

            //! Relies that order of animations does not change
            QScopedPointer<QParallelAnimationGroup> animations_;

            //! Bounding rect for animation
            QRectF boundaries_;

            //! Max card size in scene
            QRectF card_max_size_;

            QRectF current_card_max_size_;

            //! Default hide position
            QPointF hide_point_;

            //! Number of objects visible in menu
            int max_visible_objects_;

            //! Determines the left limit. Meaning that the indexes left of this one on priority list are not visible
            int left_visible_limit_index_;

            //! Determines the right limit. Meaning that the indexes right of this one on priority list are not visible
            int right_visible_limit_index_;

            //! Opacity factor is used to scale cards transparency depending on the priority. 0 means that all the cards are completely opaque. 1 means that only highlighted card is opaque, rest of the cards are fully transparent.
            //! values between 1-0 increase transparenct when priority decreases.
            qreal opacity_factor_;

            qreal scale_factor_;
            qreal current_scale_factor_;

            qreal visible_;
            qreal invisible_;

            TYPE type_;
        };
    }
}

#endif // ELLIPSEMENU_H
