#ifndef DEFINE_H
#define DEFINE_H

const QString VERSION = "0.1.0";

const QString APP_TITLE = "TED Viewer and Downloader [Ver " + VERSION + "] - Download directory : %1";

const QString HOST = "www.ted.com";

const QString PORT = "80";

const QString URI_TRANSLATION_LIST = "/talks/listRPC/lang/";

const QString URI_TED_SUBTITLE = "/talks/subtitles/id/%1/lang/%2";

const QString HOST_DOWNLOAD = "download.ted.com";

const QString TRANSLATION_JSON_FILE = QDir::tempPath() + "/" + "lang.json";

const QString LANG_CODE = "kor";

const QString DOWNLOAD_COUNT = " Downloading...%1/%2";

const int MODE_GET_TRANSLATOR = 0x0;
const int MODE_GET_TALKID = 0x1;
const int MODE_GET_SUBTITLE = 0x5;
const int MODE_GET_DOWNLOADURL = 0x10;

const int DOWNLOAD_DISABLE = 0x0;
const int DOWNLOAD_ENABLE = 0x1;

const QString SMI_TEMPLATE = QString("<SAMI> <HEAD> <TITLE> %1 </TITLE>\n")
                             + QString("<STYLE TYPE=\"text/css\">\n")
                             + "<!-- P { margin-left:2pt; margin-right:2pt; margin-bottom:1pt;\n"
                             + "margin-top:1pt; font-size:20pt; text-align:center;\n"
                             + "font-family:Arial, Sans-serif; font-weight:bold; color:white; }\n"
                             + ".KRCC {Name: Korean; lang: ko-KR; SAMI_TYPE: CC;}\n"
                             + ".ENCC {Name: English; lang: en-US; SAMI_TYPE: CC;}-->\n"
                             + "</STYLE> </HEAD> <BODY> %2 </BODY> </SMI>";

const QString SMI_LINE = "<SYNC Start=%1><P Class=KRCC>\n%2";

const QString WEBVIEW_HTML = QString("<html> <body> ")
                             + "<h1 align=\"center\">If you want to go TED web page, Please click the TED button.</h1>"
                             + "</body> </html>";

const QString ERROR_JSON_PARSING = "JSON parsing error";
const QString ERROR_SMI_WRITE = "SMI write error";
const QString ERROR_SUBTITLE_READ = "subtitle json read error";

#endif // DEFINE_H
