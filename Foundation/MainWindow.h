/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   MainWindow.h
 *  @brief  Main window, which overrides the closeEvent
 */

#include <QWidget>

namespace Foundation
{
    class Framework;

    //! Main window, which overrides the closeEvent
    class MainWindow : public QWidget
    {
    public:
        /// Constructor.
        /// @param framework Framework pointer.
        MainWindow(Foundation::Framework* framework) : framework_(framework) {}

    protected:
        /// QWidget override.
        void closeEvent(QCloseEvent* e);

    private:
        /// Framework pointer.
        Framework* framework_;
    };
}
