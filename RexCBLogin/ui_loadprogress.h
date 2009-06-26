/********************************************************************************
** Form generated from reading ui file 'loadprogress.ui'
**
** Created: Fri 26. Jun 10:14:53 2009
**      by: Qt User Interface Compiler version 4.5.1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_LOADPROGRESS_H
#define UI_LOADPROGRESS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoadProgress
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *labelInfo;
    QProgressBar *progressBar;

    void setupUi(QWidget *LoadProgress)
    {
        if (LoadProgress->objectName().isEmpty())
            LoadProgress->setObjectName(QString::fromUtf8("LoadProgress"));
        LoadProgress->resize(400, 63);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(LoadProgress->sizePolicy().hasHeightForWidth());
        LoadProgress->setSizePolicy(sizePolicy);
        LoadProgress->setMinimumSize(QSize(400, 63));
        LoadProgress->setMaximumSize(QSize(400, 63));
        verticalLayout = new QVBoxLayout(LoadProgress);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        labelInfo = new QLabel(LoadProgress);
        labelInfo->setObjectName(QString::fromUtf8("labelInfo"));
        QFont font;
        font.setFamily(QString::fromUtf8("Arial"));
        font.setPointSize(10);
        labelInfo->setFont(font);

        verticalLayout->addWidget(labelInfo);

        progressBar = new QProgressBar(LoadProgress);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setValue(0);
        progressBar->setTextVisible(true);
        progressBar->setOrientation(Qt::Horizontal);
        progressBar->setInvertedAppearance(false);
        progressBar->setTextDirection(QProgressBar::TopToBottom);

        verticalLayout->addWidget(progressBar);


        retranslateUi(LoadProgress);

        QMetaObject::connectSlotsByName(LoadProgress);
    } // setupUi

    void retranslateUi(QWidget *LoadProgress)
    {
        LoadProgress->setWindowTitle(QApplication::translate("LoadProgress", "Load progress", 0, QApplication::UnicodeUTF8));
        labelInfo->setText(QApplication::translate("LoadProgress", "Processing login request...", 0, QApplication::UnicodeUTF8));
        progressBar->setFormat(QApplication::translate("LoadProgress", "%p%", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(LoadProgress);
    } // retranslateUi

};

namespace Ui {
    class LoadProgress: public Ui_LoadProgress {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOADPROGRESS_H
