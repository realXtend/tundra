/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiProxyWidget.h
 *  @brief  Represents Naali UI widget created by embedding QWidget to the same
 *          canvas as the in-world 3D scene.
 */

#ifndef incl_Interfaces_UiProxyWidget_h
#define incl_Interfaces_UiProxyWidget_h

#include <QGraphicsProxyWidget>

class QTimeLine;
class QParallelAnimationGroup;
class QPropertyAnimation;

/// Represents Naali UI widget created by embedding QWidget to the same canvas as the in-world 3D scene.
class UiProxyWidget : public QGraphicsProxyWidget
{
    Q_OBJECT

public:
    /** Constructor.
        @param  widget The widget which will be embedded to the proxy widget.
        @param  flags Window flags. Qt::Dialog is used as default.
        It creates movable proxy widget which has title bar and frames.
        If you want add widget without title bar and frames, use Qt::Widget.
        For further information, see @see http://doc.qt.nokia.com/4.6/qt.html#WindowType-enum
    */
    UiProxyWidget(QWidget *widget, Qt::WindowFlags flags = Qt::Dialog);

    /// Destructor.
    ~UiProxyWidget();

    /** Sets opacity for unfocused state.
        @param opacity Opacity value between 0 and 100.
     */
    void SetUnfocusedOpacity(int opacity);

    /** Sets show animation speed
     *  @param speed Speed in milliseconds.
     */
    void SetShowAnimationSpeed(int speed);

    /// Brings to front in the scene, sets focus and shows this proxy widget.
    /// @todo Seems that isn't working properly.
    void BringToFront();

    /** OObject override.
        Used for changing the proxy widget's window title when application language changes.
        @note Retrieves the name from the embedded QWidget so the QWidget's window title must
        be set properly.
    */
    bool eventFilter(QObject *obj, QEvent *e);

public slots:
    /// Toggles visibility with show/hide
    void ToggleVisibility();

    /// Performs animated hide, if animations are enabled.
    void AnimatedHide();

signals:
    /// Emitted when the proxy is closed/hidden.
    void Closed();

    /** Emitted when visibility of the proxy changes.
        @param visible Visiblity.
    */
    void Visible(bool visible);

    /** Emitted when visibility of the proxy changes.
        @param visible Visiblity.
    */
    void BringProxyToFrontRequest(QGraphicsProxyWidget *widget);

    /** Emitted when proxy widget has moved.
        @param widget Widget.
        @param pos New position.
    */
    void ProxyMoved(QGraphicsProxyWidget *widget, const QPointF &pos);

    /** Emitted when proxy widget has been ungrabbed.
        @param widget
        @param pos New Position.
    */
    void ProxyUngrabbed(QGraphicsProxyWidget *widget, const QPointF &pos);

protected:
    /** QGraphicsProxyWidget override.
        Emits Visible(true) signal.
    */
    void showEvent(QShowEvent *e);

    /** QGraphicsProxyWidget override.
        Emits Visible(false) signal.
    */
    void hideEvent(QHideEvent *e);

    /** QGraphicsProxyWidget override.
        Emits Closed() signal.
    */
    void closeEvent(QCloseEvent *e);

    /** QGraphicsProxyWidget override.
        Alters opacity of the proxy widget.
    */
    void focusInEvent(QFocusEvent *e);

    /** QGraphicsProxyWidget override.
        Alters opacity of the proxy widget.
    */
    void focusOutEvent(QFocusEvent *e);

    /// QGraphicsProxyWidget override.
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    /** QGraphicsProxyWidget override.
        Emits Moved() signal.
    */
    void moveEvent(QGraphicsSceneMoveEvent * e);

    /** QGraphicsProxyWidget override.
        Emits ProxyUngrabbed() signal.
    */
    void ungrabMouseEvent(QEvent *e);

private slots:
    /// Hides proxy widget after animated hide is complete.
    void FinishHide();

private:
    /// Group of parallel animations.
    QParallelAnimationGroup *animations_;

    /// Fade animation.
    QPropertyAnimation *fade_animation_;

    /// Opacity in unfocused state, 0.0-1.0.
    qreal unfocus_opacity_;

    /// Is the animated show enabled.
    bool show_animation_enabled_;
};

#endif
