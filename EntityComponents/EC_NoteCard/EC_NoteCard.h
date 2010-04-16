// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_NoteCard_EC_NoteCard_h
#define incl_EC_NoteCard_EC_NoteCard_h

#include "ComponentInterface.h"
#include "Declare_EC.h"

class QDomDocument;
class QDomElement;
class QLineEdit;
class QTextEdit;

class EC_NoteCard : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_NoteCard);
    
    Q_OBJECT
    
public:
    //! Destructor.
    ~EC_NoteCard();
    
    virtual bool IsSerializable() const { return true; }
    virtual void SerializeTo(QDomDocument& doc, QDomElement& base_element) const;
    virtual void DeserializeFrom(QDomElement& element);
    
    void Show();
    void Hide();
    
    void SetTitle(const std::string& title);
    void SetText(const std::string& text);
    
    const std::string& GetTitle() { return title_; }
    const std::string& GetText() { return text_; }
    
    bool eventFilter (QObject *obj, QEvent *event);
    
private slots:
    void OnTitleCommitted();
    void OnTextCommitted();
    void OnTitleChanged(const QString& title);
    void OnTextChanged();
    void UpdateWidget();
    void SyncToNetwork();
    
private:
    //! Constuctor.
    /*! \param module Module.
     */
    explicit EC_NoteCard(Foundation::ModuleInterface *module);
    
    std::string title_;
    std::string text_;
    
    QWidget* widget_;
    QLineEdit* title_edit_;
    QTextEdit* text_edit_;
    
    bool title_dirty_;
    bool text_dirty_;
};

#endif
