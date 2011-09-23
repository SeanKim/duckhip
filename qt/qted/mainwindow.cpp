#include <QtNetwork>
#include <Qtcore>
#include <QtScript/QScriptEngine>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "define.h"
#include "json.h"
#include "httpdownload.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    QFont f("Arial");
    f.setPointSize(11);

    ui->setupUi(this);
    ui->dockWidget->setWidget(ui->listWidget);
    ui->webView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
    ui->listWidget->setFont(f);

    setCentralWidget(ui->webView);

    createAction();

    httpMode = MODE_GET_TRANSLATOR;
    downloadPath = QDir::currentPath();
    currentItemIndex = -1;
    isTedViewer = false;

    connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotFinished(QNetworkReply*)));

    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotItemClicked(QListWidgetItem*)));

    connect(&redirectChecker,SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
            this,SLOT(slotResponseHeaderReceived(const QHttpResponseHeader &)));

    connect(ui->webView, SIGNAL(loadFinished(bool)), this, SLOT(slotLoadFinished(bool)));

    //UI configuration
    lineEdit = new QLineEdit(ui->statusBar);
    progressBar = new QProgressBar(ui->statusBar);
    progressIndicator = new QProgressIndicator(ui->statusBar);

    lineEdit->setEnabled(false);
    lineEdit->setAlignment(Qt::AlignRight);
    progressIndicator->setAnimationDelay(50);
    progressBar->setStyle(new QPlastiqueStyle);

    ui->statusBar->addPermanentWidget(lineEdit, -1);
    ui->statusBar->addPermanentWidget(progressBar);
    ui->statusBar->addPermanentWidget(progressIndicator);

    init();

    getTranslationList(LANG_CODE);
}

void MainWindow::init()
{
    this->setWindowTitle(APP_TITLE.arg(downloadPath));

    httpMode = MODE_GET_TRANSLATOR;
    currentItemIndex = -1;
    ui->statusBar->showMessage("");
    progressBar->setValue(0);
    progressIndicator->stopAnimation();
    lineEdit->setText("");
    actionDownload->setEnabled(true);
    actionDirectory->setEnabled(true);
    actionCancel->setEnabled(false);

    foreach(Item *item, contents){
        item->downloadEnable = false;
        item->downloadComplete = false;
    }
}

void MainWindow::createAction()
{
    actionDownload = new QAction(QIcon(":/images/download.png"), tr("&TED video download"), this);
    actionDownload->setStatusTip(tr("Download video files"));
    connect(actionDownload, SIGNAL(triggered()), this, SLOT(slotDownload()));

    actionDirectory = new QAction(QIcon(":/images/directory.png"), tr("&Set download directory"), this);
    actionDirectory->setStatusTip(tr("Select download directory"));
    connect(actionDirectory, SIGNAL(triggered()), this, SLOT(slotDirectory()));

    actionWebview = new QAction(QIcon(":/images/ted.png"), tr("&Toggle viewer mode"), this);
    actionWebview->setStatusTip(tr("Enable/Disable Ted web viewer"));
    actionWebview->setCheckable(true);
    connect(actionWebview, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));

    actionCancel = new QAction(QIcon(":/images/cancel.png"), tr("&Cancel..."), this);
    actionCancel->setStatusTip(tr("Cancel..."));
    connect(actionCancel, SIGNAL(triggered()), this, SLOT(slotCancelDownload()));

    actionAbout = new QAction(QIcon(":/images/about.png"), tr("&About..."), this);
    actionAbout->setStatusTip(tr("About..."));
    connect(actionAbout, SIGNAL(triggered()), QApplication::instance(), SLOT(aboutQt()));

    ui->mainToolBar->addAction(actionDownload);
    ui->mainToolBar->addAction(actionDirectory);
    ui->mainToolBar->addAction(actionWebview);
    ui->mainToolBar->addAction(actionCancel);
    ui->mainToolBar->addAction(actionAbout);
}

void MainWindow::getTranslationList(QString lang)
{
    QNetworkRequest request(QUrl("http://" + HOST + URI_TRANSLATION_LIST + lang));
    reply = manager.get(request);

    ui->statusBar->showMessage("Downloading contents list from TED...");

    progressIndicator->startAnimation();
}

void MainWindow::getResourceItem(QString itemUrl)
{
    QNetworkRequest request(QUrl("http://" + HOST + itemUrl));
    reply = manager.get(request);

    ui->statusBar->showMessage("Get talk id and intro information from TED");
}

void MainWindow::getTEDSubtitlesByTalkID(QString takId)
{
    QNetworkRequest request(QUrl("http://" + HOST + URI_TED_SUBTITLE.arg(takId, LANG_CODE)));
    reply = manager.get(request);

    ui->statusBar->showMessage("Get subtitle from TED");
}

void MainWindow::slotFinished(QNetworkReply *reply)
{
    if( isOnlineNetworkConnection() ){

        switch(httpMode){

            //receive JSON stream(TED list of item)
        case MODE_GET_TRANSLATOR:
            {
                QByteArray ba;
                QBuffer buffer(&ba);

                if ( !buffer.open(QIODevice::WriteOnly) ){
                    ui->statusBar->showMessage("translator file read error");
                }

                buffer.write(reply->readAll());
                buffer.close();

                if( parseTranslationJson(ba) ){
                    ui->statusBar->showMessage("Downloaded item list from TED");
                } else {
                    ui->statusBar->showMessage("translator file download error from TED");
                }

                checkDownloadedContent(downloadPath);

                progressIndicator->stopAnimation();
            }
            break;

            //receive item by talk id
        case MODE_GET_TALKID:
            {
                QByteArray ba;
                QBuffer buffer(&ba);

                if ( !buffer.open(QIODevice::WriteOnly) ){
                    ui->statusBar->showMessage("resource file read error");
                }

                buffer.write(reply->readAll());
                buffer.close();

                parseResourceItem(ba);

                httpMode = MODE_GET_SUBTITLE;
                getTEDSubtitlesByTalkID(contents.at(currentItemIndex)->talkId);
            }
            break;

            //receive subtitle JSON stream
        case MODE_GET_SUBTITLE:
            {
                QByteArray ba;
                QBuffer buffer(&ba);

                if ( !buffer.open(QIODevice::WriteOnly) ){
                    ui->statusBar->showMessage(ERROR_SUBTITLE_READ);
                }

                buffer.write(reply->readAll());
                buffer.close();

                QString subtitle = convertTEDSubtitlesToSMISubtitles(contents.at(currentItemIndex)->title, ba, contents.at(currentItemIndex)->introDuration);

                //SMI File Write
                QString filename = contents.at(currentItemIndex)->fileName + ".smi";

                qDebug() << "SMI FILE NAME = " << filename;

                QFile file(this->downloadPath + "/" + filename);
                if (!file.open(QIODevice::WriteOnly)) {
                    qWarning() << ERROR_SMI_WRITE;
                }

                file.write(subtitle.toAscii());
                file.close();

                //check redirect URL
                QHttpRequestHeader header("GET", contents.at(currentItemIndex)->downloadUri);
                header.setValue("Host", HOST_DOWNLOAD);

                redirectChecker.setHost(HOST_DOWNLOAD);
                redirectChecker.request(header);

            }
            break;

        default:
            break;
        }

    } else {
        QMessageBox::warning(this, "Cannot connect to TED", "Please, check your network settings...");
        init();
    }
}

void MainWindow::parseResourceItem(QByteArray byteArray)
{
    QString item(byteArray);

    QString talkId = item.split(";ti=").at(1).split("&").at(0);
    int introDuration = item.split(";introDuration=").at(1).split("&").at(0).toInt();
    QString downloadUri = item.split("\">High-res video").at(0).split("download.ted.com").last();

    contents.value(currentItemIndex)->talkId = talkId;
    contents.value(currentItemIndex)->introDuration = introDuration;
    contents.value(currentItemIndex)->downloadUri = downloadUri;
}

bool MainWindow::parseTranslationJson(QByteArray byteArray)
{
    bool ok;
    QVariantMap json = Json::parse(QString(byteArray), ok).toMap();

    if( !ok ){
        qWarning() << ERROR_JSON_PARSING;
        ui->statusBar->showMessage(ERROR_JSON_PARSING);
        init();
        return false;
    }

    QVariantMap resultSet = json["resultSet"].toMap();

    foreach (QVariant plugin, resultSet["result"].toList()) {
      QVariantMap result = plugin.toMap();

      Item *item = new Item;
      item->downloadEnable = false;
      item->htmlTag = result["markup"].toString();
      item->itemUri = QString(result["markup"].toString().split("href=\"").at(1).split("\"><img").at(0));
      item->title = QString(result["markup"].toString().split("a title=\"").at(1).split("\" href=").at(0)).remove("&quot;");
      item->date = QString(result["markup"].toString().split("date\">").at(1).split("</em>").at(0)).remove("&quot;");
      item->fileName = QString(item->date.replace(" ", "_") + "_" + item->title).remove(QRegExp("[\\/:*?\"<>|!.]"));

      ui->listWidget->addItem(item->fileName);

      contents << item;

    }

    return true;
}

QString MainWindow::convertTEDSubtitlesToSMISubtitles(QString title, QByteArray json, int duration)
{
    bool ok;
    QString smi;
    QVariantMap map = Json::parse(QString(json), ok).toMap();

    if( !ok ){
        qWarning() << ERROR_JSON_PARSING;
    }

    foreach (QVariant caption, map["captions"].toList()) {
      QVariantMap result = caption.toMap();
      QString content = result["content"].toString();
      int startTime = result["startTime"].toInt();
//      QString duration = result["duration"].toString();
//      QString startOfParagraph = result["startOfParagraph"].toString();

      smi+= SMI_LINE.arg( QString::number(duration+startTime), content ) + "\n";
    }

    return SMI_TEMPLATE.arg(title.toUtf8(), smi);
}

void MainWindow::slotDownload()
{

        QModelIndexList entryList = ui->listWidget->selectionModel()->selectedRows();

        if( entryList.length() < 1 ){
            ui->statusBar->showMessage("Please, select video item!!!");
            return;
        }

        actionDownload->setEnabled(false);
        actionDirectory->setEnabled(false);
        actionCancel->setEnabled(true);
        lineEdit->setText("");

        //retrieve download items
        for(int i=0; i<entryList.length(); i++){
            int index = entryList.at(i).row();
            contents.value(index)->downloadEnable = true;

            //set up first index of download item
            if(i == 0){
                currentItemIndex = index;
            }
        }

        count = 0;
        totalCount = entryList.length();

        httpMode = MODE_GET_TALKID;
        getResourceItem(contents.at(currentItemIndex)->itemUri);
}

void MainWindow::slotDirectory()
{
    downloadPath = QFileDialog::getExistingDirectory(this, tr("Change save directory"), QDir::currentPath());

    if (downloadPath.isEmpty()){
        downloadPath = QDir::currentPath();
    }

    this->setWindowTitle(APP_TITLE.arg(downloadPath));
    ui->statusBar->showMessage("Changed video download path");

    checkDownloadedContent(downloadPath);
}

void MainWindow::slotToggled(bool flag)
{
    isTedViewer = flag;

    QModelIndexList entryList = ui->listWidget->selectionModel()->selectedRows();

    if( entryList.length() == 1 ){
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);

        int index = entryList.first().row();
        item->setText(contents.at(index)->fileName);
        item->setToolTip("DELETE_OBJECT");  //FIXME sign for delete object
        slotItemClicked(item);
    }
}

void MainWindow::slotItemClicked(QListWidgetItem *clickedItem)
{
    QModelIndexList entryList = ui->listWidget->selectionModel()->selectedRows();

    if( entryList.length() == 1 ){
        if( isTedViewer ){

            QString filename = clickedItem->text();

            foreach(Item *item, contents){
                if( filename == item->fileName ) {
                    ui->webView->setUrl(QUrl("http://" + HOST + item->itemUri));
                    progressIndicator->startAnimation();
                    break;
                }
            }
        }else{
            ui->webView->setHtml(WEBVIEW_HTML);
        }
    }

    //delete item
    if( clickedItem->toolTip() == "DELETE_OBJECT" ){
        qDebug() << "DELETE_OBJECT";
        delete clickedItem;
    }
}

void MainWindow::slotDownloadFinished(int id, bool isError)
{
    sender()->deleteLater();

    qDebug() << "slotDownloadFinished ID = " << QString::number(id) << endl << "isError = " << isError;

    if( isError ){
        //case : Fail download - item color = RED , download complete flag = TRUE
        ui->listWidget->item(currentItemIndex)->setBackgroundColor(Qt::red);
        contents.value(currentItemIndex)->downloadComplete = true;
    } else {
        contents.at(currentItemIndex)->httpDownloadId = -1;
        contents.value(currentItemIndex)->downloadComplete = true;
        ui->listWidget->item(currentItemIndex)->setBackgroundColor(Qt::yellow);
    }

    //trigger next download item
    int j;
    for(j=0; j<contents.size(); j++){
        if( contents.at(j)->downloadEnable && currentItemIndex < j && !contents.at(j)->downloadComplete){
            currentItemIndex = j;
            httpMode = MODE_GET_TALKID;

            getResourceItem(contents.at(currentItemIndex)->itemUri);
            qDebug() << "currentItemIndex = " << QString::number(currentItemIndex);

            break;
        }
    }

    if( j == contents.size() ){
        qDebug() << "********************** COMPLETE";
        init();
        lineEdit->setText("Download complete");
    }
}

void MainWindow::slotDataReadProgress(int readBytes, int totalBytes)
{
    progressBar->setMaximum(totalBytes);
    progressBar->setValue(readBytes);
}

void MainWindow::slotResponseHeaderReceived(const QHttpResponseHeader &resp)
{
    if( resp.statusCode() == 302) {
        contents.at(currentItemIndex)->videoUrl = resp.value("Location");
    } else if( resp.statusCode() == 200 ) {
        contents.at(currentItemIndex)->videoUrl = "http://" + HOST_DOWNLOAD + ":" + PORT + contents.at(currentItemIndex)->downloadUri;
    }
    qDebug() << "CurrentITEM = " <<QString::number(currentItemIndex) + " VIDEO URL = " << contents.at(currentItemIndex)->videoUrl;

    downloadVideo(currentItemIndex);
}

void MainWindow::slotLoadFinished(bool ok)
{
    progressIndicator->stopAnimation();
}

void MainWindow::slotCancelDownload()
{
    if(httpDownload != NULL){
        httpDownload->cancelDownload();
    }
    init();
    actionCancel->setEnabled(false);
    lineEdit->setText("Download canceled");
}

void MainWindow::downloadVideo(int index)
{
    HttpDownload *http = new HttpDownload(this);
    connect(http, SIGNAL(signalDownloadFinished(int,bool)), this, SLOT(slotDownloadFinished(int,bool)));
    connect(http, SIGNAL(signalDataReadProgress(int,int)), this, SLOT(slotDataReadProgress(int,int)));

    QUrl redirectUrl(contents.at(index)->videoUrl);
    QString uri = redirectUrl.encodedPath();
    QString host = redirectUrl.host();
    QString port = QString::number(redirectUrl.port(80));
    QString filename = contents.at(index)->fileName + ".mp4";

    qWarning() << "HTTP DOWNLOAD URL = " << redirectUrl.toString();

    int id = http->downloadFile(host, port, uri, this->downloadPath, filename);
    contents.value(index)->httpDownloadId = id;

    lineEdit->setText(DOWNLOAD_COUNT.arg(QString::number(++count), QString::number(totalCount)));
    ui->statusBar->showMessage("Get video file from TED");

    httpDownload = http;
}

void MainWindow::checkDownloadedContent(QString dir)
{
    //initialize background color of listWidget
    for(int i=0; i<contents.size(); i++){
            ui->listWidget->item(i)->setBackgroundColor(Qt::transparent);
    }

    QFileInfoList fileList = QDir(dir).entryInfoList(QStringList()<<"*.mp4");

    foreach(QFileInfo info, fileList){
        for(int i=0; i<contents.size(); i++){
            if( contents.at(i)->fileName == info.baseName() && info.size() > 0){
                ui->listWidget->item(i)->setBackgroundColor(Qt::yellow);
                break;
            }
        }
    }
}

bool MainWindow::isOnlineNetworkConnection()
{
//    bool flag = false;
//    QNetworkConfigurationManager mgr;
//    QList<QNetworkConfiguration> activeConfigs = mgr.allConfigurations(QNetworkConfiguration::Active);

//    if (activeConfigs.count() > 0){
//        flag = mgr.isOnline();
//    }

//    return flag;

    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    bool result = false;

    foreach (QNetworkInterface iface, interfaces){
        if ( iface.flags().testFlag(QNetworkInterface::IsUp)
             && !iface.flags().testFlag(QNetworkInterface::IsLoopBack) ) {

            // details of connection
            qDebug() << "name:" << iface.name() << endl
                    << "ip addresses:" << endl
                    << "mac:" << iface.hardwareAddress() << endl;

            // this loop is important
            QList<QNetworkAddressEntry> addressEntries = iface.addressEntries();

            foreach(QNetworkAddressEntry entry, addressEntries){
                qDebug() << entry.ip().toString()
                        << " / " << entry.netmask().toString() << endl;

                // we have an interface that is up, and has an ip address
                // therefore the link is present

                // we will only enable this check on first positive,
                // all later results are incorrect

                // FIXME : Do not support VPN NETWORK DEVICES
                if (result == false)
                    result = true;
            }
        }
    }

    return result;
}

MainWindow::~MainWindow()
{
    delete ui;
}
