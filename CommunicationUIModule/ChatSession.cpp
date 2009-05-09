#pragma warning( push )
#pragma warning( disable : 4250 )
#undef max

#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/actiongroup.h>
#include <libglademm.h>
#include <glade/glade.h>


#include <Poco/ClassLibrary.h>

#include "StableHeaders.h"
#include "Foundation.h"
#include <iterator>


#include "ChatSession.h"
#include "CommunicationUIModule.h"

namespace CommunicationUI
{
	//ChatSession* ChatSession::instance_;

	ChatSession::ChatSession(const char* counterpart, Foundation::ScriptObject* imScriptObject)
		: counterpart_(std::string(counterpart)), imScriptObject_(imScriptObject)//: counterpart_(std::string(counterpart)), scriptService_(scriptService), imScriptObject_(imScriptObject)
	{
		CommunicationUI::CommunicationUIModule::LogInfo("ChatSession");
		xml_ = Gnome::Glade::Xml::create("data/chatWnd.glade");
		if (!xml_)
			return;
		this->wnd_ = 0;
		wnd_ = 0;           
		xml_->get_widget("wndChat", wnd_);

		xml_->get_widget("txtChatView", txtChatView_);
		xml_->get_widget("txtEntrySend", txtEntrySend_);
		
		if(!wnd_||!txtChatView_||!txtEntrySend_)
			return;

	    xml_->connect_clicked("btnSend", sigc::mem_fun(*this, &ChatSession::onSendClicked));
		xml_->connect_clicked("btnClose", sigc::mem_fun(*this, &ChatSession::onCloseClicked));
		Glib::ustring title(counterpart);
		
		wnd_->set_title(title);
		
		chatBuffer_ = txtChatView_->get_buffer();
		
		wnd_->show();
        txtEntrySend_->grab_focus();
	}

	ChatSession::~ChatSession(void)
	{
	}

	void ChatSession::onSendClicked()
	{
		Glib::ustring sendTxt = txtEntrySend_->get_text();
        std::cout << "0" << std::endl;
		txtEntrySend_->set_text("");
		
		//// pack message
		//char** args = new char*[3];
  //      //Glib::ustring::size_type = sendTxt.length();
  //      char* addr = new char[counterpart_.length()];
		//char* mess = new char[sendTxt.length()];
  //      
		//strcpy(mess,sendTxt.c_str());
  //      strcpy(addr,counterpart_.c_str());
		//args[0] = addr;
  //      args[1] = mess;

		//std::string str = "CSendChat";
		//std::string syntax = "ss";
  //      //SIZE_T t = counterpart_.length();
  //      Foundation::ScriptObject* ret = imScriptObject_->CallMethod(str, syntax, args);

        std::cout << "1" << std::endl;
        std::string addr_mess(counterpart_);
        addr_mess.append(":");
        addr_mess.append(sendTxt.c_str());
        std::cout << addr_mess << std::endl;
        CommunicationUI::CommunicationUIModule::CallIMPyMethod("CSendChat", "s", addr_mess);
        

		Gtk::TextBuffer::iterator chatIter = chatBuffer_->end();
		chatBuffer_->insert(chatIter, "\nYou> ");
		chatIter = chatBuffer_->end();
		chatBuffer_->insert(chatIter, sendTxt);
        chatBuffer_->insert(chatIter, "\n ..");
        chatIter = chatBuffer_->end();
        txtChatView_->scroll_to(chatIter, 0.0, 1.0, 0.0);
        //txtChatView_->scroll_to_iter(chatIter);
        //delete[] args;
	}

	void ChatSession::onCloseClicked()
	{
		CommunicationUI::CommunicationUIModule::LogInfo("Close clicked");
		std::string str = "CCloseChannel";
		std::string syntax = "";
		Foundation::ScriptObject* ret = imScriptObject_->CallMethod(str, syntax, NULL);
		
	}

	void ChatSession::ChannelOpen()
	{
		Gtk::TextBuffer::iterator chatIter = chatBuffer_->end();
		chatBuffer_->insert(chatIter, "\nText channel opened");
	}
	void ChatSession::ChannelClosed()
	{
		Gtk::TextBuffer::iterator chatIter = chatBuffer_->end();
		chatBuffer_->insert(chatIter, "\nText channel closed");
	}
	
	void ChatSession::ReceivedMessage(char* mess)
	{
		Gtk::TextBuffer::iterator chatIter = chatBuffer_->end();
		chatBuffer_->insert(chatIter, "\n");
		chatIter = chatBuffer_->end();
        chatBuffer_->insert(chatIter, counterpart_.c_str());
        chatIter = chatBuffer_->end();
        chatBuffer_->insert(chatIter, "> ");
        chatIter = chatBuffer_->end();
		chatBuffer_->insert(chatIter, mess);
        chatBuffer_->insert(chatIter, "\n ..");
        chatIter = chatBuffer_->end();
        txtChatView_->scroll_to(chatIter, 0.0, 1.0, 0.5);
        //txtChatView_->scroll_to_iter(chatIter);
	}

}


#pragma warning( pop )