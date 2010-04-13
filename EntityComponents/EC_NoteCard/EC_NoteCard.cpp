// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_NoteCard.h"
#include "ModuleInterface.h"
#include "Entity.h"
#include "XMLUtilities.h"
#include "RexNetworkUtils.h"
#include "SceneEvents.h"
#include "EventManager.h"
#include "ModuleManager.h"

#include "UiModule.h"
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/InworldSceneController.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QFile>
#include <QLineEdit>
#include <QTextEdit>
#include <QUiLoader>

using namespace RexTypes;
using namespace OgreRenderer;

#define LogError(msg) Poco::Logger::get("EC_NoteCard").error(std::string("Error: ") + msg);
#define LogInfo(msg) Poco::Logger::get("EC_NoteCard").information(msg);

EC_NoteCard::EC_NoteCard(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    widget_(0),
    title_edit_(0),
    text_edit_(0),
    title_dirty_(false),
    text_dirty_(false)
{
    QObject::connect(this, SIGNAL(OnChanged()), this, SLOT(UpdateWidget()));
}

EC_NoteCard::~EC_NoteCard()
{
    if (widget_)
    {
        widget_->deleteLater();
        widget_ = 0;
    }
}

void EC_NoteCard::SerializeTo(QDomDocument& doc, QDomElement& base_element) const
{
    QDomElement comp_element = BeginSerialization(doc, base_element);
    std::string safe_title = title_;
    std::string safe_text = text_;
    
    // This is a severe limitation of xml serialization; we have no choice but to remove harmful substrings
    ReplaceSubstringInplace(safe_title, "<", "");
    ReplaceSubstringInplace(safe_title, ">", "");
    ReplaceSubstringInplace(safe_title, "&lt;", "");
    ReplaceSubstringInplace(safe_title, "&gt;", "");
    ReplaceSubstringInplace(safe_text, "<", "");
    ReplaceSubstringInplace(safe_text, ">", "");
    ReplaceSubstringInplace(safe_text, "&lt;", "");
    ReplaceSubstringInplace(safe_text, "&gt;", "");
    
    WriteAttribute(doc, comp_element, "title", safe_title);
    WriteAttribute(doc, comp_element, "text", safe_text);
}

void EC_NoteCard::DeserializeFrom(QDomElement& element)
{
    // Check that type is right, otherwise do nothing
    if (!BeginDeserialization(element))
        return;
    
    title_ = ReadAttribute(element, "title");
    text_ = ReadAttribute(element, "text");
    
    OnChanged();
}

void EC_NoteCard::SetTitle(const std::string& title)
{
    title_ = title;
    OnChanged();
    SyncToNetwork();
}

void EC_NoteCard::SetText(const std::string& text)
{
    text_ = text;
    OnChanged();
    SyncToNetwork();
}

void EC_NoteCard::OnTitleCommitted()
{
    title_ = title_edit_->text().toStdString();
    SyncToNetwork();
}

void EC_NoteCard::OnTextCommitted()
{
    text_ = text_edit_->toPlainText().toStdString();
    SyncToNetwork();
}

void EC_NoteCard::OnTitleChanged(const QString& title)
{
    title_dirty_ = true;
}

void EC_NoteCard::OnTextChanged()
{
    text_dirty_ = true;
}

void EC_NoteCard::SyncToNetwork()
{
    if (parent_entity_)
    {
        Scene::Events::SceneEventData event_data(parent_entity_->GetId());
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        event_manager->SendEvent(event_manager->QueryEventCategory("Scene"), Scene::Events::EVENT_ENTITY_ECS_MODIFIED, &event_data);
        text_dirty_ = false;
        title_dirty_ = false;
    }
    else
    {
        LogError("No parent entity set, cannot sync to network");
    }
}

void EC_NoteCard::UpdateWidget()
{
    if (!widget_)
    {
        boost::shared_ptr<UiServices::UiModule> ui_module = 
            framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (!ui_module.get())
        {
            LogError("Failed to acquire UiModule pointer");
            return;
        }
        
        QUiLoader loader;
        QFile file("./data/ui/notecard.ui");
        file.open(QFile::ReadOnly);
        widget_ = loader.load(&file);
        file.close();
        
        if (!widget_)
        {
            LogError("Failed to create notecard widget");
            return;
        }
        
        UiServices::UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(widget_,
            UiServices::UiWidgetProperties("Notecard", UiServices::SceneWidget));
        proxy->show();
        
        // Todo: add to notecard manager dialog
        // Todo: manage initial positions sensibly
        
        title_edit_ = widget_->findChild<QLineEdit*>("line_edit");
        if (title_edit_)
        {
            QObject::connect(title_edit_, SIGNAL(editingFinished()), this, SLOT(OnTitleCommitted()));
            QObject::connect(title_edit_, SIGNAL(textChanged(const QString &)), this, SLOT(OnTitleChanged(const QString &)));
        }
        
        text_edit_ = widget_->findChild<QTextEdit*>("text_edit");
        if (text_edit_)
        {
            QObject::connect(text_edit_, SIGNAL(textChanged()), this, SLOT(OnTextChanged()));
            text_edit_->installEventFilter(this);
        }
    }
    
    if (title_edit_)
    {
        QString title_str = QString::fromStdString(title_);
        if (title_edit_->text() != title_str)
            title_edit_->setText(title_str);
    }
    if (text_edit_)
    {
        QString text_str = QString::fromStdString(text_);
        if (text_edit_->toPlainText() != text_str)
            text_edit_->setText(text_str);
    }
}

bool EC_NoteCard::eventFilter(QObject *obj, QEvent *event)
{
    static bool in_event = false;
    if (in_event)
        return false;
    
    // Filter the events of the text editor so that we know when to sync to network
    if (event->type() == QEvent::FocusOut)
    {
        if (text_dirty_)
            OnTextCommitted();
    }
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyevent = checked_static_cast<QKeyEvent*>(event);
        if ((keyevent->key() == Qt::Key_Enter) || (keyevent->key() == Qt::Key_Return))
        {
            // Before committing to network, we want to see how the enter changes the text
            in_event = true;
            QCoreApplication::instance()->notify(obj, event);
            in_event = false;
            OnTextCommitted();
            return true;
        }
    }
    
    return false;
}
