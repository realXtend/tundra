/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_HoveringWidget.h
 *  @brief  EC_HoveringWidget shows a hovering Widget attached to an entity.
 *  @note   The entity must have EC_OgrePlaceable component available in advance.
*/
#ifndef incl_HoveringWidgetController_h
#define incl_HoveringWidgetController_h

#include "ui_HoveringWidget.h"
#include <QWidget>
#include <CoreTypes.h>
#include <QRect>

class QPushButton;

namespace OgreRenderer
{
    class HoveringWidgetController: public QWidget, private Ui::HoveringWidget
    {
        Q_OBJECT

    public:
        HoveringWidgetController();
        virtual ~HoveringWidgetController();

        void ShowButtons();
        void HideButtons();
        void AddButton(QPushButton *button);
        void SetText(const QString& text);
        void ForwardMouseClickEvent(Real x, Real y);

    public slots:
        void ButtonPressed();

    private:
        Real text_padding_;


    };
}

#endif