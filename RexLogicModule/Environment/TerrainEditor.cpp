#include "StableHeaders.h"
#include "QtModule.h"
//#include "RexProtocolMsgIDs.h"
//#include "RexServerConnection.h"
//#include "OpenSimProtocolModule.h"

#include "RexLogicModule.h"
#include "Environment/TerrainDecoder.h"
#include "Environment/Terrain.h"
#include "Environment/TerrainEditor.h"
#include "Environment/TerrainLabel.h"

#include <QtUiTools>
#include <QPushButton>
#include <QImage>
#include <QLabel>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>
#include <QRadioButton>
#include <QGroupBox>

namespace RexLogic
{
    TerrainEditor::TerrainEditor(RexLogicModule* rexlogicmodule):
    rexlogicmodule_(rexlogicmodule),
    editor_widget_(0),
    action_(Flatten),
    brush_size_(Small)
    //mouse_press_flag_(no_button)
    {
        InitEditorWindow();
        terrain_ = rexlogicmodule->GetTerrainHandler();
    }

    TerrainEditor::~TerrainEditor()
    {

    }

    void TerrainEditor::Toggle()
    {
        if(canvas_.get())
        {
            if(canvas_->IsHidden())
            {
                canvas_->Show();
                UpdateTerrain();
            }
            else
                canvas_->Hide();
        }
    }

    void TerrainEditor::CreateHeightmapImage()
    {
        if(terrain_.get() && editor_widget_)
        {
            Scene::EntityPtr entity = terrain_->GetTerrainEntity().lock();
            EC_Terrain *terrain_component = checked_static_cast<EC_Terrain *>(entity->GetComponent("EC_Terrain").get());

            if(terrain_component->AllPatchesLoaded())
            {
                QLabel *label = editor_widget_->findChild<QLabel *>("map_label");
                const QPixmap *pixmap = label->pixmap();
                QImage image = pixmap->toImage();

                // Make sure that image is in right size.
                if(image.height() != 256 || image.width() != 256)
                    return;

                // Generate image based on heightmap values. The Highest value in heightmap will show on image as white and the lowest as black.
                MinMaxValue values = GetMinMaxHeightmapValue(*terrain_component);
                for(int height = 0; height < image.height(); height++)
                {
                    for(int width = 0; width < image.width(); width++)
                    {
                        float value = (terrain_component->GetPoint(width, height) - values.first);
                        value /= (values.second - values.first);
                        value *= 255;
                        QRgb color_value = qRgb(value, value, value);
                        image.setPixel(width, height, color_value);
                    }
                }

                label->setPixmap(QPixmap::fromImage(image));
                label->show();
            }
        }
    }

    MinMaxValue TerrainEditor::GetMinMaxHeightmapValue(EC_Terrain &terrain)
    {
        float min, max;
        min = 65535.0f;
        max = 0.0f;

        MinMaxValue values;

        if(terrain.AllPatchesLoaded())
        {
            for(int i = 0; i < terrain.cNumPatchesPerEdge; i++)
            {
                for(int j = 0; j < terrain.cNumPatchesPerEdge; j++)
                {
                    const EC_Terrain::Patch &patch = terrain.GetPatch(i, j);
                    std::vector<float> height_data = patch.heightData;
                    for(int k = 0; k < height_data.size(); k++)
                    {
                        float value = height_data[k];
                        if(value < min)
                        {
                            min = value;
                            values.first = value;
                        }
                        else if(value > max)
                        {
                            max = value;
                            values.second = value;
                        }
                    }
                }
            }
        }
        return values;
    }

    void TerrainEditor::InitEditorWindow()
    {
        boost::shared_ptr<QtUI::QtModule> qt_module = rexlogicmodule_->GetFramework()->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();

        // Make sure that qt module was found.
        if(qt_module.get() == 0)
            return;

        // \todo Right now mouse move event wont be sented if left mouse button is down, we need to use external window and 
        canvas_ = qt_module->CreateCanvas(QtUI::UICanvas::External).lock();
        QUiLoader loader;
        QFile file("./data/ui/terrain_editor.ui");

        if(!file.exists())
        {
            RexLogicModule::LogError("Cannot find terrain editor ui file");
            return;
        }

        editor_widget_ = loader.load(&file);
        if(!editor_widget_)
            return;

        QSize size = editor_widget_->size(); 
        canvas_->SetSize(size.width(), size.height());
        canvas_->AddWidget(editor_widget_);
        canvas_->SetPosition(60, 60);

        QWidget *map_widget = editor_widget_->findChild<QWidget *>("map_widget");
        if(map_widget)
        {
            TerrainLabel *label = new TerrainLabel(map_widget, 0);
            label->setObjectName("map_label");
            QObject::connect(label, SIGNAL(SendMouseEvent(QMouseEvent*)), this, SLOT(HandleMouseEvent(QMouseEvent*)));
            label->resize(map_widget->size());

            // Create a QImage object and set it in label.
            QImage heightmap(cHeightmapImageWidth, cHeightmapImageHeight, QImage::Format_RGB32);
            label->setPixmap(QPixmap::fromImage(heightmap));
            label->show();
        }

        // Set signals
        QPushButton *update_button = editor_widget_->findChild<QPushButton *>("button_update");
        QObject::connect(update_button, SIGNAL(clicked()), this, SLOT(UpdateTerrain(UpdateTerrain())));

        QRadioButton *rad_button_flatten = editor_widget_->findChild<QRadioButton *>("rad_button_flatten");
        QRadioButton *rad_button_raise = editor_widget_->findChild<QRadioButton *>("rad_button_raise");
        QRadioButton *rad_button_lower = editor_widget_->findChild<QRadioButton *>("rad_button_lower");
        QRadioButton *rad_button_smooth = editor_widget_->findChild<QRadioButton *>("rad_button_smooth");
        QRadioButton *rad_button_roughen = editor_widget_->findChild<QRadioButton *>("rad_button_roughen");
        QRadioButton *rad_button_revert = editor_widget_->findChild<QRadioButton *>("rad_button_revert");

        QObject::connect(rad_button_flatten, SIGNAL(clicked()), this, SLOT(PaintActionChanged()));
        QObject::connect(rad_button_raise, SIGNAL(clicked()), this, SLOT(PaintActionChanged()));
        QObject::connect(rad_button_lower, SIGNAL(clicked()), this, SLOT(PaintActionChanged()));
        QObject::connect(rad_button_smooth, SIGNAL(clicked()), this, SLOT(PaintActionChanged()));
        QObject::connect(rad_button_roughen, SIGNAL(clicked()), this, SLOT(PaintActionChanged()));
        QObject::connect(rad_button_revert, SIGNAL(clicked()), this, SLOT(PaintActionChanged()));

        QRadioButton *rad_button_small = editor_widget_->findChild<QRadioButton *>("rad_button_small");
        QRadioButton *rad_button_medium = editor_widget_->findChild<QRadioButton *>("rad_button_medium");
        QRadioButton *rad_button_large = editor_widget_->findChild<QRadioButton *>("rad_button_large");

        QObject::connect(rad_button_small, SIGNAL(clicked()), this, SLOT(BrushSizeChanged()));
        QObject::connect(rad_button_medium, SIGNAL(clicked()), this, SLOT(BrushSizeChanged()));
        QObject::connect(rad_button_large, SIGNAL(clicked()), this, SLOT(BrushSizeChanged()));

        // Add canvas to control bar
        qt_module->AddCanvasToControlBar(canvas_, QString("Terrain Editor"));
        file.close();
    }

    void TerrainEditor::SendModifyLandMessage()
    {
        
    }

    void TerrainEditor::UpdateTerrain()
    {
        // We only need to update terrain information when window is visible.
        if(canvas_.get())
        {
            if(canvas_->IsHidden())
            {
                return;
            }
        }

        assert(rexlogicmodule_);
        if(!rexlogicmodule_)
        {
            RexLogicModule::LogError("Can't update terrain because rexlogicmodule is not intialized.");
            return;
        }

        terrain_ = rexlogicmodule_->GetTerrainHandler();
        if(!terrain_)
            return;

        CreateHeightmapImage();
    }

    void TerrainEditor::HandleMouseEvent(QMouseEvent *ev)
    {
        // Ugly this need to be removed when mouse move events are working corretly in Rex UICanvas.
        // Check if mouse has pressed.
        /*if(ev->type() == QEvent::MouseButtonPress)
        {
            QPoint position = ev->pos();
            Scene::EntityPtr entity = terrain_->GetTerrainEntity().lock();
            EC_Terrain *terrain_component = checked_static_cast<EC_Terrain *>(entity->GetComponent("EC_Terrain").get());
            start_height_ = terrain_component->GetPoint(position.x(), position.y());

            switch(ev->button())
            {
                case Qt::LeftButton:
                    mouse_press_flag_ |= left_button;
                    break;
                case Qt::RightButton:
                    mouse_press_flag_ |= right_button;
                    break;
                case Qt::MidButton:
                    mouse_press_flag_ |= middle_button;
                    break;
            }
        }
        else if(ev->type() == QEvent::MouseButtonRelease)
        {
            switch(ev->button())
            {
                case Qt::LeftButton:
                    mouse_press_flag_ -= left_button;
                    break;
                case Qt::RightButton:
                    mouse_press_flag_ -= right_button;
                    break;
                case Qt::MidButton:
                    mouse_press_flag_ -= middle_button;
                    break;
            }
        }

        if(mouse_press_flag_ & button_mask > 0)
        {
                QLabel *label = editor_widget_->findChild<QLabel *>("map_label");
                const QPixmap *pixmap = label->pixmap();
                QImage image = pixmap->toImage();
                image.setPixel(ev->pos(), qRgb(255, 255, 255));
                label->setPixmap(QPixmap::fromImage(image));
                label->show();

                QPoint position = ev->pos();
                Scene::EntityPtr entity = terrain_->GetTerrainEntity().lock();
                EC_Terrain *terrain_component = checked_static_cast<EC_Terrain *>(entity->GetComponent("EC_Terrain").get());
                rexlogicmodule_->SendModifyLandMessage(position.x(), position.y(), brush_size_, action_, 0.15f, start_height_);
        }*/

        if(ev->type() == QEvent::MouseButtonPress || ev->type() == QEvent::MouseMove)
        {
            if(ev->button() == Qt::LeftButton)
            {
                // Draw a white point where we have clicked.
                QLabel *label = editor_widget_->findChild<QLabel *>("map_label");
                const QPixmap *pixmap = label->pixmap();
                QImage image = pixmap->toImage();
                image.setPixel(ev->pos(), qRgb(255, 255, 255));
                label->setPixmap(QPixmap::fromImage(image));
                label->show();

                // Send modify land message to server.
                QPoint position = ev->pos();
                if(!terrain_.get())
                    return;

                Scene::EntityPtr entity = terrain_->GetTerrainEntity().lock();
                EC_Terrain *terrain_component = checked_static_cast<EC_Terrain *>(entity->GetComponent("EC_Terrain").get());
                if(!terrain_component)
                    return;

                rexlogicmodule_->SendModifyLandMessage(position.x(), position.y(), brush_size_, action_, 0.15f, terrain_component->GetPoint(position.x(), position.y()));
            }
        }
    }

    void TerrainEditor::BrushSizeChanged()
    {
        QRadioButton *rad_button = editor_widget_->findChild<QRadioButton *>("rad_button_small");
        if(rad_button->isChecked())
        {
            brush_size_ = Small;
            return;
        }

        rad_button = editor_widget_->findChild<QRadioButton *>("rad_button_medium");
        if(rad_button->isChecked())
        {
            brush_size_ = Medium;
            return;
        }

        rad_button = editor_widget_->findChild<QRadioButton *>("rad_button_large");
        if(rad_button->isChecked())
        {
            brush_size_ = Large;
            return;
        }
    }

    void TerrainEditor::PaintActionChanged()
    {
        QRadioButton *rad_button = editor_widget_->findChild<QRadioButton *>("rad_button_flatten");
        if(rad_button->isChecked())
        {
            action_ = Flatten;
            return;
        }

        rad_button = editor_widget_->findChild<QRadioButton *>("rad_button_raise");
        if(rad_button->isChecked())
        {
            action_ = Raise;
            return;
        }

        rad_button = editor_widget_->findChild<QRadioButton *>("rad_button_lower");
        if(rad_button->isChecked())
        {
            action_ = Lower;
            return;
        }

        rad_button = editor_widget_->findChild<QRadioButton *>("rad_button_smooth");
        if(rad_button->isChecked())
        {
            action_ = Smooth;
            return;
        }

        rad_button = editor_widget_->findChild<QRadioButton *>("rad_button_roughen");
        if(rad_button->isChecked())
        {
            action_ = Roughen;
            return;
        }

        rad_button = editor_widget_->findChild<QRadioButton *>("rad_button_revert");
        if(rad_button->isChecked())
        {
            action_ = Revert;
            return;
        }
    }
}