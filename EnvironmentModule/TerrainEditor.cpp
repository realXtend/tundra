#include "StableHeaders.h"
#include "QtModule.h"
//#include "RexProtocolMsgIDs.h"
//#include "RexServerConnection.h"
//#include "OpenSimProtocolModule.h"

#include "EnvironmentModule.h"
#include "TerrainDecoder.h"
#include "Terrain.h"
#include "TerrainEditor.h"
#include "TerrainLabel.h"

#include "TextureInterface.h"
#include "TextureServiceInterface.h"

#include <QtUiTools>
#include <QPushButton>
#include <QImage>
#include <QLabel>
#include <QColor>
#include <QMouseEvent>
#include <QRadioButton>
#include <QGroupBox>
#include <QTabWidget>
#include <QLineEdit>

namespace Environment
{
    TerrainEditor::TerrainEditor(EnvironmentModule* environment_module):
    environment_module_(environment_module),
    editor_widget_(0),
    action_(Flatten),
    brush_size_(Small)
    //mouse_press_flag_(no_button)
    {
        InitEditorWindow();

        // Those two arrays size should always be the same as how many terrain textures we are using.
        terrain_texture_id_list_.resize(cNumberOfTerrainTextures);
        terrain_texture_requests_.resize(cNumberOfTerrainTextures);
        terrain_ = environment_module_->GetTerrainHandler();
    }

    TerrainEditor::~TerrainEditor()
    {
        boost::shared_ptr<QtUI::QtModule> qt_module = environment_module_->GetFramework()->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();
        if(qt_module.get() && canvas_)
        {
            qt_module->DeleteCanvas(canvas_->GetID());
        }
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
                // Find image label in widget so we can get the basic information about the image that we are about to update.
                QLabel *label = editor_widget_->findChild<QLabel *>("map_label");
                const QPixmap *pixmap = label->pixmap();
                QImage image = pixmap->toImage();

                // Make sure that image is in right size.
                if(image.height() != cHeightmapImageHeight || image.width() != cHeightmapImageWidth)
                    return;

                // Generate image based on heightmap values. The Highest value in heightmap will show on image as white color and the lowest as black color.
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

                // Set new image into the label.
                label->setPixmap(QPixmap::fromImage(image));
                label->show();
            }
        }
    }

    MinMaxValue TerrainEditor::GetMinMaxHeightmapValue(EC_Terrain &terrain) const
    {
        float min, max;
        min = 65535.0f;
        max = 0.0f;

        MinMaxValue values;

        if(terrain.AllPatchesLoaded())
        {
            for(int i = 0; i < cHeightmapImageWidth; i++)
            {
                for(int j = 0; j < cHeightmapImageHeight; j++)
                {
                    float height_value = terrain.GetPoint(i, j);
                    if(height_value < min)
                    {
                        min = height_value;
                        values.first = height_value;
                    }
                    else if(height_value > max)
                    {
                        max = height_value;
                        values.second = height_value;
                    }
                }
            }
        }
        return values;
    }

    void TerrainEditor::InitEditorWindow()
    {
        boost::shared_ptr<QtUI::QtModule> qt_module = environment_module_->GetFramework()->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();

        // Make sure that qt module was found.
        if(qt_module.get() == 0)
            return;

        canvas_ = qt_module->CreateCanvas(QtUI::UICanvas::Internal).lock();
        QUiLoader loader;
        QFile file("./data/ui/terrain_editor.ui");

        if(!file.exists())
        {
            EnvironmentModule::LogError("Cannot find terrain editor ui file");
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
        QObject::connect(update_button, SIGNAL(clicked()), this, SLOT(UpdateTerrain()));

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

        QTabWidget *tab_widget = editor_widget_->findChild<QTabWidget *>("tabWidget");
        QObject::connect(tab_widget, SIGNAL(currentChanged(int)), this, SLOT(TabWidgetChanged(int)));

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

        assert(environment_module_);
        if(!environment_module_)
        {
            EnvironmentModule::LogError("Can't update terrain because rexlogicmodule is not intialized.");
            return;
        }

        terrain_ = environment_module_->GetTerrainHandler();
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
                environment_module_->SendModifyLandMessage(position.x(), position.y(), brush_size_, action_, 0.15f, start_height_);
        }*/

        if(ev->type() == QEvent::MouseButtonPress || ev->type() == QEvent::MouseMove)
        {
            if(ev->button() == Qt::LeftButton)
            {
                // Draw a white point where we have clicked.
                /*QLabel *label = editor_widget_->findChild<QLabel *>("map_label");
                const QPixmap *pixmap = label->pixmap();
                QImage image = pixmap->toImage();
                image.setPixel(ev->pos(), qRgb(255, 255, 255));
                label->setPixmap(QPixmap::fromImage(image));
                label->show();*/

                // Send modify land message to server.
                QPoint position = ev->pos();
                if(!terrain_.get())
                    return;

                Scene::EntityPtr entity = terrain_->GetTerrainEntity().lock();
                EC_Terrain *terrain_component = checked_static_cast<EC_Terrain *>(entity->GetComponent("EC_Terrain").get());
                if(!terrain_component)
                    return;

                environment_module_->SendModifyLandMessage(position.x(), position.y(), brush_size_, action_, 0.15f, terrain_component->GetPoint(position.x(), position.y()));
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

    void TerrainEditor::TabWidgetChanged(int index)
    {
        if(index == 0) // Map tab
        {
            UpdateTerrain();
        }
        else if(index == 1) // Texture tab
        {
            if(!terrain_.get())
                return;

            QLineEdit *line_edit = 0;
            for(Core::uint i = 0; i < cNumberOfTerrainTextures; i++)
            {
                //Get terrain texture asset ids so that we can request those image resources.
                RexTypes::RexAssetID terrain_id = terrain_->GetTerrainTextureID(i);
                QString line_edit_name("texture_line_edit_" + QString("%1").arg(i + 1));

                // Check if terrain texture hasn't changed for last time, if not we dont need to request a new texture resource and we can continue on next texture.
                if(terrain_texture_id_list_[i] == terrain_id)
                    continue;

                terrain_texture_id_list_[i] = terrain_id;

                line_edit = editor_widget_->findChild<QLineEdit *>(line_edit_name);
                if(!line_edit)
                    continue;
                line_edit->setText(QString::fromStdString(terrain_id));

                Foundation::ServiceManagerPtr service_manager = environment_module_->GetFramework()->GetServiceManager();
                if(service_manager)
                {
                    if(service_manager->IsRegistered(Foundation::Service::ST_Texture))
                    {
                        boost::shared_ptr<Foundation::TextureServiceInterface> texture_service = 
                            service_manager->GetService<Foundation::TextureServiceInterface>(Foundation::Service::ST_Texture).lock();
                        if(!texture_service)
                            continue;

                        // Request texture assets.
                        terrain_texture_requests_[i] = texture_service->RequestTexture(terrain_id);
                    }
                }
            }
        }
    }

    void TerrainEditor::HandleResourceReady(Resource::Events::ResourceReady *res)
    {
        for(Core::uint index = 0; index < terrain_texture_requests_.size(); index++)
        {
            if(terrain_texture_requests_[index] == res->tag_)
            {
                Foundation::TextureInterface *tex = dynamic_cast<Foundation::TextureInterface *>(res->resource_.get());
                if(!tex && tex->GetLevel() != 0)
                    return;

                uint size = tex->GetWidth() * tex->GetHeight() * tex->GetComponents();
                QImage img = ConvertToQImage(*tex);//QImage::fromData(tex->GetData(), size);
                QLabel *texture_label = editor_widget_->findChild<QLabel *>("terrain_texture_label_" + QString("%1").arg(index + 1));
                texture_label->setPixmap(QPixmap::fromImage(img));
                texture_label->show();
            }
        }
    }

    QImage TerrainEditor::ConvertToQImage(Foundation::TextureInterface &tex)
    {
        Core::uint img_width        = tex.GetWidth(); 
        Core::uint img_height       = tex.GetHeight(); 
        Core::uint img_components   = tex.GetComponents();
        Core::u8 *data              = tex.GetData();
        Core::uint img_width_step   = img_width * img_components;
        QImage image;

        if(img_width > 0 && img_height > 0 && img_components > 0)
        {
            // For RGB32.
            if(img_components == 3)
            {
                image = QImage(QSize(img_width, img_height), QImage::Format_RGB32);
                for(Core::uint height = 0; height < img_height; height++)
                {
                    for(Core::uint width = 0; width < img_width; width++)
                    {
                        Core::u8 color[3];
                        for(Core::uint comp = 0; comp < img_components; comp++)
                        {
                            Core::uint index = (height % img_height) * (img_width_step) + ((width * img_components) % (img_width_step)) + comp;
                            color[comp] = data[index];
                        }
                        image.setPixel(width, height, qRgb(color[0], color[1], color[2]));
                    }
                }
            }
        }

        return image;
    }
}