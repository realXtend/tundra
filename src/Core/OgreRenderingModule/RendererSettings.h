// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "InputFwd.h"

#include <QWidget>

class Framework;

namespace OgreRenderer
{
    /// Widget for renderer settings
    class RendererSettingsWindow : public QWidget
    {
        Q_OBJECT

    public:
        explicit RendererSettingsWindow(Framework* fw, QWidget *parent = 0);
        ~RendererSettingsWindow();

    public slots:
        void ViewDistanceChanged(double value);
        void ShadowQualityChanged(int quality);
        void TextureQualityChanged(int quality);

        /// toggles fullscreen mode
        void SetFullScreenMode(bool value);
        void KeyPressed(KeyEvent* e);

    private:
        InputContextPtr input_context_;
        Framework* framework_;
        QWidget *settings_widget_;
    };
}
