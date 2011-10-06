#include "httpdownload.h"

#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QString>
#include <QHttp>
#include <QString>
#include <QHttpResponseHeader>
#include <QHttpRequestHeader>
#include <QStringList>
#include <QUrl>

HttpDownload::HttpDownload(QObject * parent): QObject(parent){
    _downSuccess = false;
    http = new QHttp(this);
    connect(http,SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
                                     this,SLOT(slotResponseHeaderReceived(const QHttpResponseHeader &)));
    connect(http,SIGNAL(dataReadProgress(int,int)),
                                     this,SLOT(slotDataReadProgress(int, int)));
    connect(http,SIGNAL(requestFinished(int,bool)),
                                     this,SLOT(slotRequestFinished(int, bool)));
}

int HttpDownload::downloadFile(QString host, QString port, QString targetUrl, QString targetLocation, QString fileName){
    qDebug() << "Download host : " << host;
    qDebug() << "Download port : " << port;
    qDebug() << "Download targetUrl : " << targetUrl;
    _downSuccess = false;
    _conFinish = false;
    targetDir = targetLocation;
    this->fileName = fileName;
    QHttpRequestHeader header("GET", targetUrl);
    header.setValue("Host", host);

    if(!port.isEmpty()){
    	http->setHost(host,port.toInt());
    }
    else{
    	http->setHost(host);
    }

    requestId = http->request(header);

    return requestId;
}

void HttpDownload::writeFile(int id, bool error){
    QString filePath = targetDir+"/"+fileName;
    if(QFile::exists(filePath)){
            QFile::remove(filePath);
    }

    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write(http->readAll());
    file.close();
    emit signalDownloadFinished( id, error);
}

void HttpDownload::cancelDownload(){
    http->abort();
    _conFinish = true;
    _downSuccess = false;
}

bool HttpDownload::conFinish(){
    return _conFinish;
}

void HttpDownload::slotResponseHeaderReceived( const QHttpResponseHeader & resp ){

//    //redirect
//    QString location = resp.value("Location");
//    if( !location.isEmpty() && resp.statusCode() == 302) {
//        _redirectFlag = true;
//        QUrl redirectUrl(location);
//        QHttpRequestHeader header("GET", redirectUrl.encodedPath());
//        header.setValue("Host", redirectUrl.host());
//        http->setHost(redirectUrl.host(), redirectUrl.port(80));
//        http->request(header);
//    }

    QString contentLength = resp.value("Content-Length");
    if( !contentLength.isEmpty() ){
        long length = contentLength.toLong();
//        emit signalResponseContentLength(length);
    }

    //user define!!
    if(!this->fileName.isEmpty())
            return;

    QString disposition = resp.value("Content-Disposition");

    disposition.replace(" " ,"",Qt::CaseInsensitive);
    QStringList list = disposition.split(";");

    QString fileName = "";

    QRegExp exp = QRegExp("[ ]*[filename][ ]*=[ ]*.*");
    exp.setCaseSensitivity(Qt::CaseInsensitive);

    for(int i = 0 ; i < list.size() ; i ++){
            QString tempStr = list.value(i);
            if( tempStr.contains(exp)) {
                    exp = QRegExp("[ ]*filename[ ]*=[ ]*");
                    exp.setCaseSensitivity(Qt::CaseInsensitive);
                    fileName = tempStr.remove(exp);
                    break;
            }
    }
    if(!fileName.isEmpty()){
            this->fileName = fileName;
    }
}

void HttpDownload::slotDataReadProgress(int readBytes, int totalBytes){
    this->readBytes = readBytes;
    this->totalBytes = totalBytes;
    emit signalDataReadProgress(readBytes, totalBytes);
}

void HttpDownload::slotRequestFinished(int id, bool error){

    if(error){
            _downSuccess = false;
            _conFinish = true;
    }
    else{
            if(this->readBytes == this->totalBytes){_downSuccess =  true;}
            else{_downSuccess = false;}
            _conFinish = true;
    }

    if(requestId == id){
        writeFile(id, error);
    }

    http->close();
}

QString HttpDownload::getTargetDir(){
	return targetDir;
}
QString HttpDownload::getFileName(){
	return fileName;
}

bool HttpDownload::downSuccess(){
	return _downSuccess;
}
