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
		ChatSession(const char* counterpart, Foundation::ScriptObject* imScriptObject);
		~ChatSession(void);
		
		void ChannelOpen();
		void ChannelClosed();

		void ReceivedMessage(char* mess);
		
	private:

		Gtk::Window* wnd_;
		Gtk::TextView* txtChatView_;
		Gtk::Entry* txtEntrySend_;

		Glib::RefPtr<Gtk::TextBuffer> chatBuffer_;

		Glib::RefPtr<Gnome::Glade::Xml> xml_;
		

		void onSendClicked();
		void onCloseClicked();
		

		std::string counterpart_;

		
	public:
		// TODO: Eventually this code would not be here as it is no UI specific, this would be session object from CommunicationModule
		Foundation::ScriptObject* imScriptObject_;

		// instance access
		//static ChatSession* instance_;
	};
}

#endif