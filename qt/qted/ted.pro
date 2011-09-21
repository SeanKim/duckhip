#-------------------------------------------------
#
# Project created by QtCreator 2011-09-02T14:24:18
#
#-------------------------------------------------

QT += core gui network webkit

TARGET = TedDownloader
TEMPLATE = app

SOURCES += main.cpp\
    mainwindow.cpp \
    json.cpp \
    httpdownload.cpp \
    qprogressindicator.cpp

HEADERS  += mainwindow.h \
    define.h \
    langcode.h \
    json.h \
    httpdownload.h \
    qprogressindicator.h

FORMS    += mainwindow.ui

#RC_FILES += images/icon.rc

RESOURCES += \
    ted.qrc
