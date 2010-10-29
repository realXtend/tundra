#include "StableHeaders.h"
#include "TerrainWeightEditor.h"
#include "EnvironmentModule.h"
#include "EC_Terrain.h"

#include <QUiLoader>
#include <QFile>
#include <QFileDialog>
#include <QPixmap>
#include <QLabel>

#include "TerrainLabel.h"
#include "Framework.h"
#include "SceneManager.h"
#include "UiProxyWidget.h"
#include "UiServiceInterface.h"

#include "ConsoleCommandServiceInterface.h"
namespace Environment
{
    TerrainWeightEditor::TerrainWeightEditor(EnvironmentModule* env_module)
        :env_module_(env_module)
        ,scene_manager_(0),
        brush_size_(1),
        brush_modifier_(1),
        falloff_percentage_(0),
        neutral_color_(128),
        brush_size_max_(800)
    {
        
    }
    TerrainWeightEditor::~TerrainWeightEditor()
    {

    
    }

    void TerrainWeightEditor::Initialize()
    {
        UiServiceInterface *ui = env_module_->GetFramework()->GetService<UiServiceInterface>();
        if (!ui) // If this occurs, we're most probably operating in headless mode.
            return;

        QUiLoader loader;
        loader.setLanguageChangeEnabled(true);
        QString str("./data/ui/terrainwtex_editor.ui");
        QFile file(str);
        if(!file.exists())
        {
            EnvironmentModule::LogError("Cannot find " +str.toStdString()+ " file");
            return;
        }
        editor_widget_ = loader.load(&file, this);
        if (editor_widget_ == 0)
            return;
        UiProxyWidget *editor_proxy = ui->AddWidgetToScene(this);
        if(editor_proxy == 0)
            return;
        ui->AddWidgetToMenu(this, tr("Terrain Texture Weightmap Editor"));
        ui->RegisterUniversalWidget("Weights", editor_proxy);


        QSpinBox *box = editor_widget_->findChild<QSpinBox*>("brush_size");
        if(box)
        {
            brush_size_max_ = box->maximum();
            brush_size_ = box->value();
        }
        box = editor_widget_->findChild<QSpinBox*>("brush_modifier");
        if(box)
            brush_modifier_ = box->maximum();

        QDoubleSpinBox *dbox = editor_widget_->findChild<QDoubleSpinBox*>("brush_falloff");
        if(dbox)
            falloff_percentage_ = dbox->value();



        
        InitializeCanvases();
        InitializeConnections();
        emit BrushValueChanged();
    }


    Console::CommandResult TerrainWeightEditor::ShowWindow(const StringVector &params)
    {
        UiServiceInterface *ui = env_module_->GetFramework()->GetService<UiServiceInterface>();
        if (!ui) 
            return Console::ResultFailure("Failed to acquire UiModule pointer!");;

        ui->BringWidgetToFront(this);
        return Console::ResultSuccess();
    }

    void TerrainWeightEditor::DecomposeImageToCanvases(const QImage& image)
    {
        QLabel *canvas1 = editor_widget_->findChild<QLabel *>("canvas_1");
        QLabel *canvas2 = editor_widget_->findChild<QLabel *>("canvas_2");
        QLabel *canvas3 = editor_widget_->findChild<QLabel *>("canvas_3");
        QLabel *canvas4 = editor_widget_->findChild<QLabel *>("canvas_4");
        
        assert(canvas1 && canvas2 && canvas3 && canvas4);

        int width = image.width();
        int height = image.height();
        QImage::Format format = image.format();

        QImage image1(width,height,format);
        QImage image2(width,height,format);
        QImage image3(width,height,format);
        QImage image4(width,height,format);

        for(int i=0;i<width;i++)
        {
            for(int j=0; j<height;j++)
            {
                QRgb color = image.pixel(i,j);
                int alpha = qAlpha(color);
                int red = qRed(color);
                int green = qGreen(color);
                int blue = qBlue(color);

                image1.setPixel(i,j, QColor(red,red,red).rgba());
                image2.setPixel(i,j, QColor(green,green,green).rgba());
                image3.setPixel(i,j, QColor(blue,blue,blue).rgba());
                image4.setPixel(i,j, QColor(alpha,alpha,alpha).rgba());
            }
        }

        canvas1->setPixmap(QPixmap::fromImage(image1));
        canvas2->setPixmap(QPixmap::fromImage(image2));
        canvas3->setPixmap(QPixmap::fromImage(image3));
        canvas4->setPixmap(QPixmap::fromImage(image4));

    }

    void TerrainWeightEditor::InitializeCanvases()
    {

        QGroupBox* group = editor_widget_->findChild<QGroupBox*>("weight_group");
        if(!group)
            return;

        QGridLayout* layout = dynamic_cast<QGridLayout*>(group->layout());
        if(!layout)
            return;

        TerrainLabel* label1 = new TerrainLabel(this);
        label1->setObjectName("canvas_1");
        label1->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        label1->setScaledContents(true);
        layout->addWidget(label1,0,0);

        TerrainLabel* label2 = new TerrainLabel(this);
        label2->setObjectName("canvas_2");
        label2->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        label2->setScaledContents(true);
        layout->addWidget(label2,0,1);

        TerrainLabel* label3 = new TerrainLabel(this);
        label3->setObjectName("canvas_3");
        label3->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        label3->setScaledContents(true);
        layout->addWidget(label3,2,0);

        TerrainLabel* label4 = new TerrainLabel(this);
        label4->setObjectName("canvas_4");
        label4->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        label4->setScaledContents(true);
        layout->addWidget(label4,2,1);
        


        QLabel *canvas1 = editor_widget_->findChild<QLabel *>("canvas_1");
        QLabel *canvas2 = editor_widget_->findChild<QLabel *>("canvas_2");
        QLabel *canvas3 = editor_widget_->findChild<QLabel *>("canvas_3");
        QLabel *canvas4 = editor_widget_->findChild<QLabel *>("canvas_4");

        QLabel *brush_canvas = editor_widget_->findChild<QLabel *>("brush_label");
        
        assert(canvas1 && canvas2 && canvas3 && canvas4 && brush_canvas);
        QPixmap basic(1024,1024);
        basic.fill(Qt::black);

        canvas1->setPixmap(basic);
        canvas2->setPixmap(basic);
        canvas3->setPixmap(basic);
        canvas4->setPixmap(basic);

        QPixmap brush(256, 256);
        brush.fill(QColor(neutral_color_,neutral_color_,neutral_color_));
        brush_canvas->setPixmap(brush);
    }

    void TerrainWeightEditor::HandleMouseEventFromCanvas(QMouseEvent *ev, QString name)
    {
        if(ev->buttons() && Qt::LeftButton)
        {
            QLabel *canvas = editor_widget_->findChild<QLabel *>(name);
            if(!canvas)
                return;
            QImage image = canvas->pixmap()->toImage();
            
            

            //in canvas space, between [0,1]
            qreal brush_pos_x = ev->posF().x()/canvas->width();
            qreal brush_pos_y = ev->posF().y()/canvas->height();

            //in image space (texel coordinates)
            brush_pos_x = brush_pos_x*image.width();
            brush_pos_y = brush_pos_y*image.height();

            qreal half_brush_size = brush_size_*0.5f;

            int start_pos_x = brush_pos_x-half_brush_size;
            int start_pos_y = brush_pos_y-half_brush_size;
            int end_pos_x = brush_pos_x+half_brush_size;
            int end_pos_y = brush_pos_y+half_brush_size;

            //check limits
            start_pos_x = start_pos_x<0?0:start_pos_x;
            start_pos_y = start_pos_y<0?0:start_pos_y;
            end_pos_x = end_pos_x<image.width()? end_pos_x:image.width();
            end_pos_y = end_pos_y<image.height()? end_pos_y:image.height();

            for(int x = start_pos_x; x<end_pos_x;x++)
            {
                for(int y = start_pos_y; y<end_pos_y;y++)
                {
                    QRgb color = image.pixel(x,y);

                    f32 dist = sqrt(pow(x-brush_pos_x,2)+pow(y-brush_pos_y,2));
                    if(dist>half_brush_size)
                        continue;

                    int mod = abs(brush_modifier_) - abs(dist*(brush_modifier_*falloff_percentage_));

                    if(mod<1)
                        continue;

                    if(brush_modifier_<0)
                        mod = -mod;
                    int new_col = qBlue(color)+mod;
                    
                    //Limits (again)
                    new_col = new_col>255?255:new_col;
                    new_col = new_col<0?0:new_col;

                    image.setPixel(x,y, QColor(new_col,new_col,new_col).rgb());

                }
            }
            canvas->setPixmap(QPixmap::fromImage(image));
        }
    }

    QImage TerrainWeightEditor::CreateImageFromCanvases()
    {

        QLabel *canvas1 = editor_widget_->findChild<QLabel *>("canvas_1");
        QLabel *canvas2 = editor_widget_->findChild<QLabel *>("canvas_2");
        QLabel *canvas3 = editor_widget_->findChild<QLabel *>("canvas_3");
        QLabel *canvas4 = editor_widget_->findChild<QLabel *>("canvas_4");
        
        assert(canvas1 && canvas2 && canvas3 && canvas4);

        QImage image1  = canvas1->pixmap()->toImage();
        QImage image2  = canvas2->pixmap()->toImage();
        QImage image3  = canvas3->pixmap()->toImage();
        QImage image4  = canvas4->pixmap()->toImage();
        
        QImage ret_image(image1.width(), image1.height(),QImage::Format_ARGB32);

        for(int i=0;i<ret_image.width();i++)
        {
            for(int j=0; j<ret_image.height();j++)
            {
                int red = qRed(image1.pixel(i,j));
                int green = qRed(image2.pixel(i,j));
                int blue = qRed(image3.pixel(i,j));
                int alpha = qRed(image4.pixel(i,j));


                ret_image.setPixel(i,j, QColor(red,green,blue,alpha).rgba());
            }
        }
        return ret_image;
    }

    void TerrainWeightEditor::InitializeConnections()
    {

        connect(this, SIGNAL(BrushValueChanged()), this, SLOT(RecreateBrush()));

        if(!editor_widget_)
            return;

        QLabel *canvas1 = editor_widget_->findChild<QLabel *>("canvas_1");
        QLabel *canvas2 = editor_widget_->findChild<QLabel *>("canvas_2");
        QLabel *canvas3 = editor_widget_->findChild<QLabel *>("canvas_3");
        QLabel *canvas4 = editor_widget_->findChild<QLabel *>("canvas_4");

        connect(canvas1, SIGNAL(SendMouseEventWithCanvasName(QMouseEvent*, QString)), this, SLOT(HandleMouseEventFromCanvas(QMouseEvent*, QString)));
        connect(canvas2, SIGNAL(SendMouseEventWithCanvasName(QMouseEvent*, QString)), this, SLOT(HandleMouseEventFromCanvas(QMouseEvent*, QString)));
        connect(canvas3, SIGNAL(SendMouseEventWithCanvasName(QMouseEvent*, QString)), this, SLOT(HandleMouseEventFromCanvas(QMouseEvent*, QString)));
        connect(canvas4, SIGNAL(SendMouseEventWithCanvasName(QMouseEvent*, QString)), this, SLOT(HandleMouseEventFromCanvas(QMouseEvent*, QString)));

        QPushButton *load_button = editor_widget_->findChild<QPushButton *>("loadfromfilebutton");
        if(load_button)
            connect(load_button, SIGNAL(clicked()), this, SLOT(LoadWeightmapFromFile()));

        QPushButton *apply_button = editor_widget_->findChild<QPushButton *>("apply_button");
        if(apply_button)
            connect(apply_button, SIGNAL(clicked()), this, SLOT(ApplyWeightTexture()));

        QGroupBox *group = editor_widget_->findChild<QGroupBox *>("weight_group");
        if(group)
            connect(group, SIGNAL(toggled(bool)),this, SLOT(SetUseWeights(bool)));

        QSpinBox *box = editor_widget_->findChild<QSpinBox*>("brush_size");
        if(box)
            connect(box, SIGNAL(valueChanged(int)), this, SLOT(BrushSizeChanged(int)));

        QDoubleSpinBox *boxd = editor_widget_->findChild<QDoubleSpinBox*>("brush_falloff");
        if(boxd)
            connect(boxd, SIGNAL(valueChanged(double)), this, SLOT(BrushFalloffChanged(double)));

        box = editor_widget_->findChild<QSpinBox*>("brush_modifier");
        if(box)
            connect(box, SIGNAL(valueChanged(int)), this, SLOT(BrushModifierChanged(int)));


    }

    void TerrainWeightEditor::BrushSizeChanged(int val)
    {
        brush_size_ = val;
        emit BrushValueChanged();
    }
    void TerrainWeightEditor::BrushFalloffChanged(double val)
    {
        falloff_percentage_ = val;
        emit BrushValueChanged();
    }
    void TerrainWeightEditor::BrushModifierChanged(int val)
    {
        brush_modifier_ = val;
        emit BrushValueChanged();
    }

    void TerrainWeightEditor::RecreateBrush()
    {
        QLabel *brush_canvas = editor_widget_->findChild<QLabel *>("brush_label");
        if(!brush_canvas)
            return;
        QPixmap map(brush_canvas->pixmap()->width(),brush_canvas->pixmap()->height()) ;
        map.fill(QColor(neutral_color_, neutral_color_, neutral_color_));
        QImage image = map.toImage();
        
        QPoint middle(image.width()*0.5f, image.height()*0.5f);
        f32 scale = static_cast<f32>(brush_size_max_)/image.width();
        for(int x=0; x<image.width();x++)
        {
            for(int y=0; y<image.height();y++)
            {
                int col;
                f32 dist = sqrt(pow(static_cast<f32>(x-middle.x()),2)+pow(static_cast<f32>(y-middle.y()),2));

                if((dist*scale)>(brush_size_*0.5f))
                {
                    col = neutral_color_;
                }
                else
                {
                    int mod = abs(brush_modifier_) - abs(dist*(brush_modifier_*falloff_percentage_));

                    if(mod<1)
                        mod = 0;

                    if(brush_modifier_<0)
                        mod = -mod;
                    int col = neutral_color_ + mod;
                    image.setPixel(x,y, QColor(col,col,col).rgb());
                }
            }
        }
        brush_canvas->setPixmap(QPixmap::fromImage(image));
    }

    void TerrainWeightEditor::ApplyWeightTexture()
    {
        if(!GetSceneManager())
            return;

        

        QImage map  = CreateImageFromCanvases();
        
        if(map.format() != QImage::Format_ARGB32)
        {
            map = map.convertToFormat(QImage::Format_ARGB32/*,Qt::NoOpaqueDetection*/);
        }
        Ogre::Box bounds(0, 0, map.width(), map.height());
        Ogre::PixelBox bufbox(bounds, Ogre::PF_A8R8G8B8, (void *)map.bits());
        Scene::EntityList list = scene_manager_->GetEntitiesWithComponent("EC_Terrain");
        Scene::EntityList::const_iterator it = list.begin();
        while(it!= list.end())
        {
            boost::shared_ptr<EC_Terrain> ptr = (*it)->GetComponent<EC_Terrain>();
            QString texname;
            texname += (*it)->GetName();
            texname += ptr->TypeName();
            texname += ptr->Name();
            Ogre::String str(texname.toStdString());
            Ogre::TextureManager::getSingleton().remove(str);
            Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().createManual(str, "General", Ogre::TEX_TYPE_2D, bufbox.getWidth(), bufbox.getHeight(),bufbox.getDepth(),0,bufbox.format,Ogre::TU_DYNAMIC_WRITE_ONLY);
            if(tex.get())
            {
                tex->getBuffer()->blitFromMemory(bufbox);
            }
            ptr->settexture0(texname);
            it++;
            Ogre::Image img;
            img.loadDynamicImage(static_cast<Ogre::uchar*>(bufbox.data),bufbox.getWidth(), bufbox.getHeight(), bufbox.getDepth(), bufbox.format);
            
            img.save("lol.png");
        }
    }

    Scene::SceneManager* TerrainWeightEditor::GetSceneManager()
    {
        if(!scene_manager_)
        {
            Scene::ScenePtr ptr = env_module_->GetFramework()->GetDefaultWorldScene();
            if(ptr.get())
                scene_manager_ = ptr.get();
        }
         return scene_manager_;
    }

    void TerrainWeightEditor::SetUseWeights(bool val)
    {
        if(!GetSceneManager())
            return;
        Scene::EntityList list = scene_manager_->GetEntitiesWithComponent("EC_Terrain");
        Scene::EntityList::const_iterator it = list.begin();
        while(it!= list.end())
        {
            boost::shared_ptr<EC_Terrain> ptr = (*it)->GetComponent<EC_Terrain>();
            if(val)
            {
                ptr->setmaterial("Rex/TerrainPCF_weighted");
            }
            else
            {
                ptr->setmaterial("Rex/TerrainPCF");
            }
            it++;
        }
    }

    void TerrainWeightEditor::LoadWeightmapFromFile()
    {

        QString fileName = QFileDialog::getOpenFileName(this, tr("Select Image"),"" , tr("Image Files (*.png)"));
        QPixmap image(fileName);

        DecomposeImageToCanvases(image.toImage());
    }
}