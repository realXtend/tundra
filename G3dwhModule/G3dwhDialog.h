#ifndef G3DWHDIALOG_H
#define G3DWHDIALOG_H

#include <QDialog>
#include <QtGui/QToolBar>
#include <QtWebKit/QtWebKit>
#include <QLabel>
#include <QListWidgetItem>
#include <QPushButton>
#include <QMenu>
#include <QStringList>
#include <QTabBar>
#include <QWebHistory>
#include <QWebHistoryItem>

#include "SceneFwd.h"
#include "AssetReference.h"
#include "IModule.h"


namespace Ui {
    class G3dwhDialog;
}

class G3dwhDialog : public QDialog {
    Q_OBJECT
public:
    G3dwhDialog(Foundation::Framework * framework, std::string modelPath, QWidget *parent = 0);
    ~G3dwhDialog();

    void setScenePath(QString scenePath);
    void disableButtons(bool disabled);

protected:
    void changeEvent(QEvent *e);
    bool eventFilter(QObject *o, QEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    QMenu *settingsMenu;

private:
    Foundation::Framework * framework_;
    Ui::G3dwhDialog *ui;

    QToolBar *toolBar;
    QTabBar *tabBar;

    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *menuButton;
    QPushButton *helpButton;
    QLabel *infoLabel;

    QString modelFileName;
    QString sceneDir;
    QString htmlSource;
    QString modelDir;
    QUrl tabOneUrl;
    QUrl tabTwoUrl;

    bool downloadAborted;
    bool multiSelection;

    QStringList multiSelectionList;

    QAction *testSettingA;
    QAction *testSettingB;
    QAction *testSettingC;

    QList<QWebHistoryItem> tabOneHistory;
    QList<QWebHistoryItem> tabTwoHistory;

    void updateDownloads();
    int unpackDownload(QString file, QString & daeRef);
    void addToScene(QString pathToFile);
    void saveHtmlPath();
    void loadHtmlPath(QString file);
    void checkDirStructure(QString pathToDir, QString & daeRef);
    QString formatFileName(QString toFormat);

private slots:
    void downloadRequested(const QNetworkRequest &);
    void downloadFinished();
    void downloadProgress(qint64, qint64);
    void urlChanged(QUrl);
    void linkHovered(QString,QString,QString);
    void readMetaData();
    void loadFinished();
    void unsupportedContent(QNetworkReply*);

    void on_downloadList_itemClicked(QListWidgetItem *item);
    void addButton_Clicked();
    void removeButton_Clicked();
    void menuButton_Clicked();
    void helpButton_Clicked();

    void settingsMenuAction();
    void currentTabChanged(int index);
    void backActionTriggered();
    void forwardActionTriggered();
};

#endif // G3DWHDIALOG_H
