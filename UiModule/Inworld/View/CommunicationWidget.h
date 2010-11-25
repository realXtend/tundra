//$ HEADER_MOD_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_CommunicationWidget_h
#define incl_UiModule_CommunicationWidget_h

#include <QGraphicsProxyWidget>
#include "ui_CommunicationWidget.h"
#include "Input.h"

//$ BEGIN_MOD $
#include "TtsChatWidget.h"
#include "EC_TtsVoice.h"
//$ END_MOD $

class QStackedLayout;
class QTextBrowser;
class QGraphicsSceneMouseEvent;

class UiProxyWidget;

namespace Communications
{
    namespace InWorldChat
    {
        class SessionInterface;
        class TextMessageInterface;
    }
	
//$ BEGIN_MOD $
	namespace TtsChat
	{
		class TtsChatWidget;
		class TtsChatConfig;
	}
//$ END_MOD $
}

namespace CommUI
{
    class VoiceToolWidget;
}

namespace Foundation
{
    class Framework;
}

namespace CoreUi
{
    class NormalChatViewWidget;
    class ChatLabel;
    class ToolManagerWidget;

    //! Provide communications functionalities to end user
    //! CommunicationWidget is located to bottom left corner view.
    class CommunicationWidget : public QGraphicsProxyWidget, private Ui::CommunicationWidget
    {
        Q_OBJECT

    public:
        CommunicationWidget(Foundation::Framework* framework);
        enum ViewMode { Normal, History };

    public slots:
        void UpdateImWidget(UiProxyWidget *im_proxy);
        void SetFocusToChat();
        
    protected:
        void hoverMoveEvent(QGraphicsSceneHoverEvent *mouse_hover_move_event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *mouse_hover_leave_event);
        void mousePressEvent(QGraphicsSceneMouseEvent *mouse_press_event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *mouse_move_event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouse_release_event);

    private slots:
        void Initialise();
        void ChangeViewPressed();
        void ChangeView(ViewMode new_mode);
        void ToggleImWidget();

        void ShowIncomingMessage(bool self_sent_message, QString sender, QString timestamp, QString message);
		/// Takes the message from chat line and adds voice information if TTS available
        void SendMessageRequested();
        void InitializeInWorldVoice();
        void InitializeInWorldChat();
        void UninitializeInWorldVoice();
		/// Updates chat view removing voice info first
//$ BEGIN_MOD $
//$ MOD_DESCRIPTION param uuid added $
        void UpdateInWorldChatView(const Communications::InWorldChat::TextMessageInterface &message, const QString& uuid);
//$ END_MOD $

//$ BEGIN_MOD $
		/// Shows or hides TTSCHatWidget configuration window
		void ToggleTtsChatWidget();
		/// Shows TTS Button in the communication widget
		void ShowTtsChatControls();
		/// Hides TTS Button in the communication widget
        void HideTtsChatControls();
	    /// Update TTS Button color to yellow or green changing style sheet
		void UpdateTtsChatControls();
		/// Initializes all TTS system showing buttons, creating widgets and with default configuration
		void InitializeInWorldTts();
		/** Message parser (voice+msg) calls TTS Service
		\param message Message directly from network with time stamp and so on.
		\param from_uuid
		\todo Redefine with components. */
		void SpeakIncomingMessage(const Communications::InWorldChat::TextMessageInterface &message, const QString& from_uuid);
		//void GetAvatarVoiceComponent();
		//void UpdateAvatarVoice(Tts::Voice voice);
//$ END_MOD $


    private:
        Foundation::Framework* framework_;
        ViewMode viewmode_;

        QWidget *internal_widget_;
        QStackedLayout *stacked_layout_;
        QTextBrowser *history_view_text_edit_;
        NormalChatViewWidget *normal_view_widget_;
//$ BEGIN_MOD $
//$ MOD_DESCRIPTION TTS added $
        UiProxyWidget *im_proxy_,*tts_proxy_;
//$ END_MOD $

        Communications::InWorldChat::SessionInterface* in_world_chat_session_;
        CommUI::VoiceToolWidget* voice_tool_;
        ToolManagerWidget* tool_manager_;

        QPointF press_pos_;
        QPointF release_pos_;
        bool resizing_vertical_;
        bool resizing_horizontal_;

        InputContextPtr input_context_;

//$ BEGIN_MOD $
		Communications::TtsChat::TtsChatWidget* Tts_chat_widget;
		Tts::TtsServiceInterface* tts_service_;
		Communications::TtsChat::TtsChatConfig* tts_config_;
		bool ownVoiceOn,othersVoiceOn;
		boost::shared_ptr<EC_TtsVoice> avatar_voice_;
//$ END_MOD $

    signals:
        void SendMessageToServer(const QString &message);
    };

    class NormalChatViewWidget : public QWidget
    {
        Q_OBJECT

    public:
        NormalChatViewWidget(QWidget *parent);

    public slots:
        void ShowChatMessage(bool own_message, QString message);

    private slots:
        void RemoveChatLabel(ChatLabel *label);
    };

    class ChatLabel : public QLabel
    {
        Q_OBJECT

    public:
        ChatLabel(bool own_message, QString message);

    private slots:
        void TimeOut();

    signals:
        void DestroyMe(ChatLabel *);
    };
}

#endif // incl_UiModule_CommunicationWidget_h
