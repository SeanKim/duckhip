#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include "qprogressindicator.h"
#include "httpdownload.h"

namespace Ui {
    class MainWindow;
}

class Item
{
    public:
        QString itemUri;
        QString downloadUri;
        QString videoUrl;
        QString htmlTag;
        QString title;
        QString talkId;
        QString date;
        QString fileName;
        int introDuration;
        int httpDownloadId;
        bool downloadEnable;
        bool downloadComplete;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    HttpDownload *httpDownload;

    QList<Item*> contents;

    QHttp redirectChecker;

    QProgressIndicator  *progressIndicator;
    QProgressBar        *progressBar;

    QLineEdit           *lineEdit;

    bool isTedViewer;

    int httpMode;
    int currentItemIndex;
    int count;
    int totalCount;
    QString downloadPath;

    QNetworkAccessManager manager;
    QNetworkReply *reply;

    QAction *actionDownload;
    QAction *actionDirectory;
    QAction *actionWebview;
    QAction *actionAbout;
    QAction *actionCancel;

    void createAction();

    void getResourceItem(QString itemUrl);
    void getTranslationList(QString lang);
    void getTEDSubtitlesByTalkID(QString takId);
    QString convertTEDSubtitlesToSMISubtitles(QString title, QByteArray json, int duration);

    bool parseTranslationJson(QByteArray byteArray);
    void parseResourceItem(QByteArray byteArray);

    void downloadVideo(int index);

    void init();

    void debugItem(int index);

    void checkDownloadedContent(QString dir);

private slots:
    void slotFinished(QNetworkReply *reply);

    void slotDownload();

    void slotDirectory();

    void slotToggled(bool flag);

    void slotItemClicked ( QListWidgetItem * clickedItem );

    void slotDownloadFinished(int id, bool error);

    void slotResponseHeaderReceived( const QHttpResponseHeader & resp );

    void slotLoadFinished ( bool ok );

    void slotDataReadProgress(int readBytes, int totalBytes);

    void slotCancelDownload();
};

#endif // MAINWINDOW_H
