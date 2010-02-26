// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherMenu_h
#define incl_UiModule_EtherMenu_h

#include <QScopedPointer>
#include <QVector>
#include <QPointF>
#include <QGraphicsWidget>
#include <QParallelAnimationGroup>
#include <QTimer>

#include "UiModuleFwd.h"
//#include "InfoCard.h"

namespace Ether
{
    namespace View
    {
        //! Class for animated menu.
        //! note that this class is NOT responsible of deleting the QGraphicsobjects it animates
        class EtherMenu : public QObject
        {
            Q_OBJECT
            Q_PROPERTY(qreal scale_factor READ GetScaleFactor WRITE SetScaleFactor)

        public:
            //! Type of EtherMenu is either ELLIPSE_OPENS_UP, ELLIPSE_OPENS_DOWN, LINEAR_TOP, LINEAR_MID, LINEAR_BOTTOM
            EtherMenu();
            virtual ~EtherMenu();

            //! Initializes the menu with given bounds, items that menu has, maximum size of the biggest item,
            //! scalefactor (how much scale down when item is more far away from being first) and number of objects visible in menu
            virtual void Initialize(
                const QRectF& bounds,
                const QVector<InfoCard *>& items,
                const QRectF& max_size,
                qreal scalef,
                int visible_objects=5,
                qreal opacity_factor=0,
                qreal max_gap=0);

            //! Set new focus item to center
            virtual void SetFocusToCard(InfoCard *card);

            //! Changes bounding rect to new bounds
            virtual void RectChanged(const QRectF& bounds);

            virtual InfoCard *GetHighlighted();

            //! Returns the position of current index 0 (selected) item
            virtual QPointF GetHightlightScenePos() const { if (positions_.count() > 0) return positions_.at(0); else return QPointF(); }

            virtual void moveRight();

            virtual void moveLeft();

            virtual void SetScaleFactor(qreal sfactor){ scale_factor_ = sfactor; }

            virtual qreal GetScaleFactor() const { return scale_factor_;}

            virtual QVector<InfoCard *> GetObjects() const { return objects_; }

        signals:
            void ItemHighlighted(View::InfoCard*);

        protected:
            virtual void CalculateGap();
            virtual void MoveObjectToPosition(int index, const QPointF& point);
            virtual void ScaleObjectBasedOnPriority(int index,int priority);
            virtual void ZOrderObjectBasedOnPriority(int index, int priority, bool animated);
            virtual void ChangeObjectOpacityBasedOnPriority(int index,int priority);
            virtual void AssignObjectsToStates();

            //! Differs from AssignObjectsToStates() function so that this
            //! version does not animate some stages (like z buffer)
            virtual void InitializeObjectsToStates();
            virtual void InitializeObjectValues();

            InfoCard* highlighted_;

            virtual void ConstructPriorityList();
            virtual void CalculateLimits();

            virtual void InitializeAnimations();

            virtual void HideCardAtIndex(int index);
            virtual void ShowCardAtIndex(int index);

            //! Calculate positions for objects
            virtual void CalculatePositions(QVector<QPointF> &positions)=0;

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

            //! Opacity factor is used to scale cards transparency depending on the priority. 0 means that all the cards are completely opaque.
            //! 1 means that only highlighted card is opaque, rest of the cards are fully transparent.
            //! values between 1-0 increase transparenct when priority decreases.
            qreal opacity_factor_;

            qreal scale_factor_;
            qreal current_scale_factor_;

            qreal visible_;
            qreal invisible_;

            qreal current_gap_;
            qreal max_gap_;
        };
    }
}

#endif // ETHERMENU_H
