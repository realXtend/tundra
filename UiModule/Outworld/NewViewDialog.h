//$ HEADER_NEW_FILE $
#ifndef incl_UiModule_NewViewDialog_h
#define incl_UiModule_NewViewDialog_h

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;

namespace UiServices
{
	class NewViewDialog: public QDialog
	{
		Q_OBJECT

	public:
		/*! Constructs the dialog.
		 * \param views Names of available views.
		 * \param parent Parent widget.
		 * \param f Window flags.
		 */
		NewViewDialog(QStringList views, QWidget *parent = 0, Qt::WindowFlags f = 0);

		/*! Destroyes the dialog.
		 */
		~NewViewDialog();


	private slots:
		/*! Check if the name is a valid name and send the signal
		 */
		void OnSaveButtonClicked();

	signals:

		/*! Signal to save a view
		 * \param name Name of view to save
		 */
		void Save(const QString &name);

	private:

		QLineEdit *name_line_edit_;
		QPushButton *save_button_;
		QPushButton *cancel_button_;
		QStringList views_;
	};
}
#endif
