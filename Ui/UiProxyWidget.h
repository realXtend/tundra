/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiProxyWidget.h
 *  @brief  Represents a UI widget created by embedding QWidget to the same
 *          canvas as the in-world 3D scene.
 */

#pragma once

#include <QGraphicsProxyWidget>

/// Represents a UI widget created by embedding QWidget to the same canvas as the in-world 3D scene.
class UiProxyWidget : public QGraphicsProxyWidget
{
    Q_OBJECT
    Q_PROPERTY(QGraphicsEffect* graphicsEffect READ GraphicsEffect WRITE SetGraphicsEffect);

public:
    /// Constructor.
    /** @param  widget The widget which will be embedded to the proxy widget.
        @param  flags Window flags. Qt::Dialog is used as default.
        It creates movable proxy widget which has title bar and frames.
        If you want add widget without title bar and frames, use Qt::Widget.
        For further information, see @see http://doc.qt.nokia.com/4.6/qt.html#WindowType-enum
    */
    UiProxyWidget(QWidget *widget, Qt::WindowFlags flags = Qt::Dialog);

    /// Destructor.
    ~UiProxyWidget();

    /// Brings to front in the scene, sets focus and shows this proxy widget.
    /// @todo Seems that isn't working properly.
    void BringToFront();

    /// OObject override.
    /** Used for changing the proxy widget's window title when application language changes.
        @note Retrieves the name from the embedded QWidget so the QWidget's window title must
        be set properly.
    */
    bool eventFilter(QObject *obj, QEvent *e);

public slots:
    /// Add graphics effect to proxy widget.
    /** @param effect new effect that we are applying to proxy.
    */
    void SetGraphicsEffect(QGraphicsEffect *effect)
    {
        setGraphicsEffect(effect);
    }

    /// Get graphics effect from proxy widget.
    /** @return graphics effect in use (return null if no graphics effect is used).
    */
    QGraphicsEffect *GraphicsEffect() const
    {
        return graphicsEffect();
    }

signals:
    /// Emitted when the proxy is closed/hidden.
    void Closed();

    /// Emitted when visibility of the proxy changes.
    /** @param visible Visibility.
    */
    void Visible(bool visible);

    /// Emitted when visibility of the proxy changes.
    /** @param visible Visibility.
    */
    void BringProxyToFrontRequest(QGraphicsProxyWidget *widget);

protected:
    /// QGraphicsProxyWidget override.
    /** Emits Visible(true) signal.
    */
    void showEvent(QShowEvent *e);

    /// QGraphicsProxyWidget override.
    /** Emits Visible(false) signal.
    */
    void hideEvent(QHideEvent *e);

    /// QGraphicsProxyWidget override.
    /** Emits Closed() signal.
    */
    void closeEvent(QCloseEvent *e);
};

