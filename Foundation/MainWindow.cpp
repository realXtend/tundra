/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   MainWindow.cpp
 *  @brief  Main window, which overrides the closeEvent
 */

#include "StableHeaders.h"

#include "MainWindow.h"
#include "Framework.h"

#include <QCloseEvent>

namespace Foundation
{

void MainWindow::closeEvent(QCloseEvent* e)
{
    framework_->Exit();
    e->ignore();
}

}
