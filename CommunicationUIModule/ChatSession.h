#ifndef incl_ChatSession_h
#define incl_ChatSession_h

#pragma once

#pragma warning( push )
#pragma warning( disable : 4250 )
#include <gtkmm.h>

namespace CommunicationUI
{

	class ChatSession
	{
	public:
		ChatSession(const char* counterpart);
		ChatSession(Communication::IMSessionPtr session, Communication::CommunicationServicePtr communication_service);
		~ChatSession(void);
		
		void ChannelOpen();
		void ChannelClosed();

		void ReceivedMessage(std::string text);
		
	private:

		Gtk::Window* wnd_;
		Gtk::TextView* txtChatView_;
		Gtk::Entry* txtEntrySend_;

		Glib::RefPtr<Gtk::TextBuffer> chatBuffer_;

		Glib::RefPtr<Gnome::Glade::Xml> xml_;
		

		void onSendClicked();
		void onCloseClicked();

		void SetupUI();

		std::string counterpart_;
		Communication::IMSessionPtr session_;
		
	public:
//		Foundation::ScriptObject* imScriptObject_; // TODO: Eventually this code would not be here as it is no UI specific, this would be session object from CommunicationModule
		
		Communication::CommunicationServicePtr communication_service_;

		void OnMessageReceived(Communication::IMMessagePtr m);
		void OnStateChanged();
		void AddMessageToScreen(Communication::IMMessagePtr m);
		Communication::IMSessionPtr GetSession();
	};
}

#endif