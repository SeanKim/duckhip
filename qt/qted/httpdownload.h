#ifndef __HTTP_DOWNLOAD__
#define __HTTP_DOWNLOAD__

#include <QFile>
#include <QString>
#include <QHttp>
#include <QString>
#include <QHttpResponseHeader>

class HttpDownload : public QObject
{
	Q_OBJECT
	public :
            HttpDownload(QObject * parent = 0);
            int downloadFile(QString host, QString port ,QString targetUrl, QString targetDir);
            int downloadFile(QString host, QString port ,QString targetUrl, QString targetDir, QString fileName);

            void cancelDownload();
            bool conFinish();
            bool downSuccess();

            QString getTargetDir();
            QString getFileName();

	private :
            QHttp* http;

            QString targetDir;
            QString fileName;

            int readBytes;
            int totalBytes;
            int requestId;

            bool _downSuccess;
            bool _conFinish;
            void writeFile(int id, bool error);

	private slots :
            void slotResponseHeaderReceived( const QHttpResponseHeader & resp );
            void slotDataReadProgress(int readBytes, int totalBytes);
            void slotRequestFinished(int id, bool error);

	signals :
            void signalDataReadProgress(int readBytes, int totalBytes);
            void signalDownloadFinished(int id, bool error);
//            void signalResponseContentLength(long length);
};

#endif
