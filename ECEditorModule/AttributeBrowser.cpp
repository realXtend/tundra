// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "AttributeBrowser.h"
//#include "AttributeInterface.h"
#include "ComponentInterface.h"
#include "ECComponentEditor.h"
#include "SceneManager.h"

//#include <QtAbstractEditorFactoryBase>
#include <QtTreePropertyBrowser>
#include <QtBrowserItem>
//#include <QtGroupPropertyManager>
#include <QLayout>
#include <QShortcut>
#include <QMenu>

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    AttributeBrowser::AttributeBrowser(QWidget *parent): QtTreePropertyBrowser(parent),
        menu_(0),
        treeWidget_(0)
    {
        InitializeEditor();
    }

    AttributeBrowser::~AttributeBrowser()
    {
    }

    void AttributeBrowser::CreateOrAddNewComponentToEditor(Foundation::ComponentInterfacePtr newComponent)
    {
        std::string typeName = newComponent->TypeName();
        ComponentEditorMap::iterator iter = componentEditors_.find(typeName);

        if(iter == componentEditors_.end())
        {
            componentEditors_[typeName];
            ECComponentEditor *componentEditor = new ECComponentEditor(newComponent, typeName, this);
            componentEditors_[typeName] = componentEditor;
            //QObject::connect(componentEditor, SIGNAL(destroyed(QObject *)), this, SLOT(RemoveComponentEditorFromMap(QObject *)));

            componentsBrowserItemMap_[componentEditors_[typeName]->GetRootProperty()];
            componentsBrowserItemMap_[componentEditors_[typeName]->GetRootProperty()].push_back(Foundation::ComponentWeakPtr(newComponent));
        }
        else
        {
            componentEditors_[typeName]->AddNewComponent(newComponent);
            componentsBrowserItemMap_[componentEditors_[typeName]->GetRootProperty()].push_back(Foundation::ComponentWeakPtr(newComponent));
        }
    }

    void AttributeBrowser::ShowComponentContextMenu(const QPoint &pos)
    {
        QMouseEvent mouseEvent(QEvent::MouseButtonPress, pos, mapToGlobal(pos), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QtTreePropertyBrowser::mousePressEvent(&mouseEvent);

        SAFE_DELETE(menu_);
        menu_ = new QMenu(this);
        menu_->setAttribute(Qt::WA_DeleteOnClose);
        QAction *copyComponent = new QAction(tr("Copy"), menu_);
        QAction *pasteComponent = new QAction(tr("Paste"), menu_);
        QAction *editXml= new QAction(tr("Edit XML..."), menu_);
        QAction *addComponent = new QAction(tr("Add new component"), menu_);
        QAction *deleteComponent= new QAction(tr("Delete component"), menu_);

        //Set shortcuts for actions
        copyComponent->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
        pasteComponent->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
        deleteComponent->setShortcut(QKeySequence::Delete);

        menu_->addAction(copyComponent);
        menu_->addAction(pasteComponent);
        menu_->addAction(editXml);
        menu_->addAction(addComponent);
        menu_->addAction(deleteComponent);
        menu_->popup(mapToGlobal(pos));

        QObject::connect(editXml, SIGNAL(triggered()), this, SLOT(OpenComponentXmlEditor()));
        QObject::connect(addComponent, SIGNAL(triggered()), this, SIGNAL(CreateNewComponent()));
        QObject::connect(deleteComponent, SIGNAL(triggered()), this, SLOT(DeleteComponent()));

        //Check if any component is selected and if not disable delete component, and edit xml options.
        QtBrowserItem *item = currentItem();
        if(!item)
        {
            editXml->setEnabled(false);
            deleteComponent->setEnabled(false);
            copyComponent->setEnabled(false);
        }
        else
        {
            editXml->setEnabled(true);
            deleteComponent->setEnabled(true);
            copyComponent->setEnabled(true);
        }
    }

    void AttributeBrowser::DeleteComponent()
    {
        QtBrowserItem *item = currentItem();
        if(!item)
            return;

        QtProperty *property = item->property();
        PropertyToComponentsMap::iterator iter = componentsBrowserItemMap_.find(property);
        if(iter != componentsBrowserItemMap_.end())
        {
            ComponentWeakPtrVector::iterator componentIter = iter->second.begin();
            while(componentIter != componentsBrowserItemMap_[property].end())
            {
                if(!iter->second.begin()->expired())
                {
                    Foundation::ComponentInterfacePtr component = iter->second.begin()->lock();
                    std::string componentTypeName = component->TypeName();
                    emit DeleteComponent(componentTypeName);
                    break;
                }
                componentIter++;
            }
        }
    }

    void AttributeBrowser::OpenComponentXmlEditor()
    {
        QtBrowserItem *item = currentItem();
        if(!item)
            return;

        QtProperty *property = item->property();
        PropertyToComponentsMap::iterator iter = componentsBrowserItemMap_.find(property);
        if(iter != componentsBrowserItemMap_.end())
        {
            ComponentWeakPtrVector::iterator componentIter = componentsBrowserItemMap_[property].begin();
            while(componentIter != componentsBrowserItemMap_[property].end())
            {
                if(!iter->second.begin()->expired())
                {
                    Foundation::ComponentInterfacePtr component = iter->second.begin()->lock();
                    emit ShowXmlEditorForComponent(component->TypeName());
                    break;
                }
                componentIter++;
            }
        }
    }

    void AttributeBrowser::CopyComponent()
    {
        //TODO! add copy functionality.
    }

    void AttributeBrowser::PasteComponent()
    {
        //TODO! add paste functionality.
    }

    void AttributeBrowser::AddNewComponent(Foundation::ComponentInterfacePtr component)
    {
        assert(component.get());
        CreateOrAddNewComponentToEditor(component);
    }

    void AttributeBrowser::RemoveComponent(Foundation::ComponentInterfacePtr component)
    {
        ComponentEditorMap::iterator iter = componentEditors_.find(component->TypeName());
        if(iter != componentEditors_.end())
        {
            ECComponentEditor *compEditor = iter->second;
            assert(compEditor);
            compEditor->RemoveComponent(component);
            if(!compEditor->NumberOfComponents())
            {
                QtProperty *root = compEditor->GetRootProperty();
                if(root)
                {
                    PropertyToComponentsMap::iterator browIter = componentsBrowserItemMap_.find(root);
                    if(browIter != componentsBrowserItemMap_.end())
                    {
                        componentsBrowserItemMap_.erase(browIter);
                    }
                }
                SAFE_DELETE(compEditor)
                componentEditors_.erase(iter);
            }
        }
    }

    void AttributeBrowser::AddNewEntity(Scene::EntityPtr entity)
    {
        assert(entity.get());
        EntityIDSet::iterator iter = selectedEntities_.find(entity->GetId());
        //No point to add same entity many times into the property browser.
        if(iter != selectedEntities_.end())
            return;
        selectedEntities_.insert(entity->GetId());

        std::vector<Foundation::ComponentInterfacePtr> components = entity->GetComponentVector();
        for(uint i = 0; i < components.size(); i++)
            AddNewComponent(components[i]);

        // Get scene signals to keep the editor in sync when components are added or removed from the ouside.
        assert(entity->GetScene());
        QObject::connect(entity->GetScene(),
                         SIGNAL(ComponentAdded(Scene::Entity*, Foundation::ComponentInterface*, Foundation::ComponentInterface::ChangeType)),
                         this,
                         SLOT(NewEntityComponentAdded(Scene::Entity*, Foundation::ComponentInterface*)));

        QObject::connect(entity->GetScene(),
                         SIGNAL(ComponentRemoved(Scene::Entity*, Foundation::ComponentInterface*, Foundation::ComponentInterface::ChangeType)),
                         this,
                         SLOT(EntityComponentRemoved(Scene::Entity*, Foundation::ComponentInterface*)));
    }

    void AttributeBrowser::RemoveEntity(Scene::Entity *entity)
    {
        if(entity)
        {
            EntityIDSet::iterator iter = selectedEntities_.find(entity->GetId());
            if(iter != selectedEntities_.end())
            {
                std::vector<Foundation::ComponentPtr> components = entity->GetComponentVector();
                for(uint i = 0; i < components.size(); i++)
                {
                    RemoveComponent(components[i]);
                }
                selectedEntities_.erase(iter);
            }
        }
    }

    void AttributeBrowser::AddEntityComponents(std::vector<Foundation::ComponentInterfacePtr> entityComponents)
    {
        if(!entityComponents.size())
            return;

        for(uint i = 0; i < entityComponents.size(); i++)
            AddNewComponent(entityComponents[i]);
    }

    void AttributeBrowser::clear()
    {
        QtTreePropertyBrowser::clear();

        while(!componentsBrowserItemMap_.empty())
            componentsBrowserItemMap_.erase(componentsBrowserItemMap_.begin());
        while(!componentEditors_.empty())
        {
            SAFE_DELETE(componentEditors_.begin()->second)
            componentEditors_.erase(componentEditors_.begin());
        }
        selectedEntities_.clear();

        // QtTreePropertyBrowser clear seems to remove all it's children and so QShortcuts need to reinitialize.
        if(treeWidget_)
        {
            QShortcut* delete_shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), treeWidget_);
            QShortcut* copy_shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), treeWidget_);
            QShortcut* paste_shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), treeWidget_);
            connect(delete_shortcut, SIGNAL(activated()), this, SLOT(DeleteComponent()));
            connect(copy_shortcut, SIGNAL(activated()), this, SLOT(CopyComponent()));
            connect(paste_shortcut, SIGNAL(activated()), this, SLOT(PasteComponent()));
        }
    }

    void AttributeBrowser::NewEntityComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp)
    {
        EntityIDSet::iterator entIter = selectedEntities_.find(entity->GetId());
        // We aren't interested in entities that aren't selected.
        if(entIter == selectedEntities_.end())
            return;

        Foundation::ComponentInterfacePtr componentPtr = entity->GetComponent(comp->TypeName(), comp->Name());
        assert(componentPtr.get());
        CreateOrAddNewComponentToEditor(componentPtr);
    }

    void AttributeBrowser::EntityComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp)
    {
        EntityIDSet::iterator entIter = selectedEntities_.find(entity->GetId());
        // We aren't interested in entities that aren't selected.
        if(entIter == selectedEntities_.end())
            return;

        Foundation::ComponentInterfacePtr componentPtr = entity->GetComponent(comp->TypeName(), comp->Name());
        assert(componentPtr.get());
        RemoveComponent(componentPtr);
    }

    void AttributeBrowser::InitializeEditor()
    {
        QList<QTreeWidget *> treeWidgets = findChildren<QTreeWidget *>();
        for(uint i = 0; i < treeWidgets.size(); i++)
        {
            treeWidget_ = treeWidgets[i];
            treeWidget_->setSelectionMode(QAbstractItemView::ExtendedSelection);
            QShortcut* delete_shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), treeWidget_);
            QShortcut* copy_shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), treeWidget_);
            QShortcut* paste_shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), treeWidget_);
            connect(delete_shortcut, SIGNAL(activated()), this, SLOT(DeleteComponent()));
            connect(copy_shortcut, SIGNAL(activated()), this, SLOT(CopyComponent()));
            connect(paste_shortcut, SIGNAL(activated()), this, SLOT(PasteComponent()));
        }
        setResizeMode(QtTreePropertyBrowser::ResizeToContents);
        setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowComponentContextMenu(const QPoint &)));
    }
}