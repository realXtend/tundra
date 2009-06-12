
#include "Ogre3DWidget.h"


#ifndef Q_WS_WIN
#include <QX11Info>
#endif

#include <QDebug>
#include <QMouseEvent>

#include <OgreHardwarePixelBuffer.h>
#include <OgreTexture.h>
#include <OgreMaterial.h>
#include <OgreTextAreaOverlayElement.h>
#include <OgreFontManager.h> 
#include <OgrePanelOverlayElement.h>
#include <OgreTextureUnitState.h>


#include "RexWidget.h"

bool Ogre3DWidget::sceneExist_ = false;

Ogre3DWidget::Ogre3DWidget(QWidget* parent, const QRect& geometry) :
  QWidget(parent),
  pRenderWindow_(0),
  pSceneManager_(0),
  pCamera_(0),
  pViewport_(0),
  activeChild_(""),
  activeMode_(Plane)
   
{
  setAttribute(Qt::WA_PaintOnScreen);  
  //setAttribute(Qt::WA_OpaquePaintEvent);  
  setAttribute(Qt::WA_PaintOutsidePaintEvent);
  //setAttribute(Qt::WA_NoSystemBackground); 
  //setAutoFillBackground(true);  
  setAcceptDrops(true);

  if (!geometry.isNull())
    setGeometry(geometry);
    
  
  createScene();
  
}

Ogre3DWidget::~Ogre3DWidget()
{
  // This widget owns all registered widgets so finish them. 
 
  qDebug()<<"Ogre3DWidget destructor with childs: "<<lstChilds_.size();
  
  for ( int i = lstChilds_.size(); i--;)
    delete lstChilds_.takeAt(i);
   
  qDebug()<<"Ogre3DWidget destructor END";
  
}


void Ogre3DWidget::createScene()
{
  qDebug()<<"Ogre3DWidget::createScene() START";
  
  // Create scene manager

  pSceneManager_ = Ogre::Root::getSingletonPtr()->createSceneManager(Ogre::ST_GENERIC, "SceneManager");
  
  // Create render window.
  
  Ogre::NameValuePairList params;
  
#ifndef Q_WS_WIN
  
  Ogre::String externalWindowHandleParams;
  QX11Info info = x11Info();
  externalWindowHandleParams  = Ogre::StringConverter::toString((unsigned long)(info.display()));
  externalWindowHandleParams += ":";
  externalWindowHandleParams += Ogre::StringConverter::toString(static_cast<unsigned int>(info.screen()));
  externalWindowHandleParams += ":";
  
  // This which one is used depends that is widget allready created and is show()-method called. 

  if (!isHidden())
    externalWindowHandleParams += Ogre::StringConverter::toString(static_cast<unsigned long>(effectiveWinId()));
  else
    externalWindowHandleParams += Ogre::StringConverter::toString(static_cast<unsigned long>(winId()));

  //externalWindowHandleParams += ":";
  //externalWindowHandleParams += Ogre::StringConverter::toString((unsigned long)(info.visual()));
  
  params["parentWindowHandle"] = externalWindowHandleParams;	
 
#else
  params["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)(HWND)winId());
#endif
  
  pRenderWindow_ = Ogre::Root::getSingletonPtr()->createRenderWindow("View", width(), height(), false, &params);

  if ( pRenderWindow_ == 0 )
    {
      qDebug()<<"Ogre3DWidget::createScene() problem to create render window.";
      return;
    }

  // Create the camera
  
  pCamera_ = pSceneManager_->createCamera("Camera");
  
  // Create one viewport, entire window
  
  pViewport_ = pRenderWindow_->addViewport(pCamera_);
  

  // Alter the camera aspect ratio to match the viewport
  
  pCamera_->setAspectRatio(Ogre::Real(pViewport_->getActualWidth()) / Ogre::Real(pViewport_->getActualHeight()));    
  
  
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

  // Create default plane. 

  Ogre::Plane plane;
  
  plane.normal = Ogre::Vector3::UNIT_Z;
  plane.d = 0.0;
  Ogre::MeshManager::getSingleton().createPlane("Myplane",
						Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
						145,145,10,10,true,1,1,1,Ogre::Vector3::UNIT_Y);
 
  
  // Set default camera location. 

  pCamera_->move(Ogre::Vector3(0,0,700));


  // Here we tell system that render window exist etc. Ogre is ready for duty. 

  sceneExist_ = true;


  // Create default overlay

  pOverlay_ = Ogre::OverlayManager::getSingleton().create("DefaultOverlay");
  
  // Create default container which will hold all widgets as a panel. 
  
  pContainer_ = static_cast<Ogre::OverlayContainer*>(Ogre::OverlayManager::getSingleton()
						     .createOverlayElement("Panel", 
									   "DefaultContainer"));
  
  // Set dimensions as it is size of hole screen. 
  
  pContainer_->setPosition(0,0);
  pContainer_->setDimensions(1.0,1.0);


  // Add container in default overlay
  pOverlay_->add2D(pContainer_);


  qDebug()<<"Ogre3DWidget::createScene() END";

}

void Ogre3DWidget::registerWidget(RexWidget *widget)
{

  // Connect child widget textureUpdate signal to our 3D widget update() slot. 
  // This means in words: "Render this scene when child widget texture is updated"

  QObject::connect(widget, SIGNAL(textureUpdated()),this, SLOT(update()));
  
  lstChilds_.append(widget);
  
  // Create entity for the widget, use default plane mesh.
  
  Ogre::Entity* pEntity = pSceneManager_->createEntity(widget->getID().toStdString().c_str(), "Myplane");
  pEntity->setMaterialName(widget->getMaterialName().toStdString().c_str());
  
  Ogre::SceneNode* pNode = pSceneManager_->getRootSceneNode()->createChildSceneNode(widget->getID().toStdString().c_str());

  // Set widget location in world.
  
  pNode->setPosition(widget->getPosition());

  pNode->attachObject(pEntity);

  // Also set this widget in overlay. 
  
  Ogre::PanelOverlayElement* pPanelArea = static_cast<Ogre::PanelOverlayElement*>
(Ogre::OverlayManager::getSingleton().createOverlayElement("Panel",widget->getID().toStdString().c_str()));

  pPanelArea->setMaterialName(widget->getMaterialName().toStdString().c_str());
  
  // By the default hide. 
  pPanelArea->hide();

  // Set dimensions and position of overlay element.
  
  // At moment use, Qt's own frameGeometry.
  
  QSize area = widget->size();
  
  double w = (area.width())/double(width());
  double h = (area.height())/double(height());
  
  pPanelArea->setDimensions(w,h);
  pContainer_->addChild(pPanelArea);
  


}



void Ogre3DWidget::paintEvent(QPaintEvent* event)
{
  qDebug()<<"Ogre3DWidget::paintEvent() START";

  QWidget::paintEvent(event);
  
  // Inform Ogre that it needs to render a frame. 

  Ogre::Root::getSingletonPtr()->_fireFrameStarted();
  pRenderWindow_->update();
  Ogre::Root::getSingletonPtr()->_fireFrameEnded();
  
  qDebug()<<"Ogre3DWidget::paintEvent() END";
}

void Ogre3DWidget::dragEnterEvent(QDragEnterEvent *event)
{
  // Note at moment we do not check that is event input correct. 

  qDebug()<<"Ogre3DWidget::dragEnterEvent() START";

  event->setDropAction(Qt::MoveAction);
  event->accept();

  qDebug()<<"Ogre3DWidget::dragEnterEvent() END";
}


void Ogre3DWidget::dragMoveEvent(QDragMoveEvent *event)
 {
   // Note at moment we do not check that is event input correct 

   qDebug()<<"Ogre3DWidget::dragMoveEvent() START";

   event->setDropAction(Qt::MoveAction);
   event->accept();

   qDebug()<<"Ogre3DWidget::dragMoveEvent() END";
 }

void Ogre3DWidget::dropEvent(QDropEvent *event)
 {
   qDebug()<<"Ogre3DWidget::dropEvent() START";
  
  
   pDraggedWidget_->move(event->pos()-hotSpot_);
   pDraggedWidget_ = 0;

   qDebug()<<"Ogre3DWidget::dropEvent() END";
 }

void Ogre3DWidget::mouseReleaseEvent(QMouseEvent* event)
{
  qDebug()<<"Ogre3DWidget::mouseReleaseEvent() START";
  

  qDebug()<<"Ogre3DWidget::mouseReleaseEvent() END";

}

void Ogre3DWidget::mousePressEvent(QMouseEvent* event)
{
  qDebug()<<"Ogre3DWidget::mousePressEvent() START";
 

  // First check that is at that position a real child widget if there exist do drag-drop operation for child widget. 
  QWidget *child = childAt(event->pos());
  

  if (child != 0)
    {
      pDraggedWidget_ = child;

      QPixmap pixmap(child->size());
      child->render(&pixmap);
      QPoint hotSpot = event->pos() - child->pos();
      hotSpot_ = hotSpot;
      QDrag* pDrag = new QDrag(this);
      
      QMimeData *mimeData = new QMimeData;
      mimeData->setImageData(pixmap.toImage());
      
      pDrag->setMimeData(mimeData);
      pDrag->setPixmap(pixmap);
      pDrag->setHotSpot(hotSpot);
      child->hide();
      
      if (pDrag->exec(Qt::MoveAction))
	child->close();
      else
	child->show();
      
    }
  else
    {
      // There was any "real" child but is there child inside of render Ogre window?
 
      // Locate event position, now this function needs to done somehow so that it "knows" where widget is set in 3D world 
      // and can then transfer correctly event to correct widget. 
      
     
      Ogre::Entity* pEntity = entityAt(event->pos());
      
      if (pEntity != 0)
	{
	  qDebug()<<"Ogre3DWidget::mousePressEvent() : Found entity";

	  // Show entity as overlay. 
	
	  QString name(pEntity->getName().c_str());
	  int index = 0;
	  
	  for ( int i = 0; i < lstChilds_.size(); ++i)
	    {
	      if ( name == lstChilds_[i]->getID())
		{
		
		  // Here we could just calculate "real" position of event in child widget. 
		  // then we would generate new QMouseEvent which has correct position in child widget world
		  // and send that event to child widget, as example mouseEvent(..). 

		  // Now we are boring and just show found entity as overlay. 
		  
		  index = i;
		  break;

		}
	    }

	  Qt::MouseButton button = event->button();
	  
	  if ( button == Qt::LeftButton)
	    {
	      // Hide all 
	      
	      for ( int i = 0; i < lstChilds_.size(); ++i)
		{
		  // First hide all widgets which are shown as planes.  
	      
		  Ogre::SceneNode* pNode = pSceneManager_->getSceneNode(lstChilds_[i]->getID().toStdString().c_str());
		  pNode->setVisible(false, true);
		} 
	      
	      
	      // Hide all child widgets -- widgets which are "real" childs of this widget 
	      
	      QObjectList lstChildren = children();
	      for ( int i = 0; i < lstChildren.size(); ++i)
		{
		  QWidget* pChildWidget = qobject_cast<QWidget*>(lstChildren[i]);
		  pChildWidget->hide();
		}
	      
	      // Now show found plane widget as overlay 
	      
	      if ( lstChilds_.size() == 0)
		return;
	      
	      Ogre::OverlayElement* pElement = pContainer_->getChild(lstChilds_[index]->getID().toStdString().c_str());
	      
	      activeChild_ = lstChilds_[index]->getID();

	      // Show widget middle of screen.
	      
	      double w = pElement->getWidth();
	      double h = pElement->getHeight();
	      
	      
	      pElement->setPosition(0.5-w/2.0,0.5-h/2.0);
	      pElement->show();
	      
	      activeMode_ = Overlay;

	      pOverlay_->show();
	    }
	 
	  // Update textures. 
	  
	  update();

	}
      else if ( Qt::RightButton ==  event->button() && activeMode_ == Overlay)
	{
	  // Button was right button, hide current active overlay and show all widgets. 
	  
	  for ( int i = 0; i < lstChilds_.size(); ++i)
	    {
	      Ogre::SceneNode* pNode = pSceneManager_->getSceneNode(lstChilds_[i]->getID().toStdString().c_str());
	      pNode->setVisible(true, true);
	    }
	  
	  
	  QObjectList lstChildren = children();
	  for ( int i = 0; i < lstChildren.size(); ++i)
	    {
	      QWidget* pChildWidget = qobject_cast<QWidget*>(lstChildren[i]);
	      pChildWidget->show();
	    }
	  
	  activeMode_ = Plane;

	  pOverlay_->hide();
	  
	  update();
	}
      else if ( pEntity == 0 && Qt::LeftButton == event->button() && activeMode_ == Overlay)
	{
	  // Now should be that kind state that there exist a overlay which contains widget. Now we want to "pull" out widget from render window, modify it and then push it back
	  
	  int index = -1;
	  for (int i = 0; i < lstChilds_.size(); ++i)
	    {
	      if ( lstChilds_[i]->getID() == activeChild_ )
		{
		  index = i;
		  break;
		}
	    }

	  if ( index != -1 )
	    {
	      lstChilds_[index]->setParent(this);
	      lstChilds_[index]->setWindowFlags(Qt::Window);
	      lstChilds_[index]->show();
	      pOverlay_->hide();
	    }
	

	  activeMode_ = Window;
	  repaint();

	}
      else if (activeMode_ == Window && Qt::LeftButton == event->button())
	{
	  // Push window back. 
	  int index = -1;
	  for (int i = 0; i < lstChilds_.size(); ++i)
	    {
	      if ( lstChilds_[i]->getID() == activeChild_ )
		{
		  index = i;
		  break;
		}
	    }
	  if (index != -1)
	    {
	      lstChilds_[index]->setParent(0);
	      lstChilds_[index]->hide();
	      lstChilds_[index]->updateTexture();
	    }
	   
	  for ( int i = 0; i < lstChilds_.size(); ++i)
	    {
	      Ogre::SceneNode* pNode = pSceneManager_->getSceneNode(lstChilds_[i]->getID().toStdString().c_str());
	      pNode->setVisible(true, true);
	    }
	  
	  
	  QObjectList lstChildren = children();
	  for ( int i = 0; i < lstChildren.size(); ++i)
	    {
	      QWidget* pChildWidget = qobject_cast<QWidget*>(lstChildren[i]);
	      pChildWidget->show();
	    }
	  
	  activeMode_ = Plane;
	  activeChild_ = "";

	  pOverlay_->hide();
	  
	  update();

	}
      else
	{
	  // Show widgets as planes 
	  
	  for ( int i = 0; i < lstChilds_.size(); ++i)
	    lstChilds_[i]->mouseEvent(event);

	}
      

    }
  

  
  qDebug()<<"Ogre3DWidget::mousePressEvent() END";
 

  
}

Ogre::RaySceneQuery* Ogre3DWidget::createRayQuery(const Ogre::Ray& ray)
{
  Ogre::RaySceneQuery* pQuery = pSceneManager_->createRayQuery(ray);
  if ( pQuery != 0)
    pQuery->setSortByDistance(true);
  
  return pQuery;
}



Ogre::Entity* Ogre3DWidget::entityAt(const QPoint& point)
{
  // All above code is plain copy-paste from Ogre wiki. 
    
  double x = point.x()/double(width());
  double y = point.y()/double(height());
  
  Ogre::Ray ray = pCamera_->getCameraToViewportRay(x, y);
  
  // This is must be destroy by our self. 

  Ogre::RaySceneQuery* pQuery = createRayQuery(ray);
 
  // Execute query and verify that there exist objects on ray. 
  
  if (pQuery->execute().size() <= 0)
    {
      pSceneManager_->destroyQuery(pQuery);
      return 0;
    }
  else
    {
      // There exist objects. 
      
      Ogre::Real closest_distance = -1.0f;
      Ogre::Vector3 closest_result;
      Ogre::RaySceneQueryResult &query_result = pQuery->getLastResults();
      Ogre::Entity* pClosestEntity = 0;

      for (int qr_idx = 0; qr_idx < query_result.size(); ++qr_idx)
	{
	  // stop checking if we have found a raycast hit that is closer
	  // than all remaining entities

	  if ((closest_distance >= 0.0f) &&
	      (closest_distance < query_result[qr_idx].distance))
	    {
	      pSceneManager_->destroyQuery(pQuery);
    	      break;
	    }
	  
	  // only check this result if its a hit against an entity

	  if ((query_result[qr_idx].movable != 0) &&
	      (query_result[qr_idx].movable->getMovableType().compare("Entity") == 0))
	    {
	      // get the entity to check
	      Ogre::Entity *pentity = static_cast<Ogre::Entity*>(query_result[qr_idx].movable);           
	      
	      // mesh data to retrieve         
	      size_t vertex_count;
	      size_t index_count;
	      Ogre::Vector3 *vertices;
	      unsigned long *indices;
	      
	      // get the mesh information
	      getMeshInformation(pentity->getMesh(), vertex_count, vertices, index_count, indices,             
				 pentity->getParentNode()->_getDerivedPosition(),
				 pentity->getParentNode()->_getDerivedOrientation(),
				 pentity->getParentNode()->_getDerivedScale());
	      
	      // test for hitting individual triangles on the mesh
	      bool new_closest_found = false;
	      for (size_t i = 0; i < static_cast<int>(index_count); i += 3)
		{
		  // check for a hit against this triangle
		  std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ray, vertices[indices[i]],
									   vertices[indices[i+1]], vertices[indices[i+2]], true, false);
		  
		  // if it was a hit check if its the closest
		  if (hit.first)
		    {
		      if ((closest_distance < 0.0f) ||
			  (hit.second < closest_distance))
			{
			  // this is the closest so far, save it off
			  closest_distance = hit.second;
			  new_closest_found = true;
			}
		    }
		}
	      
	      // free the verticies and indicies memory
	      delete[] vertices;
	      delete[] indices;
	      
	      // if we found a new closest raycast for this object, update the
	      // closest_result before moving on to the next object.
	      if (new_closest_found)
		{
		  pClosestEntity = pentity;
		  closest_result = ray.getPoint(closest_distance);               
		}
	    }       
	}
      
      // return the result
      if (closest_distance >= 0.0f)
	{
	  // raycast success
	  //result = closest_result;	  
	  pSceneManager_->destroyQuery(pQuery);
	  return pClosestEntity;
	}
      else
	{
	  // raycast failed
	  pSceneManager_->destroyQuery(pQuery);
	  return 0;
	} 



    }
  
  pSceneManager_->destroyQuery(pQuery);
  return 0;
}

void Ogre3DWidget::getMeshInformation(const Ogre::MeshPtr mesh,
				      size_t& vertex_count,
				      Ogre::Vector3* &vertices,
				      size_t& index_count,
				      unsigned long* &indices,
				      const Ogre::Vector3& position,
				      const Ogre::Quaternion& orient,
				      const Ogre::Vector3& scale)
{

  bool added_shared = false;
  size_t current_offset = 0;
  size_t shared_offset = 0;
  size_t next_offset = 0;
  size_t index_offset = 0;
  
  vertex_count = index_count = 0;
  
  // Calculate how many vertices and indices we're going to need
  for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
      Ogre::SubMesh* submesh = mesh->getSubMesh( i );
      
      // We only need to add the shared vertices once
      if(submesh->useSharedVertices)
        {
	  if( !added_shared )
            {
	      vertex_count += mesh->sharedVertexData->vertexCount;
	      added_shared = true;
            }
        }
      else
        {
	  vertex_count += submesh->vertexData->vertexCount;
        }
      
      // Add the indices
      index_count += submesh->indexData->indexCount;
    }
  
  
  // Allocate space for the vertices and indices
  vertices = new Ogre::Vector3[vertex_count];
  indices = new unsigned long[index_count];
  
  added_shared = false;
  
  // Run through the submeshes again, adding the data into the arrays
  for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
	
        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

        if((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared))
	  {
            if(submesh->useSharedVertices)
	      {
                added_shared = true;
                shared_offset = current_offset;
	      }
	    
            const Ogre::VertexElement* posElem =
                vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

            Ogre::HardwareVertexBufferSharedPtr vbuf =
	      vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
	    
            unsigned char* vertex =
	      static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
	    
            // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
            //  as second argument. So make it float, to avoid trouble when Ogre::Real will
            //  be comiled/typedefed as double:
            //      Ogre::Real* pReal;
            float* pReal;
	    
            for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
	      {
                posElem->baseVertexPointerToElement(vertex, &pReal);
		
                Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
		
                vertices[current_offset + j] = (orient * (pt * scale)) + position;
	      }
	    
            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }
	
	
        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;
	
        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
	
        unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);
	
	
        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;
	
        if ( use32bitindexes )
	  {
            for ( size_t k = 0; k < numTris*3; ++k)
	      {
                indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
	      }
	  }
        else
	  {
            for ( size_t k = 0; k < numTris*3; ++k)
	      {
                indices[index_offset++] = static_cast<unsigned long>(pShort[k]) +
		  static_cast<unsigned long>(offset);
	      }
	  }
	
        ibuf->unlock();
        current_offset = next_offset;
    }
  
  
}
