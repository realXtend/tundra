/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiProxyWidget.h
 *  @brief  
 */

#ifndef incl_Interfaces_UiProxyWidget_h
#define incl_Interfaces_UiProxyWidget_h

#include "UiWidgetProperties.h"

#include <QGraphicsProxyWidget>

class QTimeLine;
class QParallelAnimationGroup;
class QPropertyAnimation;

/**
 *
 */
class UiProxyWidget : public QGraphicsProxyWidget
{
    Q_OBJECT

public:
    /** Constructor.
     *  @param widget The widget which will be embedded to the proxy widget.
     */
    UiProxyWidget(QWidget *widget, const UiServices::UiWidgetProperties &properties);

    /// Destructor.
    ~UiProxyWidget();

    /// Get this proxys widget properties
    /// @return UiWidgetProperties.
    UiServices::UiWidgetProperties GetWidgetProperties() const { return properties_; }

    /// Set new opacity
    void SetUnfocusedOpacity(int new_opacity);

    /// Set new show animation speed
    void SetShowAnimationSpeed(int new_speed);

    /// Brings to front in the scene, sets focus and shows this proxy widget.
    /// @todo Seems that isn't working properly.
    void BringToFront();

public slots:
    /// Performs animated hide, if animations are enabled.
    void AnimatedHide();

protected:
    /// QGraphicsProxyWidget override.
    void showEvent(QShowEvent *show_event);

    /// QGraphicsProxyWidget override.
    void hideEvent(QHideEvent *hide_event);

    /// QGraphicsProxyWidget override.
    void closeEvent(QCloseEvent *close_event);

    /// QGraphicsProxyWidget override.
    void focusInEvent(QFocusEvent *focus_event);

    /// QGraphicsProxyWidget override.
    void focusOutEvent(QFocusEvent *focus_event);

    /// QGraphicsProxyWidget override.
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    /// QGraphicsProxyWidget override.
    void moveEvent(QGraphicsSceneMoveEvent * event);

    /// QGraphicsProxyWidget override.
    void ungrabMouseEvent(QEvent *event);

private slots:
    void FinishHide();

private:
    UiServices::UiWidgetProperties properties_;
    QParallelAnimationGroup *animations_;
    QPropertyAnimation *fade_animation_;
    qreal unfocus_opacity_;
    bool show_animation_enabled_;

signals:
    /// Emitted when the proxy is closed/hidden.
    void Closed();

    /** Emitted when visibility of the proxy changes.
     *  @param visible Visiblity.
     */
    void Visible(bool visible);

    void BringProxyToFrontRequest(QGraphicsProxyWidget*);
    void ProxyMoved(QGraphicsProxyWidget*, const QPointF &);
    void ProxyUngrabbed(QGraphicsProxyWidget*, const QPointF &);
};

#endif
