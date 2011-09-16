#include "G3dwhDialog.h"
#include "ui_G3dwhDialog.h"
#include "G3dwhModule.h"

#include <QDir>
#include <QFile>
#include <QString>
#include <QDirIterator>
#include <QMouseEvent>
#include <QMessageBox>

#include <quazip.h>
#include <quazipfile.h>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QDebug>


#include "SceneStructureModule.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("G3dwh");

/*When model is successfully downloaded with QtWebKit the system unpacks the .zip
  file containing the model unless it is already unpacked. The path to the model
  is saved to daeRef variable. The model is added to scene by using SceneStructureModule,
  which receives the reference to the model from daeRef variable.
  The downloade models are saved to applicationDataDirectory.

*/

G3dwhDialog::G3dwhDialog(Foundation::Framework * framework, std::string modelPath, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::G3dwhDialog)
{
    ui->setupUi(this);

    framework_ = framework;
    modelDir=QString::fromUtf8(modelPath.c_str());

    toolBar = new QToolBar(this);
    toolBar->setObjectName(QString::fromUtf8("toolBar"));

    tabBar = new QTabBar(this);
    tabBar->addTab("Google 3D Warehouse");
    tabBar->addTab("OurBricks");

    tabOneUrl=QUrl("http://sketchup.google.com/3dwarehouse/");
    tabTwoUrl=QUrl("http://ourbricks.com");

    ui->gridLayout->addWidget(toolBar);
    ui->gridLayout->addWidget(tabBar,0,0,0);
    ui->gridLayout->removeWidget(ui->warehouseView);
    ui->gridLayout->addWidget(ui->warehouseView,1,0,0);

    ui->warehouseView->setUrl(QUrl("http://sketchup.google.com/3dwarehouse/"));
    ui->warehouseView->show();

    multiSelection=false;
    multiSelectionList.clear();

    downloadAborted=false;
    ui->downloadProgress->setValue(0);

    updateDownloads();

    ui->warehouseView->page()->setForwardUnsupportedContent(true);

    infoLabel=new QLabel(this);

    QDir downloadDir(modelDir+"/models");
    if(!downloadDir.exists())
    {
        QDir().mkdir(modelDir+"/models");
    }


    addButton = new QPushButton(this);
    addButton->setText("ADD TO SCENE");
    addButton->setToolTip("Add the selected model to scene");
    connect(addButton, SIGNAL(clicked()), this, SLOT(addButton_Clicked()));

    removeButton = new QPushButton(this);
    removeButton->setText("DELETE");
    removeButton->setToolTip("Delete the downloaded model");
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeButton_Clicked()));

    /*
    menuButton = new QPushButton(this);
    menuButton->setText("SETTINGS");
    menuButton->setToolTip("Import Settings");
    connect(menuButton, SIGNAL(clicked()), this, SLOT(menuButton_Clicked()));
    */

    helpButton = new QPushButton(this);
    helpButton->setText("HELP");
    helpButton->setToolTip("Help");
    connect(helpButton, SIGNAL(clicked()), this, SLOT(helpButton_Clicked()));

    settingsMenu = new QMenu();
    testSettingA =  settingsMenu->addAction("TEST SETTING 1");
    testSettingA->setCheckable(true);
    testSettingA->setChecked(false);
    settingsMenu->addSeparator();
    testSettingB =  settingsMenu->addAction("TEST SETTING 2");
    settingsMenu->addSeparator();
    testSettingC =  settingsMenu->addAction("TEST SETTING 3");

    connect(testSettingA, SIGNAL(triggered()), this, SLOT(settingsMenuAction()));

    toolBar->addAction(ui->warehouseView->pageAction(QWebPage::Back));
    toolBar->addAction(ui->warehouseView->pageAction(QWebPage::Forward));
    toolBar->addAction(ui->warehouseView->pageAction(QWebPage::Reload));
    toolBar->addAction(ui->warehouseView->pageAction(QWebPage::Stop));

    toolBar->addWidget(addButton);
    toolBar->addWidget(removeButton);
    //toolBar->addWidget(menuButton);
    toolBar->addWidget(helpButton);
    toolBar->addWidget(infoLabel);


    ui->warehouseView->installEventFilter(this);

    connect(tabBar,SIGNAL(currentChanged(int)),this,SLOT(currentTabChanged(int)));

    connect(ui->warehouseView->page(), SIGNAL(unsupportedContent(QNetworkReply*)), this, SLOT(unsupportedContent(QNetworkReply*)));

    connect(ui->warehouseView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));
    connect(ui->warehouseView->page(), SIGNAL(downloadRequested(const QNetworkRequest&)), this, SLOT(downloadRequested(const QNetworkRequest&)));
    connect(ui->warehouseView, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChanged(QUrl)));
    connect(ui->warehouseView->page(), SIGNAL(linkHovered(QString,QString,QString)), SLOT(linkHovered(QString,QString,QString)));

    connect(ui->warehouseView->pageAction(QWebPage::Back), SIGNAL(triggered()), this, SLOT(backActionTriggered()));
    connect(ui->warehouseView->pageAction(QWebPage::Forward), SIGNAL(triggered()), this, SLOT(forwardActionTriggered()));

}

G3dwhDialog::~G3dwhDialog()
{
    delete ui;
}

//Enables multiselection of models when CTRL is pressed
void G3dwhDialog::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()){
    case Qt::Key_Control:
        if(ui->downloadList->currentItem()!=NULL)
            multiSelectionList.append("models/"+ui->downloadList->currentItem()->text());

        modelFileName.clear();
        ui->downloadList->setSelectionMode(QAbstractItemView::MultiSelection);
        multiSelection=true;
        break;
    case Qt::Key_Delete:
        removeButton_Clicked();
        break;
    }

}

//Disables multiselection of models when CTRL is released and clears the list of selected models
void G3dwhDialog::keyReleaseEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Control)
    {
        //multiSelectionList.clear();
        //ui->downloadList->clearSelection();
        //ui->downloadList->setCurrentItem(NULL);
        ui->downloadList->setSelectionMode(QAbstractItemView::SingleSelection);
        multiSelection=false;
    }
}

void G3dwhDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

bool G3dwhDialog::eventFilter(QObject *o, QEvent *e)
{
    if(e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
        if (mouseEvent->button() == Qt::XButton1)
        {
            ui->warehouseView->pageAction(QWebPage::Back)->trigger();

            mouseEvent->accept();
            return true;
        }

        if (mouseEvent->button() == Qt::XButton2)
        {
            ui->warehouseView->pageAction(QWebPage::Forward)->trigger();

            mouseEvent->accept();
            return true;
        }

    }
    return false;
}


//If multiselection is enabled, go trough the list and add the models to scene
//If not, add the currently selected model
void G3dwhDialog::addButton_Clicked()
{
    if(ui->downloadList->count()>0)
    {
        //if(multiSelection)
        //{
            if(!multiSelectionList.empty())
            {
                for(int i=0;i<multiSelectionList.length();i++)
                {
                    QString addFileName=multiSelectionList.at(i);
                    addToScene(addFileName);
                }
                return;
            }
        //}
        else if(ui->downloadList->currentItem()!=NULL)
        {
            QString addFileName="models/"+ui->downloadList->currentItem()->text();
            addToScene(addFileName);
            return;
        }
        else
            LogInfo("No item selected");
    }
    else
        LogInfo("No models downloaded");
}

//Add the model file specified if pathToFile to scene
void G3dwhDialog::addToScene(QString pathToFile)
{
    QString daeRef;
    //Unpack the downloaded zip containing the model and textures
    unpackDownload(pathToFile, daeRef);
    //Check that the directory structure of unpacked model is like we want
    checkDirStructure(pathToFile, daeRef);

    //Add the model to the scene
    bool clearScene = false;
    SceneStructureModule *sceneStruct = framework_->GetModule<SceneStructureModule>();

    if (sceneStruct)
        //Send path to dae file (daeRef) to SceneStructureModule and add the model to the scene
        sceneStruct->InstantiateContent(daeRef, Vector3df(), clearScene);
    else
        LogInfo("No SceneStructureModule found");
}

void G3dwhDialog::backActionTriggered()
{
    //Check which tab is open and move in history based on that information.
    if(ui->warehouseView->page()->history()->currentItemIndex()!=0)
    {
        if(tabBar->currentIndex()==0)
        {
            if(!ui->warehouseView->page()->history()->currentItem().url().toString().contains("google"))
            {
                ui->warehouseView->pageAction(QWebPage::Back)->trigger();
            }
        }else if(tabBar->currentIndex()==1)
        {
            if(!ui->warehouseView->page()->history()->currentItem().url().toString().contains("ourbricks"))
            {
                ui->warehouseView->pageAction(QWebPage::Back)->trigger();
            }
        }
    }
}

//Check directory structure and repair if needed
//The required structure is modelname/models/model.dae modelname/images/texture.imageformat
void G3dwhDialog::checkDirStructure(QString pathToDir,QString &daeRef)
{
    QString targetDirName = sceneDir+pathToDir.replace(".zip","/");

    QDir imagesDir(targetDirName+"/images");
    QDir modelsDir(targetDirName+"/models");
    if(imagesDir.exists() && modelsDir.exists())
        return;
    else
    {
        QDir().mkpath(targetDirName+"/images");
        QDir().mkpath(targetDirName+"/models");

        QString dirContent;
        QStringList filter;
        filter<<"*.dae"<<"*.jpg"<<"*.png"<<"*.gif"<<"*.jpeg"<<"*.JPG"<<"*.DAE"<<"*.PNG"<<"*.GIF"<<"*.JPEG";
        QDirIterator dirIterator(targetDirName,filter,QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot,QDirIterator::Subdirectories);

        while(dirIterator.hasNext())
        {
            dirIterator.next();
            dirContent=dirIterator.filePath();

            if(dirContent.contains(".dae"))
            {
                QFile daeFile(dirContent);
                daeFile.rename(targetDirName+"models/"+dirIterator.fileName());
                daeRef=daeFile.fileName();
            }

            for(int i=1;i<filter.length();i++)
            {
                QString imageFilter=filter[i];
                imageFilter.replace("*","");
                if(dirContent.contains(imageFilter))
                {
                    QFile imageFile(dirContent);
                    imageFile.rename(targetDirName+"images/"+dirIterator.fileName());
                }
            }
        }
    }
}

void G3dwhDialog::currentTabChanged(int index)
{
    switch (index){
    case 0:
        if(ui->warehouseView->url()!=tabOneUrl)
        tabTwoUrl=ui->warehouseView->url();
        ui->warehouseView->load(tabOneUrl);
        break;
    case 1:
        if(ui->warehouseView->url()!=tabTwoUrl)
        tabOneUrl=ui->warehouseView->url();
        ui->warehouseView->load(tabTwoUrl);
        break;
    }
}

//Disaples buttons, called from G3dwhModule when no scene storage is available
void G3dwhDialog::disableButtons(bool disabled)
{
    addButton->setDisabled(disabled);
}

//Called when download sends finished signal.
//If download wasn't aborted write the downloaded data to file
void G3dwhDialog::downloadFinished()
{
    ui->downloadProgress->setValue(0);
    QNetworkReply *reply = ((QNetworkReply*)sender());

    if( downloadAborted == false )
    {
        QFile fileTest(modelFileName);
        int index=0;

        //Rename if file with same name already exists
        while(fileTest.exists())
        {
            QString indexStr;
            indexStr.setNum(index);
            QString modelName = fileTest.fileName();
            modelName.replace(".zip","");
            modelName.replace(QRegExp("_[0-9]{1,9}$"),"");

            fileTest.setFileName(modelName+"_"+indexStr+".zip");
            modelFileName=fileTest.fileName();
            index++;
        }

        QFile file(modelFileName);
        if (file.open(QFile::ReadWrite))
            file.write(reply->readAll());
        file.close();
    }

    downloadAborted=false;
    //Save url of origin to sources and update the list of downloaded models
    saveHtmlPath();
    updateDownloads();

}

//Update progress bar
void G3dwhDialog::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    ui->downloadProgress->setMaximum(bytesTotal);
    ui->downloadProgress->setValue(bytesReceived);
}

//Handle download request from webview
void G3dwhDialog::downloadRequested(const QNetworkRequest &request)
{
    QNetworkRequest newRequest = request;
    newRequest.setAttribute(QNetworkRequest::User,"tmpmodelFileName");
    QNetworkAccessManager *networkManager = ui->warehouseView->page()->networkAccessManager();
    QNetworkReply *reply = networkManager->get(newRequest);

    connect(reply, SIGNAL(metaDataChanged()),this, SLOT(readMetaData()));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),this, SLOT(downloadProgress(qint64, qint64)));
    connect(reply, SIGNAL(finished()),this, SLOT(downloadFinished()));

}

//Replace some special characters that won't work in filenames.
QString G3dwhDialog::formatFileName(QString toFormat)
{
    QString formattedString = toFormat;
    //!"#$%&'()*+,:;<=>?@* to _
    formattedString.replace(QRegExp("[\x0021-\x002B\x003A-\x0040\x002F\x00B4]"),"_");
    //àáâãäåæÀÁÂÃÄÅÆ to a
    formattedString.replace(QRegExp("[\x00E0-\x00E6\x00C0-\x00C6]"),"a");
    //èéêëÈÉÊË to e
    formattedString.replace(QRegExp("[\x00E8-\x00EB\x00C8-\x00CB]"),"e");
    //ÌÍÎÏìíîï to i
    formattedString.replace(QRegExp("[\x00EC-\x00EF\x00CC-\x00CF]"),"i");
    //òóôõöøÒÓÔÕÖØ to o
    formattedString.replace(QRegExp("[\x00D2-\x00D6\x00F2-\x00F6\x00D8\x00F8\x009C]"),"o");
    //ùúûüÙÚÛÜ to u
    formattedString.replace(QRegExp("[\x00D9-\x00DC\x00F9-\x00FC]"),"u");
    //ýÝ to y
    formattedString.replace(QRegExp("[\x00DD\x00FD]"),"y");
    //formattedString.replace(QString::fromWCharArray(L"\u123B"),"s");
    QVector<uint> ucsVector = formattedString.toUcs4();
    QList<uint> replaceableChars;
    //282-283 replace with e
    //209,241,323-324 replace with n
    //321-322 replace with l
    //346-353 replace with s
    //374-375 replace with y
    //377-382 replace with z
    replaceableChars<<209<<241<<282<<283<<321<<322<<323<<324<<346<<347<<352<<353<<374<<375<<377 \
                   <<378<<379<<380<<381<<382;

    for(int i=0;i<replaceableChars.count();i++)
    {
        for(int ii=0;ii<ucsVector.count();ii++)
        {
            if(ucsVector.value(ii)==replaceableChars.value(i))
            {
                unsigned int tmpValue = replaceableChars.value(i);
                if(tmpValue==282 || tmpValue==283)
                {
                    ucsVector[ii]=101;
                }
                else if(tmpValue==323 || tmpValue==324 || tmpValue==209 || tmpValue==241)
                {
                    ucsVector[ii]=110;
                }
                else if(tmpValue==321 || tmpValue==322)
                {
                    ucsVector[ii]=76;
                }
                else if(tmpValue>=346 && tmpValue<=353)
                {
                    ucsVector[ii]=115;
                }
                else if(tmpValue==374 || tmpValue==375)
                {
                    ucsVector[ii]=121;
                }
                else if(tmpValue>=377 || tmpValue<=382)
                {
                    ucsVector[ii]=122;
                }
            }
        }
    }
    formattedString.clear();
    formattedString=QString::fromUcs4(ucsVector.data(),ucsVector.count());
    ucsVector.clear();
    return formattedString;
}

void G3dwhDialog::forwardActionTriggered()
{
    //Check which tab is open and move in history based on that information.
    if(ui->warehouseView->page()->history()->currentItemIndex()<ui->warehouseView->page()->history()->count()+1)
    {
        if(tabBar->currentIndex()==0)
        {
            if(!ui->warehouseView->page()->history()->currentItem().url().toString().contains("google"))
            {
                ui->warehouseView->pageAction(QWebPage::Forward)->trigger();
            }
        }else if(tabBar->currentIndex()==1)
        {
            if(!ui->warehouseView->page()->history()->currentItem().url().toString().contains("ourbricks"))
            {
                ui->warehouseView->pageAction(QWebPage::Forward)->trigger();
            }
        }
    }
}

void G3dwhDialog::helpButton_Clicked()
{
    QMessageBox *helpBox = new QMessageBox();
    helpBox->setWindowTitle("Help");
    helpBox->setText("How to use:\n" \
                     "Downloading models: Once you find a model you would like to download " \
                     "click \"Download Model\" and select Collada if available. All models " \
                     "aren't available for download or not in Collada format. The downloaded " \
                     "models are listed on the right. By clicking a file in the list you are " \
                     "directed to the page of the model for details about it.\n\n" \
                     "Adding models to the scene: Select the model you want to add from the " \
                     "list and click \"ADD TO SCENE\". You can select multiple models by " \
                     "by holding down the Ctrl-key.\n\n"\
                     "Deleting models: You can delete the downloaded model from your computer " \
                     "by selecting it from the list and clicking \"DELETE\". You can select "\
                     "multiple models by holding down the Ctrl-key.\n\n" \
                     "Use the keys on toolbar to move to next or previous page, refresh the " \
                     "page or to stop loading the page. If your mouse has buttons for next/previous " \
                     "page they should work too.");
    helpBox->setStandardButtons(QMessageBox::Ok);
    helpBox->exec();
}

//Mouse hovers over link
void G3dwhDialog::linkHovered(QString url, QString title, QString content)
{
    //qDebug()<<url;
}

//Finished loading web page
void G3dwhDialog::loadFinished()
{
    infoLabel->setText("");
}

//Load url reference for file
void G3dwhDialog::loadHtmlPath(QString file)
{
    QFile htmlSources(modelDir+"/models/sources");
    if (!htmlSources.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    while (!htmlSources.atEnd())
    {
        QByteArray fileInput = htmlSources.readLine();
        QString line = fileInput;
        if (line.contains(file,Qt::CaseSensitive))
        {
            QStringList parseList=line.split("|");
            QString newUrl=parseList[0];

            if(newUrl.contains("google"))
                tabBar->setCurrentIndex(0);
            else if(newUrl.contains("ourbricks"))
                tabBar->setCurrentIndex(1);

            ui->warehouseView->load(QUrl(newUrl));

        }
    }

}

//Display popup menu for settings
void G3dwhDialog::menuButton_Clicked()
{
    QPoint displayPoint = QCursor::pos();
    settingsMenu->popup(displayPoint,0);
}

//Add selected model to list if multiselection is enabled. If not, display the original
//download page for the model
void G3dwhDialog::on_downloadList_itemClicked(QListWidgetItem *item)
{
    if(multiSelection)
    {
        if(!multiSelectionList.contains("models/"+item->text()))
        {
            multiSelectionList.append("models/"+item->text());
            loadHtmlPath(modelDir+"/"+multiSelectionList.last());
        }
        else
        {
            multiSelectionList.removeAt(multiSelectionList.indexOf("models/"+item->text()));
        }
    }
    else
    {
        multiSelectionList.clear();
        QString modelFileName=modelDir+"/models/"+item->text();
        loadHtmlPath(modelFileName);
    }
}

//Called when metadata of network request changes. The complete metadata contains
//information needed to determine the type of data being downloaded.
//If datatype is not application/zip abort the download.
//Compose the modelFileName from the title of the models page.
void G3dwhDialog::readMetaData()
{
    QNetworkReply *reply = ((QNetworkReply*)sender());
    QString dataType=reply->header(QNetworkRequest::ContentTypeHeader).toString();
    if (dataType != "application/zip" && dataType !="application/octet-stream")
    {
        downloadAborted=true;
        reply->close();
        infoLabel->setText("Wrong format, select Collada if available.");
    }
    else
    {
        QStringList titleList = ui->warehouseView->title().split(" by");
        QStringList parseList = titleList[0].split(" ");
        modelFileName.clear();
        modelFileName=modelDir+"/models/"+formatFileName(parseList.join("_")+".zip");
        modelFileName.replace(",","");
        htmlSource = ui->warehouseView->url().toString();

    }
}

//If multiselection is enabled, delete selected models and remove from model list
//If not, delete the selected model and remove from model list
//Remove the data of the deleted model from sources file
void G3dwhDialog::removeButton_Clicked()
{

    if (ui->downloadList->currentItem() == NULL && multiSelectionList.isEmpty())
    {
        LogInfo("No item selected");
        return;
    }

    QStringList removeList;
    if(!multiSelectionList.empty())
        removeList=multiSelectionList;
    else
        removeList.append("models/"+ui->downloadList->currentItem()->text());

    for(int i=0;i<removeList.length();i++)
    {
        QString removeFileName=modelDir+"/"+removeList.at(i);
        QFile file(removeFileName);
        file.remove();
        updateDownloads();

        QFile htmlSources(modelDir+"/models/sources");
        if (!htmlSources.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

        QStringList fileContent;
        QStringList newFileContent;

        while (!htmlSources.atEnd())
        {
            QByteArray fileInput = htmlSources.readLine();
            QString line=fileInput;
            fileContent.append(line);
        }
        htmlSources.close();
        htmlSources.remove();

        QString checkFile;

        for(int i=0;i<fileContent.count();i++)
        {
            checkFile=fileContent.at(i).split("|").last().remove(QRegExp("\n"));
            if(QFile::exists(checkFile))
                newFileContent.append(fileContent.at(i));
        }

        if (!htmlSources.open(QIODevice::Append | QIODevice::Text))
            return;

        QTextStream out(&htmlSources);
        for(int i=0; i<newFileContent.length(); i++)
        {
            out << newFileContent.at(i);
        }
        htmlSources.close();
    }

}

//Save url reference of downloaded model to sources file
void G3dwhDialog::saveHtmlPath()
{
    QFile htmlSources(modelDir+"/models/sources");
    if (!htmlSources.open(QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&htmlSources);
    out << htmlSource+"|"+modelFileName.toAscii()+"\n";
    htmlSources.close();
}

//Actions for settings popup menu
void G3dwhDialog::settingsMenuAction()
{
    if (testSettingA->isChecked())
        LogInfo("Option1 set");
    else
        LogInfo("Option1 unset");
}

//Set the path to directory to the currently open scene
void G3dwhDialog::setScenePath(QString scenePath)
{
    sceneDir = scenePath;
}

//Unpack the zip archive containing the model
int G3dwhDialog::unpackDownload(QString file, QString & daeRef)
{
    QFile inFile(modelDir+"/"+file);
    QFile outFile;

    QString targetName = file.replace(".zip","/");

    //Check if the file was already unpacked
    QDir testDir(sceneDir+targetName);
    QStringList filter;
    filter<<"*.dae";
    if(testDir.exists())
    {
        QDirIterator dirIterator(testDir.absolutePath(),filter,QDir::AllDirs|QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot,QDirIterator::Subdirectories);

        while(dirIterator.hasNext())
        {
            dirIterator.next();

            if(dirIterator.fileName().endsWith(".dae"))
                daeRef=dirIterator.filePath();
        }
        return true;
    }
    else
    {
        QDir().mkpath(sceneDir+targetName);
    }

    //Use QuaZip to extract the files

    QuaZip zip(inFile.fileName());

    if(!zip.open(QuaZip::mdUnzip)) {
        qWarning("testRead(): zip.open(): %d", zip.getZipError());
        return false;
    }

    QuaZipFileInfo info;
    QuaZipFile zFile(&zip);
    QString name;
    char c;

    //Some test code to check to contents of zip archive
    QStringList archiveContent;
    for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile())
    {
        archiveContent.append(zFile.getActualFileName());
    }

    for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile())
    {
        if(!zip.getCurrentFileInfo(&info)) {
        qWarning("testRead(): getCurrentFileInfo(): %d\n", zip.getZipError());
        return false;
        }
        if(!zFile.open(QIODevice::ReadOnly)) {
            qWarning("testRead(): file.open(): %d", zFile.getZipError());
            return false;
        }
        name=zFile.getActualFileName();
        if(zFile.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.getmodelFileName(): %d", zFile.getZipError());
            return false;
        }

        if (name.endsWith(".dae"))
        {
            name.replace(" ","");
            //daeRef contains the path to the model file
            daeRef = targetName + name;
        }
        QString dirn = sceneDir + targetName + name;

        if (name.contains('/')) {
            dirn.chop(dirn.length() - dirn.lastIndexOf("/"));
            QDir().mkpath(dirn);
        }
        outFile.setFileName( sceneDir + targetName + name);

        //Write extracted data to file
        outFile.open(QIODevice::WriteOnly);
        char buf[4096];
        int len = 0;
        while (zFile.getChar(&c))
        {
            // we could just do this, but it's about 40% slower:
            // out.putChar(c);
            buf[len++] = c;
            if (len >= 4096)
            {
                outFile.write(buf, len);
                len = 0;
            }
        }
        if (len > 0)
        {
            outFile.write(buf, len);
        }
        outFile.close();


        if(zFile.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.getmodelFileName(): %d", zFile.getZipError());
            return false;
        }
        if(!zFile.atEnd()) {
            qWarning("testRead(): read all but not EOF");
            return false;
        }
        zFile.close();
        if(zFile.getZipError()!=UNZ_OK) {
            qWarning("testRead(): file.close(): %d", zFile.getZipError());
            return false;
        }
    }
    zip.close();
    if(zip.getZipError()!=UNZ_OK) {
        qWarning("testRead(): zip.close(): %d", zip.getZipError());
        return false;
    }

    return true;

}

//Update list of downloaded models
void G3dwhDialog::updateDownloads()
{
    ui->downloadList->clear();

    QDir dir(modelDir+"/models");
    QStringList nameFilters;
    nameFilters.append("*.zip");
    dir.setNameFilters(nameFilters);
    QStringList downloadedItems=dir.entryList();

    ui->downloadList->addItems(downloadedItems);
}

//Handle download request when download link is clicked.
void G3dwhDialog::unsupportedContent(QNetworkReply *reply)
{
    QNetworkRequest request(reply->url());
    downloadRequested(request);
}

//Called when url changes, prevents moving away from 3d Warehouse
void G3dwhDialog::urlChanged(QUrl url)
{
    QString newUrl=url.toString();
    if(!newUrl.contains("sketchup.google.com/3dwarehouse") && !newUrl.contains("ourbricks.com"))
    {
        if(tabBar->currentIndex()==0)
        {
            ui->warehouseView->load(QUrl("http://sketchup.google.com/3dwarehouse/"));
        }

        if(tabBar->currentIndex()==1)
        {
            ui->warehouseView->load(QUrl("http://ourbricks.com"));
        }
    }
}

