#-------------------------------------------------
#
# Project created by QtCreator 2012-04-29T22:23:49
#
#-------------------------------------------------

QT += core gui

TARGET = cheetah-texture-packer

QT_VERSION=$$[QT_VERSION]

contains(QT_VERSION, "^5.*") {
  QT += widgets
} else {
}

TEMPLATE = app

SOURCES += main.cpp\
	mainwindow.cpp \
	view.cpp \
	imagepacker.cpp \
	imagecrop.cpp \
	imagesort.cpp \
	maxrects.cpp

HEADERS  += mainwindow.h \
	view.h \
	imagepacker.h \
	maxrects.h
FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -Wextra -Werror

TRANSLATIONS += tile_ru.ts
