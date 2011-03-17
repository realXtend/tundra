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
	//! Dialog for adding new component to entity.
	class ViewDialog: public QDialog
	{
		Q_OBJECT

	public:
		//! Constructs the dialog.
		/*! \param views Names of available views.
			\param parent Parent widget.
			\param f Window flags.
		*/
		ViewDialog(QStringList views, QWidget *parent = 0, Qt::WindowFlags f = 0);

		//! Destroyes the dialog.
		~ViewDialog();

		void UpdateViews(QStringList views);

	private slots:

		void OnRenameButtonClicked();
		void OnSaveButtonClicked();
		void OnDeleteButtonClicked();

	signals:

		void Rename(const QString &oldName,const QString &nnewName);
		void Save(const QString &name);
		void Delete(const QString &name);

	private:

		void save();

		QLineEdit *name_line_edit_;
		//QLineEdit *icon_line_edit_;
		QComboBox *view_combo_box_;
		QPushButton *save_button_;
		QPushButton *cancel_button_;
		QPushButton *delete_button_;
		QPushButton *rename_button_;

		QStringList views_;
	};
}
#endif
