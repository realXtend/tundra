/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiProxyWidget.cpp
 *  @brief  Represents Naali UI widget created by embedding QWidget to the same
 *          canvas as the in-world 3D scene.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "UiWidget.h"
#include "MemoryLeakCheck.h"

UiWidget::UiWidget(QWidget *widget, Qt::WindowFlags flags):
QWidget(widget, flags)
{
}

UiWidget::~UiWidget(void)
{
}
