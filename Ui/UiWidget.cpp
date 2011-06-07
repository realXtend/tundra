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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVariant>

UiWidget::UiWidget(QWidget *widget, QWidget *parent, Qt::WindowFlags flags, const QStringList &params):
QWidget(parent, flags),
widget_(widget)
{
    setWindowTitle(widget_->windowTitle());

    QHBoxLayout* h_box = new QHBoxLayout();
    h_box->addWidget(widget_);
    
    QVBoxLayout* v_box = new QVBoxLayout();
    v_box->addLayout(h_box);
    
    setLayout(v_box);

    int i = 1;
    while (i < params.size())
    {
        setProperty(params.at(i-1).toStdString().c_str(), QVariant::fromValue(params.at(i)));
        i += 2;
    }
}

UiWidget::~UiWidget(void)
{
}

void UiWidget::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    emit visibilityChanged(visible);
}

void UiWidget::toogleVisibility()
{
    setVisible(!isVisible());
}