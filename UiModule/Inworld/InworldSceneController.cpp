// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "InworldSceneController.h"
#include "UiAPI.h"
#include "Common/AnchorLayoutManager.h"
#include "UiProxyWidget.h"
#include "UiMainWindow.h"

#include "MemoryLeakCheck.h"
#include "TundraLogicModule.h"
#include "Client.h"
#include "Renderer.h"

#include <QGraphicsView>

#define DOCK_WIDTH          (300)
#define DIST_FROM_BOTTOM    (200)
#define DIST_FROM_TOP       (50)

namespace UiServices
{
    InworldSceneController::InworldSceneController(Foundation::Framework *framework, QGraphicsView *ui_view) :
          framework_(framework),
          ui_view_(ui_view)
    {
        assert(ui_view_);

        // Store scene pointer
        inworld_scene_ = ui_view_->scene();
        assert(inworld_scene_);

        // Init layout manager with scene
        layout_manager_ = new CoreUi::AnchorLayoutManager(this, inworld_scene_);
        connect(framework->Ui()->MainWindow(), SIGNAL(WindowResizeEvent(int, int)), layout_manager_, SLOT(AdjustLayoutContainer(int, int)));
		boost::shared_ptr<TundraLogic::Client> client=framework_->GetModule<TundraLogic::TundraLogicModule>()->GetClient();
		connect(client.get(), SIGNAL(Disconnected()), SLOT(worldDisconnected()));

        // Apply new positions to active widgets when the inworld_scene_ is resized
        connect(inworld_scene_, SIGNAL(sceneRectChanged(const QRectF)), this, SLOT(ApplyNewProxyPosition(const QRectF)));
	}

    InworldSceneController::~InworldSceneController()
    {
    }


    bool InworldSceneController::AddAnchoredWidgetToScene(QGraphicsProxyWidget *qgrap, Qt::Corner corner, Qt::Orientation orientation, int priority, bool persistence)
    {
        //Add to non_priority list if not persistent between worlds
		if (!persistence)
			non_persistent_widgets.append(qgrap);

		//Case
		switch(corner)
		{
		case Qt::BottomLeftCorner:
			{
				if (orientation == Qt::Horizontal)
				{
					if (bottomleft_horiz_.empty())
						bottomleft_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < bottomleft_horiz_.size()) {
							if (bottomleft_horiz_.at(i).second < priority)
								break;
							i++;
						}
						bottomleft_horiz_.insert(i, internal_element(qgrap, priority));
					}
				}
				else
				{
					if (bottomleft_horiz_.empty()) //HORIZ!
						bottomleft_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < bottomleft_vert_.size()) {
							if (bottomleft_vert_.at(i).second < priority)
								break;
							i++;
						}
						bottomleft_vert_.insert(i, internal_element(qgrap, priority));
					}
				}
				break;
			}
		case Qt::BottomRightCorner:
			{
				if (orientation == Qt::Horizontal)
				{
					if (bottomright_horiz_.empty())
						bottomright_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < bottomright_horiz_.size()) {
							if (bottomright_horiz_.at(i).second < priority)
								break;
							i++;
						}
						bottomright_horiz_.insert(i, internal_element(qgrap, priority));
					}
				}
				else
				{
					if (bottomright_horiz_.empty()) //HORIZ!
						bottomright_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < bottomright_vert_.size()) {
							if (bottomright_vert_.at(i).second < priority)
								break;
							i++;
						}
						bottomright_vert_.insert(i, internal_element(qgrap, priority));
					}
				}
				break;
			}
		case Qt::TopLeftCorner:
			{
				if (orientation == Qt::Horizontal)
				{
					if (topleft_horiz_.empty())
						topleft_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < topleft_horiz_.size()) {
							if (topleft_horiz_.at(i).second < priority)
								break;
							i++;
						}
						topleft_horiz_.insert(i, internal_element(qgrap, priority));
					}
				}
				else
				{
					if (topleft_horiz_.empty()) //HORIZ!
						topleft_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < topleft_vert_.size()) {
							if (topleft_vert_.at(i).second < priority)
								break;
							i++;
						}
						topleft_vert_.insert(i, internal_element(qgrap, priority));
					}
				}
				break;
			}
		default: //case Qt::Corner::TopRightCorner:
			{
				if (orientation == Qt::Horizontal)
				{
					if (topright_horiz_.empty())
						topright_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < topright_horiz_.size()) {
							if (topright_horiz_.at(i).second < priority)
								break;
							i++;
						}
						topright_horiz_.insert(i, internal_element(qgrap, priority));
					}
				}
				else
				{
					if (topright_horiz_.empty()) //HORIZ!
						topright_horiz_.push_back(internal_element(qgrap, priority));
					else
					{
						int i = 0;
						while (i < topright_vert_.size()) {
							if (topright_vert_.at(i).second < priority)
								break;
							i++;
						}
						topright_vert_.insert(i, internal_element(qgrap, priority));
					}
				}
			}
		}
		AddInternalWidgets();
		return true;

    }

	bool InworldSceneController::AddAnchoredWidgetToScene(QWidget *widget, Qt::Corner corner, Qt::Orientation orientation, int priority, bool persistence) 
	{
		//Create the QGraphicsProxyWidget
		QGraphicsProxyWidget *qgrap = new QGraphicsProxyWidget(0, Qt::Widget);
		qgrap->setWidget(widget);
        return AddAnchoredWidgetToScene(qgrap, corner, orientation, priority, persistence);
	}

    bool InworldSceneController::RemoveAnchoredWidgetFromScene(QWidget *widget) 
    { 
        QGraphicsProxyWidget *qgrap = widget->graphicsProxyWidget();
        if(qgrap)
            return RemoveAnchoredWidgetFromScene(qgrap);
        else
            return false;
    }

    bool InworldSceneController::RemoveAnchoredWidgetFromScene(QGraphicsProxyWidget *qgrap) 
	{
        //Clean lists
        int i = 0;
		while(i<bottomleft_horiz_.size())
        {
            if (bottomleft_horiz_.at(i).first == qgrap) {
				bottomleft_horiz_.removeAt(i);
                AddInternalWidgets();
                return true;
            }
			i++;
        }
		//bottomleft_vert_
		i = 0;
        while(i<bottomleft_vert_.size())
        {
            if (bottomleft_vert_.at(i).first == qgrap) {
				bottomleft_vert_.removeAt(i);
                AddInternalWidgets();
                return true;
            }
			i++;
        }
		//bottomright_horiz_
		i = 0;
        while(i<bottomright_horiz_.size())
        {
            if (bottomright_horiz_.at(i).first == qgrap) {
				bottomright_horiz_.removeAt(i);
                AddInternalWidgets();
                return true;
            }
			i++;
        }		
		//bottomright_vert_
        i = 0;
		while(i<bottomright_vert_.size())
        {
            if (bottomright_vert_.at(i).first == qgrap) {
				bottomright_vert_.removeAt(i);
                AddInternalWidgets();
                return true;
            }
			i++;
        }
		//topleft_horiz_
		i = 0;
		while(i<topleft_horiz_.size())
        {
            if (topleft_horiz_.at(i).first == qgrap) {
				topleft_horiz_.removeAt(i);
                AddInternalWidgets();
                return true;
            }
			i++;
        }
		//topleft_vert_
		i = 0;
		while(i<topleft_vert_.size())
        {
            if (topleft_vert_.at(i).first == qgrap) {
				topleft_vert_.removeAt(i);
                AddInternalWidgets();
                return true;
            }
			i++;
        }
		//topright_horiz_
		i = 0;
		while(i<topright_horiz_.size())
        {
            if (topright_horiz_.at(i).first == qgrap) {
				topright_horiz_.removeAt(i);
                AddInternalWidgets();
                return true;
            }
			i++;
        }
		//topright_vert_
		i = 0;
		while(i<topright_vert_.size())
        {
            if (topright_vert_.at(i).first == qgrap) {
				topright_vert_.removeAt(i);
                AddInternalWidgets();
                return true;
            }
			i++;
        }		
        return false;
	}

	void InworldSceneController::worldDisconnected()
	{
		//Clean lists
		int i = 0;
		while(i<bottomleft_horiz_.size())
			if (non_persistent_widgets.contains(bottomleft_horiz_.at(i).first))
				bottomleft_horiz_.removeAt(i);
			else
				i++;
		//bottomleft_vert_
		i = 0;
		while(i<bottomleft_vert_.size())
			if (non_persistent_widgets.contains(bottomleft_vert_.at(i).first))
				bottomleft_vert_.removeAt(i);
			else
				i++;
		//bottomright_horiz_
		i = 0;
		while(i<bottomright_horiz_.size())
			if (non_persistent_widgets.contains(bottomright_horiz_.at(i).first))
				bottomright_horiz_.removeAt(i);
			else
				i++;
		i = 0;
		//bottomright_vert_
		while(i<bottomright_vert_.size())
			if (non_persistent_widgets.contains(bottomright_vert_.at(i).first))
				bottomright_vert_.removeAt(i);
			else
				i++;
        i = 0;
		//topleft_horiz_
		while(i<topleft_horiz_.size())
			if (non_persistent_widgets.contains(topleft_horiz_.at(i).first))
				topleft_horiz_.removeAt(i);
			else
				i++;
        i = 0;
		//topleft_vert_
		while(i<topleft_vert_.size())
			if (non_persistent_widgets.contains(topleft_vert_.at(i).first))
				topleft_vert_.removeAt(i);
			else
				i++;
        i = 0;
		//topleft_vert_
		while(i<topright_horiz_.size())
			if (non_persistent_widgets.contains(topright_horiz_.at(i).first))
				topright_horiz_.removeAt(i);
			else
				i++;
        i = 0;
		//topleft_vert_
		while(i<topright_vert_.size())
			if (non_persistent_widgets.contains(topright_vert_.at(i).first))
				topright_vert_.removeAt(i);
			else
				i++;
		AddInternalWidgets();
	}

	void InworldSceneController::AddInternalWidgets()
	{
		//Delete items from layout/ Clear it
		layout_manager_->resetLayout();
		//Get each list and add it to the main layout

		//Qt::Corner::BottomLeftCorner
		if (!bottomleft_horiz_.empty())
		{
			layout_manager_->AddCornerAnchor(bottomleft_horiz_.at(0).first, Qt::BottomLeftCorner, Qt::BottomLeftCorner);
			if (bottomleft_horiz_.size() > 1)
				for(int i = 1; i<bottomleft_horiz_.size(); i++)
					layout_manager_->AnchorWidgetsHorizontally(bottomleft_horiz_.at(i).first, bottomleft_horiz_.at(i-1).first, false);

			//bottomleft_vert_
			if (!bottomleft_vert_.empty()){
				layout_manager_->AnchorWidgetsVertically(bottomleft_horiz_.at(0).first, bottomleft_vert_.at(0).first, true);
				for(int i = 1; i<bottomleft_vert_.size(); i++)
					layout_manager_->AnchorWidgetsVertically(bottomleft_vert_.at(i-1).first, bottomleft_vert_.at(i).first, true);
			}
		}

		//Qt::Corner::BottomRightCorner
		if (!bottomright_horiz_.empty())
		{
			layout_manager_->AddCornerAnchor(bottomright_horiz_.at(0).first, Qt::BottomRightCorner, Qt::BottomRightCorner);
			if (bottomright_horiz_.size() > 1)
				for(int i = 1; i<bottomright_horiz_.size(); i++)
					layout_manager_->AnchorWidgetsHorizontally(bottomright_horiz_.at(i-1).first, bottomright_horiz_.at(i).first, false);

			//bottomright_vert_
			if (!bottomright_vert_.empty()){
				layout_manager_->AnchorWidgetsVertically(bottomright_horiz_.at(0).first, bottomright_vert_.at(0).first, false);
				for(int i = 1; i<bottomright_vert_.size(); i++)
					layout_manager_->AnchorWidgetsVertically(bottomright_vert_.at(i-1).first, bottomright_vert_.at(i).first, false);
			}
		}

		//Qt::Corner::TopLeftCorner
		if (!topleft_horiz_.empty())
		{
			layout_manager_->AddCornerAnchor(topleft_horiz_.at(0).first, Qt::TopLeftCorner, Qt::TopLeftCorner);
			if (topleft_horiz_.size() > 1)
				for(int i = 1; i<topleft_horiz_.size(); i++)
					layout_manager_->AnchorWidgetsHorizontally(topleft_horiz_.at(i).first, topleft_horiz_.at(i-1).first, true);

			//topleft_vert_
			if (!topleft_vert_.empty()){
				layout_manager_->AnchorWidgetsVertically(topleft_vert_.at(0).first, topleft_horiz_.at(0).first, true);
				for(int i = 1; i<topleft_vert_.size(); i++)
					layout_manager_->AnchorWidgetsVertically(topleft_vert_.at(i).first, topleft_vert_.at(i-1).first, true);
			}
		}

		//Qt::Corner::TopRightCorner
		if (!topright_horiz_.empty())
		{
			layout_manager_->AddCornerAnchor(topright_horiz_.at(0).first, Qt::TopRightCorner, Qt::TopRightCorner);
			if (topright_horiz_.size() > 1)
				for(int i = 1; i<topright_horiz_.size(); i++)
					layout_manager_->AnchorWidgetsHorizontally(topright_horiz_.at(i-1).first, topright_horiz_.at(i).first, true);

			//topright_vert_
			if (!topright_vert_.empty()){
				layout_manager_->AnchorWidgetsVertically(topright_vert_.at(0).first, topright_horiz_.at(0).first, false);
				for(int i = 1; i<topright_vert_.size(); i++)
					layout_manager_->AnchorWidgetsVertically(topright_vert_.at(i).first, topright_vert_.at(i-1).first, false);
			}
		}
	}
}
