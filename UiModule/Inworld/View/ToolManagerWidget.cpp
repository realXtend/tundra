/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   VoiceController.cpp
 *  @brief  In-world voice controller user interface
 *          
 */
 
#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "IToolWidget.h"
#include "ToolManagerWidget.h"

#include "DebugOperatorNew.h"

namespace CoreUi
{
    ToolContainerWidget::ToolContainerWidget(QString name, IToolWidget* tool_widget) : 
        name_(name),
        tool_widget_(tool_widget)
    {
        setupUi(this);
        this->nameLabel->setText(name_);
        this->toolWidget->layout()->addWidget(tool_widget);
    }

    ToolContainerWidget::~ToolContainerWidget()
    {

    }

    QString ToolContainerWidget::GetName()
    {
        return name_;
    }

    IToolWidget* ToolContainerWidget::GetToolWidget()
    {
        return tool_widget_;
    }




    ToolManagerWidget::ToolManagerWidget() 
    {
        setupUi(this);
    }

    ToolManagerWidget::~ToolManagerWidget()
    {
        RemoveAllToolWidgets();
    }

    void ToolManagerWidget::AddToolWidget(QString name, IToolWidget* widget)
    {
        ToolContainerWidget* w = new ToolContainerWidget(name, widget);
        tool_widgets_.append(w);
        this->contentWidget->layout()->addWidget(w);
        emit ToolWidgetAdded(w->GetToolWidget());
    }

    void ToolManagerWidget::RemoveToolWidget(IToolWidget* widget)
    {
        for (QList<ToolContainerWidget*>::iterator i = tool_widgets_.begin(); i != tool_widgets_.end(); ++i)
        {
            ToolContainerWidget* tcw = *i;
            if (tcw->GetToolWidget() == widget)
            {
                tool_widgets_.erase(i);
                this->contentWidget->layout()->removeWidget(tcw);
                emit ToolRemoved( tcw->GetToolWidget() );
                SAFE_DELETE(tcw);
                return;
            }
        }
    }

    void ToolManagerWidget::RemoveAllToolWidgets()
    {
        for (QList<ToolContainerWidget*>::iterator i = tool_widgets_.begin(); i != tool_widgets_.end(); ++i)
        {
            ToolContainerWidget* tcw = *i;
            this->contentWidget->layout()->removeWidget(tcw);
            emit ToolRemoved( tcw->GetToolWidget() );
            SAFE_DELETE(tcw);
        }
        tool_widgets_.clear();
    }

} // namespace CoreUi
