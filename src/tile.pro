#-------------------------------------------------
#
# Project created by QtCreator 2012-04-29T22:23:49
#
#-------------------------------------------------

QT       += core gui

TARGET = tile
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    view.cpp \
    imagepacker.cpp \
    imagecrop.cpp \
    guillotine.cpp \
    imagesort.cpp \
    maxrects.cpp

HEADERS  += mainwindow.h \
    view.h \
    imagepacker.h \
    guillotine.h \
    maxrects.h
FORMS    += mainwindow.ui

TRANSLATIONS += tile_ru.ts
