QT += widgets

TEMPLATE = app
TARGET = ProductDatabase

CONFIG += c++17 release windows
CONFIG -= debug_and_release debug

INCLUDEPATH += $$PWD/src

DESTDIR = $$OUT_PWD
MOC_DIR = $$OUT_PWD/moc
OBJECTS_DIR = $$OUT_PWD/obj

SOURCES += \
    src/main.cpp \
    src/db/Record.cpp \
    src/db/HashIndex.cpp \
    src/db/DatabaseManager.cpp \
    src/gui/RecordDialog.cpp \
    src/gui/DeleteDialog.cpp \
    src/gui/SearchDialog.cpp \
    src/gui/MainFrame.cpp

HEADERS += \
    src/db/Record.h \
    src/db/BinaryIO.h \
    src/db/DbException.h \
    src/db/HashIndex.h \
    src/db/DatabaseManager.h \
    src/gui/RecordDialog.h \
    src/gui/DeleteDialog.h \
    src/gui/SearchDialog.h \
    src/gui/MainFrame.h
