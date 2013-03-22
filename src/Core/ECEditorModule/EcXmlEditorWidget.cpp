/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   EcXmlEditorWidget.cpp
 *  @brief  Widget for entity-component XML editing.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EcXmlEditorWidget.h"
#include "ECEditorModule.h"
#include "ECEditorWindow.h"

#include "Framework.h"
#include "SceneAPI.h"
#include "Scene/Scene.h"
#include "IComponent.h"
#include "Entity.h"
#include "LoggingFunctions.h"
#include "UndoCommands.h"
#include "UndoManager.h"

#include "MemoryLeakCheck.h"

EcXmlEditorWidget::EcXmlEditorWidget(Framework *fw, QWidget *parent) :
    QWidget(parent),
    framework(fw),
    xmlEdit(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setObjectName("mainlayout");
    layout->setContentsMargins(5,5,5,5);
    setLayout(layout);
    setWindowTitle(tr("Entity-Component XML Editor"));
    resize(300, 300);

    xmlEdit = new QTextEdit(this);
    layout->addWidget(xmlEdit);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setObjectName("buttonlayout");

    QSpacerItem *spacer = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hlayout->addSpacerItem(spacer);

    QPushButton *saveButton = new QPushButton(tr("Save"), this);
    connect(saveButton, SIGNAL(pressed()), this, SLOT(Save()));
    hlayout->addWidget(saveButton);

    QPushButton *revertButton = new QPushButton(tr("Revert"), this);
    connect(revertButton, SIGNAL(pressed()), this, SLOT(Revert()));
    hlayout->addWidget(revertButton);

    layout->addLayout(hlayout);
}

EcXmlEditorWidget::~EcXmlEditorWidget()
{
}

void EcXmlEditorWidget::SetEntity(const QList<EntityPtr> &entities)
{
    targetEntities.clear();
    targetComponents.clear();

    foreach(const EntityPtr &ent, entities)
        targetEntities << ent;

    Refresh();
}

void EcXmlEditorWidget::SetComponent(const QList<ComponentPtr> &components)
{
    targetEntities.clear();
    targetComponents.clear();

    foreach(const ComponentPtr &comp, components)
        targetComponents << comp;

    Refresh();
}

void EcXmlEditorWidget::Refresh()
{
    xmlEdit->clear();

    ///\todo Check for expired entities & components and drop them.
//    if (entity_.expired())
//        return;
//    EntityPtr entity = entity_.lock();

    QDomDocument temp_doc;

    // Iterate through individually selected components
    if (targetComponents.size() > 0)
    {
        QListIterator<ComponentWeakPtr> it(targetComponents);

        QDomElement entity_elem = temp_doc.createElement("entity");
        temp_doc.appendChild(entity_elem);
        entity_elem.setAttribute("id", QString::number((int)it.peekNext().lock()->ParentEntity()->Id()));

        while(it.hasNext())
            it.next().lock()->SerializeTo(temp_doc, entity_elem);
    }

    // Insert "entities" tag if we have multiple entities within the same doc, otherwise XML parsing error occurs.
    QDomElement targetEntitieselem = temp_doc.createElement("entities");
    bool multiple = false;
    if (targetEntities.size() > 1)
    {
        multiple = true;
        temp_doc.appendChild(targetEntitieselem);
        targetEntitieselem.setAttribute("count", QString::number(targetEntities.size()));
    }

    // Iterate through individually selected entities.
    QListIterator<EntityWeakPtr> it(targetEntities);
    while(it.hasNext())
    {
        EntityPtr entity = it.next().lock();
        if (!entity)
            continue;

        QDomElement entity_elem = temp_doc.createElement("entity");
        if (multiple)
            targetEntitieselem.appendChild(entity_elem);
        else
            temp_doc.appendChild(entity_elem);
        entity_elem.setAttribute("id", QString::number((int)entity->Id()));

        const Entity::ComponentMap &components = entity->Components();
        for(Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
        {
            i->second->SerializeTo(temp_doc, entity_elem);
            if (multiple)
                targetEntitieselem.appendChild(entity_elem);
            else
                temp_doc.appendChild(entity_elem);
        }
    }
    unsavedState_ = temp_doc;
    xmlEdit->setText(temp_doc.toString());
}

void EcXmlEditorWidget::Revert()
{
    /*
    QMutableListIterator<EntityWeakPtr> it(targetEntities);
    while(cdIt.hasNext())
    foreach(EntityWeakPtr entity, targetEntities)
        if (entity.expired())
            xmlEdit->clear();
*/
    Refresh();
}

void EcXmlEditorWidget::Save()
{
    ///\todo Check for expired entities & components and drop them.
/*
    foreach(EntityWeakPtr entity, targetEntities)
        if (entity.expired())
        {
            xmlEdit->clear();
            Refresh();
            return;
        }
*/

    QString text = xmlEdit->toPlainText();
    if (!text.length())
    {
        LogWarning("EcXmlEditorWidget::Save: Tried to save empty XML data.");
        return;
    }

    Scene *scene = framework->Scene()->MainCameraScene();
    if (!scene)
    {
        LogError("EcXmlEditorWidget::Save: No scene avaiblable.");
        return;
    }

    QString errorMsg;
    QDomDocument edited_doc;
    if (!edited_doc.setContent(text, false, &errorMsg))
    {
        LogWarning("EcXmlEditorWidget::Save: Could not parse XML data: " + errorMsg);
        return;
    }

    // Check if multi-entity or single-entity
    QDomElement targetEntitieselem = edited_doc.firstChildElement("entities");

    // Deserialize all entities/components contained in the data, provided we still find them from the scene
    QDomElement entity_elem;
    if (!targetEntitieselem.isNull())
        entity_elem = targetEntitieselem.firstChildElement("entity");
    else
        entity_elem = edited_doc.firstChildElement("entity");

    bool entity_found = false;
    while(!entity_elem.isNull())
    {
        entity_found = true;
        entity_elem = entity_elem.nextSiblingElement("entity");
    }

    // Refresh immediately after, so that any extra stuff is stripped out, and illegal parameters are (hopefully) straightened
    if (entity_found)
    {
        ECEditorWindow * activeEditor = framework->GetModule<ECEditorModule>()->ActiveEditor();
        if (activeEditor)
            activeEditor->GetUndoManager()->Push(new EditXMLCommand(scene->shared_from_this(), unsavedState_, edited_doc));
        Refresh();
        emit Saved();
    }
    else
        LogWarning("No entity elements in XML data");
}

void EcXmlEditorWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        setWindowTitle(tr("Entity-Component XML Editor"));
    else
       QWidget::changeEvent(event);
}
