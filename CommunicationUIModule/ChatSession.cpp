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

	ChatSession::ChatSession(const char* counterpart): counterpart_(std::string(counterpart))
	{
		SetupUI();
	}

	/**
	 * Find out participants (currently just one)
	 * and set window title along first participant
	 */
	ChatSession::ChatSession(Communication::IMSessionPtr session, Communication::CommunicationServicePtr comm_service)
	{
		communication_service_ = comm_service;
		session_ = session;
		
		Communication::ParticipantPtr originator = session->GetOriginator();
		
		if (originator == session->GetUser())
		{
			// We select the address of the first counter participant to be the window title
			Communication::ParticipantListPtr list = session->GetParticipants();
			for (Communication::ParticipantList::iterator i = list->begin(); i != list->end(); ++i)
			{
				Communication::ParticipantPtr p = *i;
				if (p != session->GetUser())
				{
					counterpart_ = p->GetContact()->GetName();
					break;
				}
			}
		}
		else
		{
			// We select the adderss of orginator of the session to be the windows title
			counterpart_ = session->GetOriginator()->GetContact()->GetName();
		}
		
		SetupUI();
	}

	ChatSession::~ChatSession(void)
	{
	}

	void ChatSession::SetupUI()
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
		Glib::ustring title(counterpart_);
		
		wnd_->set_title(title);
		
		chatBuffer_ = txtChatView_->get_buffer();
		
		wnd_->show();
        txtEntrySend_->grab_focus();
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

        //std::cout << "1" << std::endl;
        //std::string addr_mess(counterpart_);
        //addr_mess.append(":");
        //addr_mess.append(sendTxt.c_str());
        //std::cout << addr_mess << std::endl;

		std::string text = sendTxt.c_str();
		Communication::IMMessagePtr message = communication_service_->CreateIMMessage(text);
		session_->SendIMMessage(message);
		AddMessageToScreen(message);
	}


	void ChatSession::onCloseClicked()
	{
		session_->Close();
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
	
	void ChatSession::ReceivedMessage(std::string text)
	{
		Gtk::TextBuffer::iterator chatIter = chatBuffer_->end();
		chatBuffer_->insert(chatIter, "\n");
		chatIter = chatBuffer_->end();
        chatBuffer_->insert(chatIter, counterpart_.c_str());
        chatIter = chatBuffer_->end();
        chatBuffer_->insert(chatIter, "> ");
        chatIter = chatBuffer_->end();
		chatBuffer_->insert(chatIter, text);
        chatBuffer_->insert(chatIter, "\n ..");
        chatIter = chatBuffer_->end();
        txtChatView_->scroll_to(chatIter, 0.0, 1.0, 0.5);
        //txtChatView_->scroll_to_iter(chatIter);
	}

	void ChatSession::OnMessageReceived(Communication::IMMessagePtr m)
	{
		AddMessageToScreen(m);
	}

	void ChatSession::OnStateChanged()
	{

	}

	/*
	 *  Add given message to chat window message history view
	 *  Format: <time stamp> <author> ">>" <message text>
	 */
	void ChatSession::AddMessageToScreen(Communication::IMMessagePtr m)
	{
		Communication::ParticipantPtr author = m->GetAuthor();
		std::string text;
		text.append(m->GetTimeStamp());
		text.append(" ");
		if (author)
			text.append(m->GetAuthor()->GetContact()->GetName());
		else
			text.append("You");
		text.append(" >> ");
		text.append(m->GetText());

		Gtk::TextBuffer::iterator chatIter = chatBuffer_->end();
		chatBuffer_->insert(chatIter, "\n");
		chatIter = chatBuffer_->end();
		chatBuffer_->insert(chatIter, text);
        chatIter = chatBuffer_->end();
        txtChatView_->scroll_to(chatIter, 0.0, 1.0, 1.0);
		chatIter = chatBuffer_->end();
		chatBuffer_->place_cursor(chatIter);
	}

	/*
	 *  Return chat session object
	 */
	Communication::IMSessionPtr ChatSession::GetSession()
	{
		return session_;
	}
}


#pragma warning( pop )