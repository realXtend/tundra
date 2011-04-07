//$ HEADER_NEW_FILE $
#ifndef incl_UiModule_ViewDialog_h
#define incl_UiModule_ViewDialog_h

#include <QDialog>

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;

namespace UiServices
{
	class ViewDialog: public QDialog
	{
		Q_OBJECT

	public:

		/*! Constructs the dialog.
		 * \param views Names of available views.
		 * \param parent Parent widget.
		 * \param f Window flags.
		 */
		ViewDialog(QStringList views, QWidget *parent = 0, Qt::WindowFlags f = 0);

		/*! Destroyes the dialog.
		 */
		~ViewDialog();

		/*! Update the comboBox with the available views.
		 * \param views Names of available views.
		 */
		void UpdateViews(QStringList views);

	private slots:

		/*! Check if the can be renamed and send the signal
		 */
		void OnRenameButtonClicked();
		/*! Check if the can be saved and send the signal
		 */
		void OnSaveButtonClicked();
		/*! Check if the can be deleted and send the signal
		 */
		void OnDeleteButtonClicked();

	signals:

		/*! Signal to rename a view
		 * \param oldName Name of view to rename
         * \param newName New name of view
		 */
		void Rename(const QString &oldName,const QString &nnewName);
		/*! Signal to save a view
         * \param name Name of view to save
         */
		void Save(const QString &name);
		/*! Signal to delete a view
         * \param name Name of view to delete
         */
		void Delete(const QString &name);

	private:

		QLineEdit *name_line_edit_;
		QComboBox *view_combo_box_;
		QPushButton *save_button_;
		QPushButton *cancel_button_;
		QPushButton *delete_button_;
		QPushButton *rename_button_;

		QStringList views_;
	};
}
#endif
