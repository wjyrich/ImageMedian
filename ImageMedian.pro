#-------------------------------------------------
#
# Project created by QtCreator 2016-10-04T17:42:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageMedian
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    graphicssceneex.cpp \
    graphicsviewex.cpp

HEADERS  += mainwindow.h \
    extcolordefs.h \
    graphicssceneex.h \
    graphicsviewex.h

FORMS    += mainwindow.ui
target.path = $$PREFIX/bin
desktop.files = ImageMedian.desktop
desktop.path = $$PREFIX/share/applications/
icons.path = $$PREFIX/share/icons/hicolor/apps/
icons.files = ImageMedian.png

INSTALLS += target desktop icons
