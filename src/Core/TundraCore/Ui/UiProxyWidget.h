/**
    For conditions of distribution and use, see copyright notice in LICENSE
 
    @file   UiProxyWidget.h
    @brief  Represents a UI widget created by embedding QWidget to the same canvas as the in-world 3D scene. */

#pragma once

#include "TundraCoreApi.h"

#include <QGraphicsProxyWidget>

/// Represents a UI widget created by embedding QWidget to the same canvas as the in-world 3D scene.
class TUNDRACORE_API UiProxyWidget : public QGraphicsProxyWidget
{
    Q_OBJECT
    Q_PROPERTY(QGraphicsEffect* graphicsEffect READ GraphicsEffect WRITE SetGraphicsEffect);

public:
    /// Constructor.
    /** @param  widget The widget which will be embedded to the proxy widget.
        @param  flags Window flags. Qt::Dialog is used as default.
        It creates movable proxy widget which has title bar and frames.
        If you want add widget without title bar and frames, use Qt::Widget.
        For further information, see @see http://doc.qt.nokia.com/4.6/qt.html#WindowType-enum */
    UiProxyWidget(QWidget *widget, Qt::WindowFlags flags = Qt::Dialog);

    virtual ~UiProxyWidget();

    /// Brings to front in the scene, sets focus and shows this proxy widget.
    /// @todo Seems that isn't working properly.
    void BringToFront();

    /// OObject override.
    /** Used for changing the proxy widget's window title when application language changes.
        @note Retrieves the name from the embedded QWidget so the QWidget's window title must be set properly. */
    virtual bool eventFilter(QObject *obj, QEvent *e);

public slots:
    /// Add graphics effect to proxy widget.
    /** @param effect new effect that we are applying to proxy.
        @note This function exists for QtScript compatibility.
        @todo Doens't need to be a slot, exposed as property. */
    void SetGraphicsEffect(QGraphicsEffect *effect) { setGraphicsEffect(effect); }

    /// Get graphics effect from proxy widget.
    /** @return graphics effect in use (return null if no graphics effect is used).
        @note This function exists for QtScript compatibility.
        @todo Doens't need to be a slot, exposed as property. */
    QGraphicsEffect *GraphicsEffect() const { return graphicsEffect(); }

signals:
    /// Emitted when the proxy is closed/hidden.
    void Closed();

    /// Emitted when visibility of the proxy changes.
    /** @param visible Visibility. */
    void Visible(bool visible);

    /// Emitted when visibility of the proxy changes.
    /** @param visible Visibility. */
    void BringProxyToFrontRequest(QGraphicsProxyWidget *widget);

protected:
    /// QGraphicsProxyWidget override.
    /** Emits Visible(true) signal. */
    virtual void showEvent(QShowEvent *e);

    /// QGraphicsProxyWidget override.
    /** Emits Visible(false) signal. */
    virtual void hideEvent(QHideEvent *e);

    /// QGraphicsProxyWidget override.
    /** Emits Closed() signal. */
    virtual void closeEvent(QCloseEvent *e);
};
