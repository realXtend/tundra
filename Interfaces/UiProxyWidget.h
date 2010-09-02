/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiProxyWidget.h
 *  @brief  
 */

#ifndef incl_Interfaces_UiProxyWidget_h
#define incl_Interfaces_UiProxyWidget_h

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
     *  @param  widget The widget which will be embedded to the proxy widget.
     *  @param  flags Window flags. Qt::Dialog is used as default.
     *          It creates movable proxy widget which has title bar and frames.
     *          If you want add widget without title bar and frames, use Qt::Widget.
     *          For further information, see @see http://doc.qt.nokia.com/4.6/qt.html#WindowType-enum
     */
    UiProxyWidget(QWidget *widget, Qt::WindowFlags flags = Qt::Dialog);

    /// Destructor.
    ~UiProxyWidget();

    /** Sets opacity for unfocused state.
     *  @param opacity Opacity value between 0 and 100.
     */
    void SetUnfocusedOpacity(int opacity);

    /** Sets show animation speed
     *  @param speed Speed in milliseconds.
     */
    void SetShowAnimationSpeed(int speed);

    /// Brings to front in the scene, sets focus and shows this proxy widget.
    /// @todo Seems that isn't working properly.
    void BringToFront();

public slots:
    /// Toggles visibility with show/hide
    void ToggleVisibility();

    /// Performs animated hide, if animations are enabled.
    void AnimatedHide();

signals:
    /// Emitted when the proxy is closed/hidden.
    void Closed();

    /** Emitted when visibility of the proxy changes.
     *  @param visible Visiblity.
     */
    void Visible(bool visible);

    /** Emitted when visibility of the proxy changes.
     *  @param visible Visiblity.
     */
    void BringProxyToFrontRequest(QGraphicsProxyWidget *widget);

    /** Emitted when proxy widget has moved.
     *  @param widget Widget.
     *  @param pos New position.
     */
    void ProxyMoved(QGraphicsProxyWidget *widget, const QPointF &pos);

    /** Emitted when proxy widget has been ungrabbed.
     *  @param widget
     *  @param pos New Position.
     */
    void ProxyUngrabbed(QGraphicsProxyWidget *widget, const QPointF &pos);

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
    /// Finishes (hides) proxy widget after animated hide.
    void FinishHide();

private:
    ///
    QParallelAnimationGroup *animations_;

    ///
    QPropertyAnimation *fade_animation_;

    /// Opacity in unfocused state, 0.0-1.0.
    qreal unfocus_opacity_;

    /// Is the animated show enabled.
    bool show_animation_enabled_;
};

#endif
