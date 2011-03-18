//$ HEADER_NEW_FILE $
#ifndef incl_UiModule_NewViewDialog_h
#define incl_UiModule_NewViewDialog_h

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;

namespace UiServices
{
	//! Dialog for adding new component to entity.
	class NewViewDialog: public QDialog
	{
		Q_OBJECT

	public:
		//! Constructs the dialog.
		/*! \param views Names of available views.
			\param parent Parent widget.
			\param f Window flags.
		*/
		NewViewDialog(QStringList views, QWidget *parent = 0, Qt::WindowFlags f = 0);

		//! Destroyes the dialog.
		~NewViewDialog();


	private slots:

		void OnSaveButtonClicked();

	signals:

		void Save(const QString &name);

	private:

		void save();

		QLineEdit *name_line_edit_;
		QPushButton *save_button_;
		QPushButton *cancel_button_;
		QStringList views_;
	};
}
#endif
