// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "AttributeBrowser.h"
#include "AttributeInterface.h"
#include "ECComponentEditor.h"

#include <QtAbstractEditorFactoryBase>
#include <QtTreePropertyBrowser>
#include <QtGroupPropertyManager>
#include <QLayout>

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    AttributeBrowser::AttributeBrowser(QWidget *parent): QWidget(parent),
        propertyBrowser_(0)
    {
        InitializeEditor();
    }

    AttributeBrowser::~AttributeBrowser()
    {
    }

    void AttributeBrowser::AddEntityComponents(std::vector<Foundation::ComponentInterfacePtr> entityComponents)
    {
        if(!entityComponents.size())
            return;

        for(uint i = 0; i < entityComponents.size(); i++)
        {
            std::string name = entityComponents[i]->TypeName();
            if(entityComponents_.find(name) == entityComponents_.end())
                entityComponents_[name];
            entityComponents_[name].push_back(Foundation::ComponentWeakPtr(entityComponents[i]));
        }
        // Clear previous ui files from the propertyBrowser.
        propertyBrowser_->clear();
        RefreshAttributeComponents();
    }

    void AttributeBrowser::RefreshAttributeComponents()
    {
        EntityComponentMap::iterator iter = entityComponents_.begin();
        while(iter != entityComponents_.end())
        {
            //The final component list that is passed to ECComponentEditor object.
            std::vector<Foundation::ComponentInterfacePtr> components;
            std::vector<Foundation::ComponentWeakPtr>::iterator compIter = iter->second.begin();
            while(compIter != iter->second.end())
            {
                // If the component is expired no point to keep it in the vector.
                if(compIter->expired())
                {
                    compIter = iter->second.erase(compIter);
                    continue;
                }
                components.push_back(Foundation::ComponentInterfacePtr(*compIter));
                compIter++;
            }
            componentEditors_[iter->first] = new ECComponentEditor(components, propertyBrowser_, this);
            QObject::connect(componentEditors_[iter->first], SIGNAL(destroyed(QObject*)), this, SLOT(ComponentEditorDestoryed(QObject *)));
            QObject::connect(componentEditors_[iter->first], SIGNAL(AttributeChanged()), this, SIGNAL(AttributesChanged()));
            iter++;
        }

        //Collapse all property tree's browseritems so that only root items are visible.
        if(propertyBrowser_)
        {
            QList<QtProperty*> properties = propertyBrowser_->properties();
            for(uint i = 0; i < properties.size(); i++)
            {
                QList<QtBrowserItem*> items = propertyBrowser_->items(properties[i]);
                for(uint j = 0; j < items.size(); j++)
                    propertyBrowser_->setExpanded(items[j], false);
            }
        }
    }

    void AttributeBrowser::ClearBrowser()
    {
        propertyBrowser_->clear();
        while(!entityComponents_.empty())
            entityComponents_.erase(entityComponents_.begin());

        while(!componentEditors_.empty())
        {
            SAFE_DELETE(componentEditors_.begin()->second)
            componentEditors_.erase(componentEditors_.begin());
        }
    }

    void AttributeBrowser::ComponentEditorDestoryed(QObject * obj)
    {
        // No point to iterate throught the map if the object is wrong type.
        ECComponentEditor *editor = dynamic_cast<ECComponentEditor*>(obj);
        if(editor)
        {
            ComponentEditorMap::iterator iter = componentEditors_.begin();
            while(iter != componentEditors_.end())
            {
                if(iter->second == editor)
                {
                    componentEditors_.erase(iter);
                    break;
                }
                iter++;
            }
        }
    }

    void AttributeBrowser::InitializeEditor()
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);

        propertyBrowser_ = new QtTreePropertyBrowser(this);
        propertyBrowser_->setResizeMode(QtTreePropertyBrowser::ResizeToContents);
        layout->addWidget(propertyBrowser_);
    }
}