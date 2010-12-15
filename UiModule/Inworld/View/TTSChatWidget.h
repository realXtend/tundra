//$ HEADER_NEW_FILE $
/********************************************************************
/*********                  TtsChatWidget                ************
/*																    *
/*  This QWidget is used to configure InWorldChat TtsChat translation   *
/*																    *
/*		It's necessary to use Communications:TtsChat namespace.			*										
/*		The class is TtsChatWidget (QWidget)							*
/********************************************************************/


#ifndef TTSCHATWIDGET_H
#define TTSCHATWIDGET_H

#include <QtGui/QMainWindow>
#include "ui_TtsChatWidget.h"
#include "TtsServiceInterface.h"

#include <phonon>


namespace Communications //added
{
	namespace TtsChat //added
	{
		class TtsChatConfig
		{
		public:
			TtsChatConfig();
			virtual const Tts::Voice getOwnVoice();
			virtual void setOwnVoice(Tts::Voice voice);
			// Set the voice of the others
			virtual void setActiveOwnVoice(bool active);
			// return true if the own voce is active
			virtual bool isActiveOwnVoice();
			// Active the others voice
			virtual void setActiveOthersVoice(bool active);
			// return true if the others voce is active
			virtual bool isActiveOthersVoice();	

		private:
			Tts::Voice OwnVoice_;
			bool activeOwnVoice_,activeOthersVoice_;
		};

		class TtsChatWidget : public QWidget
		{
			Q_OBJECT //macro

		public:
			TtsChatWidget(QWidget *parent = 0, Qt::WFlags flags = 0); //constructor
			~TtsChatWidget(); //destructor
			


		public slots:
				//Initializes enable values (ownTtsVoiceStatus,othersTtsVoiceStatus) and voices (ownTtsVoice,othersTtsVoice)
				//Configures and connects SIGNALS with SLOTS
				void ConfigureInterface(Communications::TtsChat::TtsChatConfig* tts_config);
				void reloadItems();
		
		private:

			Ui::TtsChatWidgetClass ui;

			//Variables to drag and move TtsChatWidget QWindow
			QPoint mouse_last_pos_;
			bool mouse_dragging_;
			Communications::TtsChat::TtsChatConfig* tts_config_;
			Phonon::MediaObject *media_object_;
			Phonon::AudioOutput *audio_output_;

			QString fileName;

		 signals:
				void TtsstateChanged();
				void TtsVoiceChanged(Tts::Voice voice);

		protected:
			//Methods to move the window
			virtual void mouseMoveEvent(QMouseEvent *);
			virtual void mousePressEvent(QMouseEvent *);
			virtual void mouseReleaseEvent(QMouseEvent *);

		private slots:
			//This takes the information from the ComboBoxes and saves into the variables
			// After that, stored information can be obtained with the get channels above.
			void saveChanges();
		    void demoButtonPressed();
			void sendTtsStateChanged();

		};
	}//End TtsChat namespace

} //End Communications namespace

namespace Phonon
{
    class MediaObject;
	class AudioOutput;
}

#endif // TtsCHATWIDGET_H

