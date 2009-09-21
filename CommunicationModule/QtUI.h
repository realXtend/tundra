#ifndef incl_QtUI_h
#define incl_QtUI_h

#include "StableHeaders.h"
#include "Foundation.h"

#include <QtGui>

namespace CommunicationUI
{
	class QtUI : public QWidget
	{
	
	Q_OBJECT

	public:
		QtUI(QWidget *parent, Foundation::Framework* framework);
		~QtUI(void);

	private:
		void initWidget();
		void setAllEnabled(bool enabled);

		QLayout *layout_;
		QWidget *widget_;
		QTabWidget *tabWidgetCoversations_;
		QPushButton *buttonSendMessage_;
		QLineEdit *lineEditMessage_;
		QLabel *labelUsername_;
		QLineEdit *lineEditStatus_;
		QComboBox *comboBoxStatus_;
		QLabel *connectionStatus_;

	};

	// CONVERSATION CLASS

	class Conversation : public QWidget
	{
	
	Q_OBJECT

	friend class QtUI;

	public:
		Conversation(QWidget *parent); // Add as inparam also the "conversation object" from mattiku
		~Conversation(void);

	private:
		void initWidget();

		QLayout *layout_;
		QWidget *widget_;
		QPlainTextEdit *textEditChat_;
		
	};

} //end if namespace: CommunicationUI

#endif // incl_QtUI_h